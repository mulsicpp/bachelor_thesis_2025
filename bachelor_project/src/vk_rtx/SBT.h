#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"

#include "ShaderGroup.h"

namespace vk {

    class RtxPipeline;

    class SBT : public utils::Move, public ptr::ToShared<SBT> {
        friend class RtxPipeline;
    private:
        Buffer _buffer{};
        VkStridedDeviceAddressRegionKHR ray_gen_region{};
        VkStridedDeviceAddressRegionKHR miss_region{};
        VkStridedDeviceAddressRegionKHR hit_region{};

    public:
        SBT() = default;
    };

    class SBTInfo {
        friend class RtxPipeline;
    public:
        using Ref = SBTInfo&;

    private:
        ShaderGroup _ray_gen_group{};
        std::vector<ShaderGroup> _miss_groups{};
        std::vector<ShaderGroup> _hit_groups{};
    public:
        SBTInfo() = default;

        inline Ref ray_gen_group(const ShaderGroup& ray_gen_group) { _ray_gen_group = ray_gen_group; return *this; }
        inline Ref miss_groups(const std::vector<ShaderGroup>& miss_groups) { _miss_groups = miss_groups; return *this; }
        inline Ref hit_groups(const std::vector<ShaderGroup>& hit_groups) { _hit_groups = hit_groups; return *this; }
    };
}