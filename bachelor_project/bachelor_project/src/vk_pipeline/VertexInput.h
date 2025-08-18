#pragma once

#include "external/volk.h"

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
		uint32_t binding{ 0 };
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

		std::vector<VkVertexInputBindingDescription> binding_descriptions{};
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

		VertexInput() = default;

		Ref set_binding_infos(std::vector<VertexBindingInfo> binding_infos);
		Ref add_binding_info(VertexBindingInfo binding_info);

		Ref set_attribute_infos(std::vector<VertexAttributeInfo> attribute_infos);
		Ref add_attribute_info(VertexAttributeInfo attribute_info);

		VkPipelineVertexInputStateCreateInfo as_create_info() const;
	};
}