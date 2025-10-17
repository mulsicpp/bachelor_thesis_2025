#include "RtxPipeline.h"

#include "vk_core/Context.h"

#include "utils/align_up.h"

#include <set>
#include <cstring>

namespace vk {

    void hexdump(void* ptr, int buflen) {
        unsigned char* buf = (unsigned char*)ptr;
        int i, j;
        for (i = 0; i < buflen; i += 16) {
            printf("%06x: ", i);
            for (j = 0; j < 16; j++)
                if (i + j < buflen)
                    printf("%02x ", buf[i + j]);
                else
                    printf("   ");
            printf(" ");
            for (j = 0; j < 16; j++)
                if (i + j < buflen)
                    printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
            printf("\n");
        }
    }

    SBT RtxPipeline::build_sbt(const SBTInfo& info) const {
        SBT sbt{};

        sbt.info = info;

        const auto& raytracing_props = Context::get()->get_raytracing_props();

        const auto handle_size = raytracing_props.shaderGroupHandleSize;
        const auto handle_alignment = raytracing_props.shaderGroupHandleAlignment;
        const auto base_alignment = raytracing_props.shaderGroupBaseAlignment;

        const auto aligned_handle_size = utils::align_up(handle_size, handle_alignment);

        VkDeviceSize ray_gen_size = aligned_handle_size;
        VkDeviceSize miss_size = aligned_handle_size * info._miss_groups.size();
        VkDeviceSize hit_size = aligned_handle_size * info._hit_groups.size();

        VkDeviceSize ray_gen_offset = 0;
        VkDeviceSize miss_offset = utils::align_up<VkDeviceSize>(ray_gen_size, base_alignment);
        VkDeviceSize hit_offset = miss_offset + utils::align_up<VkDeviceSize>(miss_size, base_alignment);

        Buffer staging_buffer = BufferBuilder()
            .add_queue_type(QueueType::Transfer)
            .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .memory_usage(VMA_MEMORY_USAGE_CPU_ONLY)
            .size(hit_offset + hit_size)
            .build();

        uint8_t* staging_data = staging_buffer.mapped_data();

        std::vector<uint8_t> handle_buffer{};
        handle_buffer.resize(_shader_groups.size() * handle_size);

        const auto device = Context::get()->get_device();

        if (vkGetRayTracingShaderGroupHandlesKHR(device, *pipeline, 0, static_cast<uint32_t>(_shader_groups.size()), handle_buffer.size(), handle_buffer.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to get shader group handles!");
        }

        auto store_region = [&](const std::vector<ShaderGroup> groups, VkDeviceSize offset) {
            for (uint32_t i = 0; i < groups.size(); i++) {
                const auto& group = groups[i];
                auto idx = _group_id_to_idx.at(group.id());

                hexdump(handle_buffer.data() + idx * handle_size, handle_size);

                memcpy(staging_data + offset + aligned_handle_size * i, handle_buffer.data() + idx * handle_size, handle_size);
                if (aligned_handle_size > handle_size) {
                    memset(staging_data + offset + aligned_handle_size * i + handle_size, 0, aligned_handle_size - handle_size);
                }
            }
            };

        store_region(std::vector<ShaderGroup>{ info._ray_gen_group }, ray_gen_offset);
        store_region(info._miss_groups, miss_offset);
        store_region(info._hit_groups, hit_offset);

        printf("\n");

        // hexdump(staging_data, staging_buffer.size());

        sbt._buffer = BufferBuilder()
            .queue_types({ QueueType::Transfer, QueueType::Compute })
            .add_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .add_usage(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)
            .add_usage(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
            .size(staging_buffer.size())
            .build();

        auto device_address = sbt._buffer.device_address();

        staging_buffer.copy_into(&sbt._buffer);

        sbt.ray_gen_region.deviceAddress = device_address + ray_gen_offset;
        sbt.ray_gen_region.stride = aligned_handle_size;
        sbt.ray_gen_region.size = ray_gen_size;

        sbt.miss_region.deviceAddress = device_address + miss_offset;
        sbt.miss_region.stride = aligned_handle_size;
        sbt.miss_region.size = miss_size;

        sbt.hit_region.deviceAddress = device_address + hit_offset;
        sbt.hit_region.stride = aligned_handle_size;
        sbt.hit_region.size = hit_size;

        return sbt;
    }

    void RtxPipeline::rebuild_sbt(SBT* sbt) const {
        *sbt = build_sbt(sbt->info);
    }


    void RtxPipeline::cmd_bind(ReadyCommandBuffer cmd_buffer) const {
        vkCmdBindPipeline(cmd_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
    }

    void RtxPipeline::cmd_push_constant(ReadyCommandBuffer cmd_buffer, const void* value) {
        vkCmdPushConstants(cmd_buffer.handle(), _layout->handle(), _layout->push_constant().stage_flags, 0, _layout->push_constant().size, value);
    }

    void RtxPipeline::cmd_trace_rays(ReadyCommandBuffer cmd_buffer, const SBT& sbt, const VkExtent2D& image_extent) {
        VkStridedDeviceAddressRegionKHR call_region{};
        vkCmdTraceRaysKHR(cmd_buffer.handle(), &sbt.ray_gen_region, &sbt.miss_region, &sbt.hit_region, &call_region, image_extent.width, image_extent.height, 1);
    }


    RtxPipeline RtxPipelineBuilder::build() const {
        RtxPipeline pipeline{};

        std::vector<ptr::Shared<Shader>> shaders{};

        auto get_shader_idx = [&](const ptr::Shared<Shader>& shader) -> uint32_t {
            if (!shader) return VK_SHADER_UNUSED_KHR;

            for (uint32_t i = 0; i < shaders.size(); i++) {
                if (shaders[i].get() == shader.get()) {
                    return i;
                }
            }
            shaders.push_back(shader);
            return shaders.size() - 1;
            };

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> vk_group_infos{};

        for (uint32_t i = 0; i < _shader_groups.size(); i++) {
            const auto& shader_group = _shader_groups[i];
            const auto type = shader_group.type();

            VkRayTracingShaderGroupCreateInfoKHR group_info = {};
            group_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;

            if (type == ShaderGroupType::General) {
                const auto& general_group = shader_group.as_general();

                group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;

                group_info.generalShader = get_shader_idx(general_group.general);

                group_info.closestHitShader = VK_SHADER_UNUSED_KHR;
                group_info.anyHitShader = VK_SHADER_UNUSED_KHR;
                group_info.intersectionShader = VK_SHADER_UNUSED_KHR;
            }
            else {
                const auto& hit_group = shader_group.as_hit();

                group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;

                group_info.closestHitShader = get_shader_idx(hit_group.closest_hit);
                group_info.anyHitShader = get_shader_idx(hit_group.any_hit);

                group_info.generalShader = VK_SHADER_UNUSED_KHR;
                group_info.intersectionShader = VK_SHADER_UNUSED_KHR;
            }

            vk_group_infos.push_back(group_info);
            pipeline._group_id_to_idx[shader_group.id()] = i;
        }

        std::vector<VkPipelineShaderStageCreateInfo> vk_shader_infos{};
        for (const auto& shader : shaders) {
            vk_shader_infos.push_back(shader->get_create_info());
        }

        VkRayTracingPipelineCreateInfoKHR pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipeline_info.stageCount = static_cast<uint32_t>(vk_shader_infos.size());
        pipeline_info.pStages = vk_shader_infos.data();
        pipeline_info.groupCount = static_cast<uint32_t>(vk_group_infos.size());
        pipeline_info.pGroups = vk_group_infos.data();
        pipeline_info.maxPipelineRayRecursionDepth = _max_ray_recursion_depth;
        pipeline_info.layout = _layout->handle();

        const auto device = Context::get()->get_device();

        if (vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &*pipeline.pipeline) != VK_SUCCESS) {
            throw std::runtime_error("RTX pipeline creation failed!");
        }

        pipeline._shader_groups = _shader_groups;
        pipeline._layout = _layout;
        pipeline._max_ray_recursion_depth = _max_ray_recursion_depth;

        return pipeline;
    }
}