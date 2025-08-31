#include "RtxPipeline.h"

#include "vk_core/Context.h"

#include <set>

namespace vk {
    RtxPipeline RtxPipelineBuilder::build() const {
        RtxPipeline pipeline;

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

        for (const auto shader_group : _shader_groups) {
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

                vk_group_infos.push_back(group_info);
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