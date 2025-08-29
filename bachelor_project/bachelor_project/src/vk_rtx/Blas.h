#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"
#include "vk_resources/SubBuffer.h"

#include "vk_pipeline/VertexInput.h"

#include "external/volk.h"

namespace vk
{
    struct BlasGeometry {
        using Ref = BlasGeometry&;

        SubBuffer vertex_buffer{};
        uint32_t vertex_count{};

        VkFormat vertex_format{ VK_FORMAT_UNDEFINED };
        uint32_t vertex_stride{};
        uint32_t vertex_offset{};

        SubBuffer index_buffer{};
        VkIndexType index_type{ VK_INDEX_TYPE_NONE_KHR };

        uint32_t triangle_count{};

        Ref set_position_input(const SubBuffer& vertex_buffer, uint32_t vertex_count, const VertexInput& vertex_input, uint32_t location);
        Ref set_index_input(const SubBuffer& index_buffer, VkIndexType index_type);
        Ref set_triangle_count(uint32_t triangle_count) { this->triangle_count = triangle_count; return *this; }

        VkAccelerationStructureGeometryKHR as_vk_struct() const;
    };


    class BlasBuilder;

    class Blas : public utils::Move, public ptr::ToShared<Blas>
    {
        friend class BlasBuilder;
    private:
        Buffer update_scratch_buffer{};
        Buffer buffer{};
        Handle<VkAccelerationStructureKHR> blas{};

        std::vector<BlasGeometry> _geometries{};

    public:
        Blas() = default;

		inline VkAccelerationStructureKHR handle() const { return *blas; }

        VkDeviceAddress device_address() const;
    };

    class BlasBuilder
    {
    public:
        using Ref = BlasBuilder &;

    private:
        std::vector<BlasGeometry> _geometries{};
    public:
        Ref geometries(const std::vector<BlasGeometry>& geometries) { _geometries = geometries; return *this; }
        Ref add_geometry(const BlasGeometry& geometry) { _geometries.push_back(geometry); return *this; }

        Blas build() const;
    };
}