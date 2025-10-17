#pragma once

#include "external/volk.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

namespace vk {

    class SamplerBuilder;

    class Sampler : public utils::Move, public ptr::ToShared<Sampler> {
        friend class SamplerBuilder;
    private:
        Handle<VkSampler> sampler{};

    public:
        Sampler() = default;

        inline VkSampler handle() const { return *sampler; }
    };

    class SamplerBuilder {
    public:
        using Ref = SamplerBuilder&;
    private:
        VkFilter _mag_filter{ VK_FILTER_LINEAR };
        VkFilter _min_filter{ VK_FILTER_LINEAR };

        VkSamplerAddressMode _address_mode_u { VK_SAMPLER_ADDRESS_MODE_REPEAT };
        VkSamplerAddressMode _address_mode_v { VK_SAMPLER_ADDRESS_MODE_REPEAT };

    public:
        SamplerBuilder() = default;

        inline Ref mag_filter(VkFilter mag_filter) { _mag_filter = mag_filter; return *this; }
        inline Ref min_filter(VkFilter min_filter) { _min_filter = min_filter; return *this; }

        inline Ref address_mode_u(VkSamplerAddressMode address_mode_u) { _address_mode_u = address_mode_u; return *this; }
        inline Ref address_mode_v(VkSamplerAddressMode address_mode_v) { _address_mode_v = address_mode_v; return *this; }

        Sampler build() const;
    };
}