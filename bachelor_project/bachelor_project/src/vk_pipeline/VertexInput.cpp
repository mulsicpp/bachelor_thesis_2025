#include "VertexInput.h"

#include <glm/glm.hpp>

namespace vk {

	VkVertexInputBindingDescription VertexBindingInfo::as_description() const {
		VkVertexInputBindingDescription description{};

		description.binding = binding;
		description.stride = stride;
		description.inputRate = input_rate;

		return description;
	}

	VkVertexInputAttributeDescription VertexAttributeInfo::as_description() const {
		VkVertexInputAttributeDescription description{};

		description.location = location;
		description.binding = binding;
		description.format = format;
		description.offset = offset;

		return description;
	}

	VkPipelineVertexInputStateCreateInfo VertexInput::as_create_info() const {
		VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
		vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::vector<VkVertexInputBindingDescription> binding_descriptions{ binding_infos.size() };
		for (int i = 0; i < binding_infos.size(); i++) {
			binding_descriptions[i] = binding_infos[i].as_description();
		}

		std::vector<VkVertexInputAttributeDescription> attribute_descriptions{ attribute_infos.size() };
		for (int i = 0; i < attribute_infos.size(); i++) {
			attribute_descriptions[i] = attribute_infos[i].as_description();
		}

		vertex_input_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
		vertex_input_create_info.pVertexBindingDescriptions = binding_descriptions.data();
		vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
		vertex_input_create_info.pVertexAttributeDescriptions = attribute_descriptions.data();

		return vertex_input_create_info;
	}
}