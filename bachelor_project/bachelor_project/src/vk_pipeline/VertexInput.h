#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace vk {

	struct VertexBindingInfo {
		using Ref = VertexBindingInfo&;

		uint32_t binding{ UINT32_MAX };
		uint32_t stride{ 0 };
		VkVertexInputRate input_rate{ VK_VERTEX_INPUT_RATE_VERTEX };

		VertexBindingInfo() = default;

		inline Ref set_binding(uint32_t binding) { this->binding = binding; return *this; }
		inline Ref set_stride(uint32_t stride) { this->stride = stride; return *this; }
		inline Ref set_input_rate(VkVertexInputRate input_rate) { this->input_rate = input_rate; return *this; }

		VkVertexInputBindingDescription as_description() const;
	};

	struct VertexAttributeInfo {
		using Ref = VertexAttributeInfo&;

		uint32_t location{ UINT32_MAX };
		uint32_t binding{ UINT32_MAX };
		VkFormat format{ VK_FORMAT_UNDEFINED };
		uint32_t offset{ 0 };

		inline Ref set_location(uint32_t location) { this->location = location; return *this; }
		inline Ref set_binding(uint32_t binding) { this->binding = binding; return *this; }
		inline Ref set_format(VkFormat format) { this->format = format; return *this; }
		inline Ref set_offset(uint32_t offset) { this->offset = offset; return *this; }

		VkVertexInputAttributeDescription as_description() const;
	};

	struct VertexInput {
		using Ref = VertexInput&;

		std::vector<VertexBindingInfo> binding_infos{};
		std::vector<VertexAttributeInfo> attribute_infos{};

		VertexInput() = default;

		inline Ref set_binding_infos(const std::vector<VertexBindingInfo>& binding_infos) { this->binding_infos = binding_infos; return *this; }
		inline Ref add_binding_info(const VertexBindingInfo& binding_info) { this->binding_infos.push_back(binding_info); return *this; }

		inline Ref set_attribute_infos(const std::vector<VertexAttributeInfo>& attribute_infos) { this->attribute_infos = attribute_infos; return *this; }
		inline Ref add_attribute_info(const VertexAttributeInfo& attribute_info) { this->attribute_infos.push_back(attribute_info); return *this; }

		VkPipelineVertexInputStateCreateInfo as_create_info() const;
	};
}