#include "DescriptorPool.h"

#include "vk_core/Context.h"

namespace vk {

	VkDescriptorBufferInfo BufferDescriptorInfo::as_vk_struct() const {
		VkDescriptorBufferInfo vk_struct{};

		vk_struct.buffer = buffer->handle();
		vk_struct.offset = offset;
		vk_struct.range = size;

		return vk_struct;
	}

	void DescriptorPool::cmd_bind_set(ReadyCommandBuffer cmd_buffer, uint32_t set_index) {
		vkCmdBindDescriptorSets(cmd_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout->handle(), _set_infos[set_index].index, 1, &_sets[set_index], 0, nullptr);
	}

	void DescriptorPool::cmd_bind_set_dyn(ReadyCommandBuffer cmd_buffer, uint32_t set_index, uint32_t offset) {
		vkCmdBindDescriptorSets(cmd_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout->handle(), _set_infos[set_index].index, 1, &_sets[set_index], 1, &offset);
	}

	void DescriptorPool::update_set_binding(uint32_t set_index, uint32_t binding, const BufferDescriptorInfo& info) {
		update_set_binding(set_index, binding, std::vector<BufferDescriptorInfo>{ info });
	}

	void DescriptorPool::update_set_binding(uint32_t set_index, uint32_t binding, const std::vector<BufferDescriptorInfo>& infos) {
		std::vector<VkDescriptorBufferInfo> buffer_infos{ infos.size() };

		for (uint32_t i = 0; i < infos.size(); i++) {
			buffer_infos[i] = infos[i].as_vk_struct();
		}

		VkDescriptorType descriptor_type{};

		for (const auto& layout_binding : _pipeline_layout->descriptor_set_layouts()[_set_infos[set_index].index]->bindings()) {
			if (layout_binding.binding == binding) {
				descriptor_type = layout_binding.type;
				break;
			}
		}

		dbg_log("descriptot type: %u", descriptor_type);

		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = _sets[set_index];
		descriptor_write.dstBinding = binding;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = descriptor_type;
		descriptor_write.descriptorCount = static_cast<uint32_t>(buffer_infos.size());
		descriptor_write.pBufferInfo = buffer_infos.data();

		vkUpdateDescriptorSets(Context::get()->get_device(), 1, &descriptor_write, 0, nullptr);

		_set_infos[set_index].bindings[binding] = infos;
	}



	DescriptorPool DescriptorPoolBuilder::build() const {
		DescriptorPool pool;

		pool._pipeline_layout = _pipeline_layout;
		pool._set_infos = _set_infos;

		const auto& all_set_layouts = _pipeline_layout->descriptor_set_layouts();
		std::vector<VkDescriptorSetLayout> set_layout_handles{};
		set_layout_handles.reserve(_set_infos.size());

		std::map<VkDescriptorType, uint32_t> size_map{};

		for (const auto& set_info : _set_infos) {
			set_layout_handles.push_back(all_set_layouts[set_info.index]->handle());
			for (const auto& binding : all_set_layouts[set_info.index]->bindings()) {
				size_map[binding.type] += binding.count;
			}
		}

		std::vector<VkDescriptorPoolSize> sizes{};
		sizes.reserve(size_map.size());

		for (const auto& [type, count] : size_map) {
			sizes.push_back({ type, count });
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(sizes.size());
		pool_info.pPoolSizes = sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(_set_infos.size());

		if (vkCreateDescriptorPool(Context::get()->get_device(), &pool_info, nullptr, &*pool.descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("Descriptor pool creation failed!");
		}

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = *pool.descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(_set_infos.size());
		alloc_info.pSetLayouts = set_layout_handles.data();

		pool._sets.resize(_set_infos.size());
		if (vkAllocateDescriptorSets(Context::get()->get_device(), &alloc_info, pool._sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Descriptor set allocation failed!");
		}

		for (uint32_t i = 0; i < _set_infos.size(); i++) {
			for (const auto& [binding, infos] : _set_infos[i].bindings) {
				pool.update_set_binding(i, binding, infos);
			}
		}

		return pool;
	}
}