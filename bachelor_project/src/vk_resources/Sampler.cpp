#include "Sampler.h"
#include "vk_core/Context.h"

namespace vk {


    Sampler SamplerBuilder::build() const {
        Sampler sampler;

        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = _mag_filter;
        sampler_info.minFilter = _min_filter;
        sampler_info.addressModeU = _address_mode_u;
        sampler_info.addressModeV = _address_mode_v;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.anisotropyEnable = VK_FALSE;
        sampler_info.maxAnisotropy = 0;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(Context::get()->get_device(), &sampler_info, nullptr, &*sampler.sampler) != VK_SUCCESS) {
            throw std::runtime_error("Sampler creation failed!");
        }

        return sampler;
    }
}