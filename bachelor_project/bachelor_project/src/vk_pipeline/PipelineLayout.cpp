#include "PipelineLayout.h"

#include "vk_core/Context.h"

namespace vk {
	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding::as_vk_struct() const {
		VkDescriptorSetLayoutBinding vk_struct{};

		vk_struct.binding = binding;
		vk_struct.descriptorType = type;
		vk_struct.descriptorCount = count;
		vk_struct.stageFlags = stage_flags;

		return vk_struct;
	}

	DescriptorSetLayout DescriptorSetLayoutBuilder::build() const {
		DescriptorSetLayout layout;

		layout._bindings.resize(_bindings.size());
		std::vector<VkDescriptorSetLayoutBinding> vk_bindings{ _bindings.size() };
		for (uint32_t i = 0; i < _bindings.size(); i++) {
			auto binding = _bindings[i];
			if (binding.binding == UINT32_MAX) {
				binding.binding = i;
			}
			layout._bindings[i] = binding;
			vk_bindings[i] = binding.as_vk_struct();
		}

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(vk_bindings.size());
		layout_info.pBindings = vk_bindings.data();

		if (vkCreateDescriptorSetLayout(Context::get()->get_device(), &layout_info, nullptr, &*layout.descriptor_set_layout) != VK_SUCCESS) {
			throw std::runtime_error("Descriptor set layout creation failed!");
		}

		return layout;
	}

	PipelineLayout PipelineLayoutBuilder::build() const {
		PipelineLayout layout;

		std::vector<VkDescriptorSetLayout> vk_layouts{ _layouts.size() };
		for (uint32_t i = 0; i < _layouts.size(); i++) {
			vk_layouts[i] = _layouts[i]->handle();
		}

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(vk_layouts.size());
		pipeline_layout_info.pSetLayouts = vk_layouts.data();

		if (vkCreatePipelineLayout(Context::get()->get_device(), &pipeline_layout_info, nullptr, &*layout.pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("Pipeline layout creation failed!");
		}

		layout._descriptor_set_layouts = _layouts;

		return layout;
	}
}