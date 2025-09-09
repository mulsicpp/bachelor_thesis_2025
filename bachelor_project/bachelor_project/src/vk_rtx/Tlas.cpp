#include "Tlas.h"
#include "vk_core/Context.h"

#include "utils/align_up.h"

namespace vk {

    VkAccelerationStructureInstanceKHR TlasInstance::as_vk_struct() const {
        VkAccelerationStructureInstanceKHR vk_struct{};

        vk_struct.accelerationStructureReference = blas->device_address();

        for (uint32_t i = 0; i < 3; i++) {
            for (uint32_t j = 0; j < 4; j++) {
                vk_struct.transform.matrix[i][j] = transform[j][i];
            }
        }

        vk_struct.instanceCustomIndex = custom_index;
        vk_struct.mask = 0xFF;
        vk_struct.instanceShaderBindingTableRecordOffset = 0; // TODO selects correct hit group
        vk_struct.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;

        return vk_struct;
    }

    VkDeviceAddress Tlas::device_address() const {
        VkAccelerationStructureDeviceAddressInfoKHR addr_info{};
        addr_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
            addr_info.accelerationStructure = *tlas;

        return vkGetAccelerationStructureDeviceAddressKHR(Context::get()->get_device(), &addr_info);
    }


    void Tlas::refit(const std::vector<TlasInstance>& instances) {
        build(ASBuildMode::Refit, instances);
    }

    void Tlas::rebuild(const std::vector<TlasInstance>& instances) {
        build(ASBuildMode::Rebuild, instances);
    }

    void Tlas::build(ASBuildMode mode, const std::vector<TlasInstance>& instances) {
        const auto& acceleration_structure_props = Context::get()->get_acceleration_structure_props();
        const auto scratch_alignment = acceleration_structure_props.minAccelerationStructureScratchOffsetAlignment;

        if (mode == ASBuildMode::Refit && !_dynamic) {
            throw std::runtime_error("TLAS refitting is only possible for a dynamic TLAS!");
        }

        if (!instances.empty()) {
            _instances = instances;
        }

        if (mode == ASBuildMode::InitialBuild) {
            instances_staging_buffer = BufferBuilder()
                .add_queue_type(QueueType::Transfer)
                .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                .memory_usage(VMA_MEMORY_USAGE_CPU_ONLY)
                .size(sizeof(VkAccelerationStructureInstanceKHR) * _instances.size())
                .build();

            instances_buffer = BufferBuilder()
                .queue_types({ QueueType::Transfer, QueueType::Compute })
                .add_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                .add_usage(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
                .add_usage(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                .size(sizeof(VkAccelerationStructureInstanceKHR) * _instances.size())
                .build();
        }

        auto* p_instances = instances_staging_buffer.mapped_data<VkAccelerationStructureInstanceKHR>();

        for (uint32_t i = 0; i < _instances.size(); i++) {
            p_instances[i] = _instances[i].as_vk_struct();
        }


        instances_staging_buffer.copy_into(&instances_buffer);

        VkAccelerationStructureGeometryInstancesDataKHR instances_data{};
        instances_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        instances_data.arrayOfPointers = VK_FALSE;
        instances_data.data.deviceAddress = instances_buffer.device_address();

        VkAccelerationStructureGeometryKHR geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        geometry.flags = 0;
        geometry.geometry.instances = instances_data;

        VkAccelerationStructureBuildRangeInfoKHR range_info{};
        range_info.primitiveCount = static_cast<uint32_t>(_instances.size());
        range_info.primitiveOffset = 0;
        range_info.firstVertex = 0;
        range_info.transformOffset = 0;

        const VkAccelerationStructureBuildRangeInfoKHR* p_range_info = &range_info;

        VkBuildAccelerationStructureFlagsKHR build_flags = 0;

        build_flags |= _fast_build ?
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR :
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

        if (_dynamic) {
            build_flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        }
        VkBuildAccelerationStructureModeKHR build_mode = mode == ASBuildMode::Refit ?
            VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR :
            VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

        VkAccelerationStructureBuildGeometryInfoKHR build_info{};
        build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        build_info.flags = build_flags;
        build_info.mode = build_mode;
        build_info.geometryCount = 1;
        build_info.pGeometries = &geometry;

        if (mode == ASBuildMode::InitialBuild) {
            VkAccelerationStructureBuildSizesInfoKHR size_info{};
            size_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

            const auto device = Context::get()->get_device();

            vkGetAccelerationStructureBuildSizesKHR(
                device,
                VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &build_info,
                &range_info.primitiveCount,
                &size_info
            );

            build_scratch_buffer = BufferBuilder()
                .usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                .size(size_info.buildScratchSize + scratch_alignment)
                .build();

            if (_dynamic) {
                update_scratch_buffer = BufferBuilder()
                    .usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                    .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                    .size(size_info.updateScratchSize + scratch_alignment)
                    .build();
            }

            buffer = BufferBuilder()
                .usage(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                .queue_types({ QueueType::Transfer, QueueType::Compute })
                .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                .size(size_info.accelerationStructureSize)
                .build();

            VkAccelerationStructureCreateInfoKHR tlas_create_info{};
            tlas_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            tlas_create_info.buffer = buffer.handle();
            tlas_create_info.size = size_info.accelerationStructureSize;
            tlas_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

            if (vkCreateAccelerationStructureKHR(device, &tlas_create_info, nullptr, &*tlas) != VK_SUCCESS) {
                throw std::runtime_error("TLAS handle creation failed!");
            }
        }

        build_info.srcAccelerationStructure = mode == ASBuildMode::InitialBuild ? VK_NULL_HANDLE : *tlas;
        build_info.dstAccelerationStructure = *tlas;
        build_info.scratchData.deviceAddress = utils::align_up<VkDeviceAddress>((mode == ASBuildMode::Refit ? update_scratch_buffer : build_scratch_buffer).device_address(), scratch_alignment);

        const auto recorder = [&](ReadyCommandBuffer cmd_buffer) {
            vkCmdBuildAccelerationStructuresKHR(cmd_buffer.handle(), 1, &build_info, &p_range_info);

            VkMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
            barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

            vkCmdPipelineBarrier(
                cmd_buffer.handle(),
                VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR |
                VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                0,
                1, &barrier,
                0, nullptr,
                0, nullptr
            );
            };

        CommandBuffer::single_time_submit(QueueType::Compute, recorder);
    }



    Tlas TlasBuilder::build() const {
        Tlas tlas;

        tlas._instances = _instances;
        tlas._dynamic = _dynamic;
        tlas._fast_build = _fast_build;

        tlas.build(ASBuildMode::InitialBuild);

        dbg_log("tlas buffer size: %u", tlas.buffer.size());
        dbg_log("tlas scratch size: %u", tlas.build_scratch_buffer.size());

        return tlas;
    }
}