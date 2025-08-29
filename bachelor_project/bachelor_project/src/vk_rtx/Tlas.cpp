#include "Tlas.h"
#include "vk_core/Context.h"

namespace vk {

    VkAccelerationStructureInstanceKHR TlasInstance::as_vk_struct() const {
        VkAccelerationStructureInstanceKHR vk_struct{};

        vk_struct.accelerationStructureReference = blas->device_address();

        for (uint32_t i = 0; i < 3; i++) {
            for (uint32_t j = 0; j < 4; j++) {
                vk_struct.transform.matrix[i][j] = transform[j][i];
            }
        }

        vk_struct.instanceCustomIndex = 0;
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



    Tlas TlasBuilder::build() const {
        Tlas tlas;

        Buffer staging_buffer = BufferBuilder()
            .add_queue_type(QueueType::Transfer)
            .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .memory_usage(VMA_MEMORY_USAGE_CPU_ONLY)
            .size(sizeof(VkAccelerationStructureInstanceKHR) * _instances.size())
            .build();

        auto* p_instances = staging_buffer.mapped_data<VkAccelerationStructureInstanceKHR>();

        for (uint32_t i = 0; i < _instances.size(); i++) {
            p_instances[i] = _instances[i].as_vk_struct();
        }

        Buffer instance_buffer = BufferBuilder()
            .queue_types({ QueueType::Transfer, QueueType::Compute })
            .add_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .add_usage(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
            .add_usage(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
            .size(sizeof(VkAccelerationStructureInstanceKHR) * _instances.size())
            .build();

        staging_buffer.copy_into(&instance_buffer);

        VkAccelerationStructureGeometryInstancesDataKHR instances_data{};
        instances_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        instances_data.arrayOfPointers = VK_FALSE;
        instances_data.data.deviceAddress = instance_buffer.device_address();

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

        VkAccelerationStructureBuildGeometryInfoKHR build_info{};
        build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        build_info.geometryCount = 1;
        build_info.pGeometries = &geometry;

        const auto device = Context::get()->get_device();

        VkAccelerationStructureBuildSizesInfoKHR size_info{};
        size_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;


        vkGetAccelerationStructureBuildSizesKHR(
            device,
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &build_info,
            &range_info.primitiveCount,
            &size_info
        );

        Buffer build_scratch_buffer = BufferBuilder()
            .usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
            .size(size_info.buildScratchSize)
            .build();

        tlas.buffer = BufferBuilder()
            .usage(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            .queue_types({ QueueType::Transfer, QueueType::Compute })
            .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
            .size(size_info.accelerationStructureSize)
            .build();

        VkAccelerationStructureCreateInfoKHR tlas_create_info{};
        tlas_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        tlas_create_info.buffer = tlas.buffer.handle();
        tlas_create_info.size = size_info.accelerationStructureSize;
        tlas_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

        if (vkCreateAccelerationStructureKHR(device, &tlas_create_info, nullptr, &*tlas.tlas) != VK_SUCCESS) {
            throw std::runtime_error("TLAS handle creation failed!");
        }

        build_info.dstAccelerationStructure = *tlas.tlas;
        build_info.scratchData.deviceAddress = build_scratch_buffer.device_address();

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

        tlas._instances = _instances;

        dbg_log("tlas buffer size: %u", tlas.buffer.size());
        dbg_log("tlas scratch size: %u", build_scratch_buffer.size());

        return tlas;
    }
}