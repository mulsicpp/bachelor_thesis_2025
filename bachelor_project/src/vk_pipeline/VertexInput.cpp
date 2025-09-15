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



	VertexInput::Ref VertexInput::set_binding_infos(std::vector<VertexBindingInfo> binding_infos) {
		this->binding_descriptions.resize(binding_infos.size());

		for (uint32_t i = 0; i < binding_infos.size(); i++) {
			if (binding_infos[i].binding == UINT32_MAX) {
				binding_infos[i].binding = i;
			}
			this->binding_descriptions[i] = binding_infos[i].as_description();
		}

		this->binding_infos = binding_infos;
		return *this;
	}

	VertexInput::Ref VertexInput::add_binding_info(VertexBindingInfo binding_info) {
		if (binding_info.binding == UINT32_MAX) {
			binding_info.binding = static_cast<uint32_t>(this->binding_infos.size());
		}
		this->binding_infos.push_back(binding_info);
		this->binding_descriptions.push_back(binding_info.as_description());
		return *this;
	}



	VertexInput::Ref VertexInput::set_attribute_infos(std::vector<VertexAttributeInfo> attribute_infos) {
		this->attribute_descriptions.resize(attribute_infos.size());

		for (uint32_t i = 0; i < attribute_infos.size(); i++) {
			if (attribute_infos[i].location == UINT32_MAX) {
				attribute_infos[i].location = i;
			}
			this->attribute_descriptions[i] = attribute_infos[i].as_description();
		}
		this->attribute_infos = attribute_infos;
		return *this;
	}

	VertexInput::Ref VertexInput::add_attribute_info(VertexAttributeInfo attribute_info) {
		if (attribute_info.location == UINT32_MAX) {
			attribute_info.location = static_cast<uint32_t>(this->attribute_infos.size());
		}
		this->attribute_infos.push_back(attribute_info);
		this->attribute_descriptions.push_back(attribute_info.as_description());
		return *this;
	}



	VkPipelineVertexInputStateCreateInfo VertexInput::as_create_info() const {
		VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
		vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
		vertex_input_create_info.pVertexBindingDescriptions = binding_descriptions.data();
		vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
		vertex_input_create_info.pVertexAttributeDescriptions = attribute_descriptions.data();

		return vertex_input_create_info;
	}
}