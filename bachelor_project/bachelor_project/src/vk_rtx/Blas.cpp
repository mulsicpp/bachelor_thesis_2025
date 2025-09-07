#include "Blas.h"

#include "vk_core/Context.h"

#include <string>

namespace vk
{

    BlasGeometry::Ref BlasGeometry::set_position_input(const SubBuffer& vertex_buffer, uint32_t vertex_count, const VertexInput& vertex_input, uint32_t location) {
        this->vertex_buffer = vertex_buffer;
        this->vertex_count = vertex_count;

        for (const auto& info : vertex_input.attribute_infos)
        {
            if (info.location == location)
            {
                this->vertex_format = info.format;
                this->vertex_offset = info.offset;
                for (const auto& binding_info : vertex_input.binding_infos)
                {
                    if (binding_info.binding == info.binding)
                    {
                        this->vertex_stride = binding_info.stride;
                        return *this;
                    }
                }
                throw std::runtime_error("No binding with index " + std::to_string(info.binding) + " found");
            }
        }
        throw std::runtime_error("No attribute with location " + std::to_string(location) + " found");
    }

    BlasGeometry::Ref BlasGeometry::set_index_input(const SubBuffer& index_buffer, VkIndexType index_type) {
        this->index_buffer = index_buffer;
        this->index_type = index_type;

        return *this;
    }

    VkAccelerationStructureGeometryKHR BlasGeometry::as_vk_struct() const {
        VkAccelerationStructureGeometryKHR vk_struct{};
        vk_struct.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        vk_struct.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

        vk_struct.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

        VkAccelerationStructureGeometryTrianglesDataKHR triangle_data{};

        triangle_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        triangle_data.vertexFormat = vertex_format;
        triangle_data.vertexStride = vertex_stride;
        triangle_data.vertexData.deviceAddress = vertex_buffer.buffer()->device_address() + vertex_buffer.offset() + vertex_offset;
        triangle_data.maxVertex = vertex_count - 1;

        triangle_data.indexType = index_type;
        triangle_data.indexData.deviceAddress = index_type == VK_INDEX_TYPE_NONE_KHR ? 0 : index_buffer.buffer()->device_address() + index_buffer.offset();

        triangle_data.transformData.deviceAddress = 0;

        vk_struct.geometry.triangles = triangle_data;

        return vk_struct;
    }

    VkDeviceAddress Blas::device_address() const {
        VkAccelerationStructureDeviceAddressInfoKHR addr_info{};
        addr_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        addr_info.accelerationStructure = *blas;

        return vkGetAccelerationStructureDeviceAddressKHR(Context::get()->get_device(), &addr_info);
    }

    void Blas::refit(const std::vector<BlasGeometry>& geometries) {
        build(ASBuildMode::Refit, geometries);
    }

    void Blas::rebuild(const std::vector<BlasGeometry>& geometries) {
        build(ASBuildMode::Rebuild, geometries);
    }

    void Blas::build(ASBuildMode mode, const std::vector<BlasGeometry>& geometries) {
        if (mode == ASBuildMode::Refit && !_dynamic) {
            throw std::runtime_error("BLAS refitting is only possible for a dynamic BLAS!");
        }

        if (!geometries.empty()) {
            _geometries = geometries;
        }

        std::vector<VkAccelerationStructureGeometryKHR> vk_geometries{};

        vk_geometries.resize(_geometries.size());

        dbg_log("geometry count: %lu", _geometries.size());

        for (uint32_t i = 0; i < vk_geometries.size(); i++) {
            vk_geometries[i] = _geometries[i].as_vk_struct();
        }

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
        build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        build_info.flags = build_flags;
        build_info.mode = build_mode;
        build_info.geometryCount = static_cast<uint32_t>(vk_geometries.size());
        build_info.pGeometries = vk_geometries.data();

        if (mode == ASBuildMode::InitialBuild) {

            VkAccelerationStructureBuildSizesInfoKHR size_info{};
            size_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

            const auto device = Context::get()->get_device();

            std::vector<uint32_t> triangle_counts{};

            triangle_counts.resize(_geometries.size());

            uint32_t total_triangle_count = 0;
            for (uint32_t i = 0; i < _geometries.size(); i++) {
                triangle_counts[i] = _geometries[i].triangle_count;
                // dbg_log("triangle count %u: %u", i, triangle_counts[i]);
                total_triangle_count += triangle_counts[i];
            }
            dbg_log("total triangle count: %u", total_triangle_count);

            vkGetAccelerationStructureBuildSizesKHR(
                device,
                VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &build_info,
                triangle_counts.data(),
                &size_info);

            build_scratch_buffer = BufferBuilder()
                .usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                .size(size_info.buildScratchSize)
                .build();

            if (_dynamic) {
                update_scratch_buffer = BufferBuilder()
                    .usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                    .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                    .size(size_info.updateScratchSize)
                    .build();
            }

            buffer = BufferBuilder()
                .usage(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
                .queue_types({ QueueType::Transfer, QueueType::Compute })
                .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                .size(size_info.accelerationStructureSize)
                .build();


            VkAccelerationStructureCreateInfoKHR blas_create_info{};
            blas_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            blas_create_info.buffer = buffer.handle();
            blas_create_info.size = size_info.accelerationStructureSize;
            blas_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

            if (vkCreateAccelerationStructureKHR(device, &blas_create_info, nullptr, &*blas) != VK_SUCCESS) {
                throw std::runtime_error("BLAS handle creation failed!");
            }
        }

        build_info.srcAccelerationStructure = mode == ASBuildMode::InitialBuild ? VK_NULL_HANDLE : *blas;
        build_info.dstAccelerationStructure = *blas;
        build_info.scratchData.deviceAddress = (mode == ASBuildMode::Refit ? update_scratch_buffer : build_scratch_buffer).device_address();

        std::vector<VkAccelerationStructureBuildRangeInfoKHR> range_infos{};

        range_infos.resize(_geometries.size());

        VkAccelerationStructureBuildRangeInfoKHR range_info{};
        range_info.primitiveOffset = 0;
        range_info.firstVertex = 0;
        range_info.transformOffset = 0;

        for (uint32_t i = 0; i < _geometries.size(); i++) {
            range_info.primitiveCount = _geometries[i].triangle_count;
            range_infos[i] = range_info;
        }

        const auto* p_range_infos = range_infos.data();

        const auto recorder = [&](ReadyCommandBuffer cmd_buffer) {
            vkCmdBuildAccelerationStructuresKHR(cmd_buffer.handle(), 1, &build_info, &p_range_infos);

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



    Blas BlasBuilder::build() const {
        Blas blas;

        blas._geometries = _geometries;
        blas._dynamic = _dynamic;
        blas._fast_build = _fast_build;

        blas.build(ASBuildMode::InitialBuild);

        dbg_log("blas buffer size: %u", blas.buffer.size());
        dbg_log("blas scratch size: %u", blas.build_scratch_buffer.size());

        return blas;
    }
}
