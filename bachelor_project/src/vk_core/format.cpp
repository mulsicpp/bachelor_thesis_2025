#include "format.h"

#include "Context.h"

#include <glm/glm.hpp>
#include <stdexcept>
#include <vector>



#define IMPL_FORMAT_FROM_TYPE(type, format) \
template<>									\
VkFormat vk::format_of_type<type>() {		\
	return format;							\
}

IMPL_FORMAT_FROM_TYPE(float, VK_FORMAT_R32_SFLOAT)
IMPL_FORMAT_FROM_TYPE(glm::vec2, VK_FORMAT_R32G32_SFLOAT)
IMPL_FORMAT_FROM_TYPE(glm::vec3, VK_FORMAT_R32G32B32_SFLOAT)
IMPL_FORMAT_FROM_TYPE(glm::vec4, VK_FORMAT_R32G32B32A32_SFLOAT)

IMPL_FORMAT_FROM_TYPE(int32_t, VK_FORMAT_R32_SINT)
IMPL_FORMAT_FROM_TYPE(glm::ivec2, VK_FORMAT_R32G32_SINT)
IMPL_FORMAT_FROM_TYPE(glm::ivec3, VK_FORMAT_R32G32B32_SINT)
IMPL_FORMAT_FROM_TYPE(glm::ivec4, VK_FORMAT_R32G32B32A32_SINT)

IMPL_FORMAT_FROM_TYPE(uint32_t, VK_FORMAT_R32_UINT)
IMPL_FORMAT_FROM_TYPE(glm::uvec2, VK_FORMAT_R32G32_UINT)
IMPL_FORMAT_FROM_TYPE(glm::uvec3, VK_FORMAT_R32G32B32_UINT)
IMPL_FORMAT_FROM_TYPE(glm::uvec4, VK_FORMAT_R32G32B32A32_UINT)

IMPL_FORMAT_FROM_TYPE(int16_t, VK_FORMAT_R16_SINT)
IMPL_FORMAT_FROM_TYPE(uint16_t, VK_FORMAT_R16_UINT)

IMPL_FORMAT_FROM_TYPE(int8_t, VK_FORMAT_R8_SINT)
IMPL_FORMAT_FROM_TYPE(uint8_t, VK_FORMAT_R8_UINT)

VkFormat vk::find_depth_format() {
    std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

    for (VkFormat format : formats) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(Context::get()->get_physical_device(), format, &props);

        if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            return format;
        }
    }

    throw std::runtime_error("Depth format not found!");
}