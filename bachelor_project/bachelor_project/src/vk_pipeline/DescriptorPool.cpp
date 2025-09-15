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

	VkDescriptorImageInfo ImageDescriptorInfo::as_vk_struct() const {
		VkDescriptorImageInfo vk_struct{};

		vk_struct.imageView = image_view->handle();
		vk_struct.imageLayout = layout;

		return vk_struct;
	}

	void DescriptorPool::cmd_bind_set(ReadyCommandBuffer cmd_buffer, uint32_t set_index, VkPipelineBindPoint bind_point, std::vector<uint32_t> offsets) const {
		vkCmdBindDescriptorSets(cmd_buffer.handle(), bind_point, _pipeline_layout->handle(), set_index, 1, &_sets[set_index], static_cast<uint32_t>(offsets.size()), offsets.data());
	}

	void DescriptorPool::update_set_binding(uint32_t set_index, uint32_t binding, const DescriptorInfo& info) {
		VkDescriptorType descriptor_type{};
		for (const auto& layout_binding : _pipeline_layout->descriptor_set_layouts()[set_index]->bindings()) {
			if (layout_binding.binding == binding) {
				descriptor_type = layout_binding.type;
				break;
			}
		}

		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = _sets[set_index];
		descriptor_write.dstBinding = binding;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = descriptor_type;

		switch (info.info.index()) {
		case 0: {
			const auto& buffer_infos = std::get<0>(info.info);
			std::vector<VkDescriptorBufferInfo> vk_buffer_infos{ buffer_infos.size() };

			for (uint32_t i = 0; i < buffer_infos.size(); i++) {
				vk_buffer_infos[i] = buffer_infos[i].as_vk_struct();
			}

			descriptor_write.descriptorCount = static_cast<uint32_t>(vk_buffer_infos.size());
			descriptor_write.pBufferInfo = vk_buffer_infos.data();

			vkUpdateDescriptorSets(Context::get()->get_device(), 1, &descriptor_write, 0, nullptr);

			break;
		}

		case 1: {
			const auto& image_infos = std::get<1>(info.info);
			std::vector<VkDescriptorImageInfo> vk_image_infos{ image_infos.size() };

			for (uint32_t i = 0; i < image_infos.size(); i++) {
				vk_image_infos[i] = image_infos[i].as_vk_struct();
			}

			descriptor_write.descriptorCount = static_cast<uint32_t>(vk_image_infos.size());
			descriptor_write.pImageInfo = vk_image_infos.data();

			vkUpdateDescriptorSets(Context::get()->get_device(), 1, &descriptor_write, 0, nullptr);

			break;
		}

		case 2: {
			const auto& tlas_info = std::get<2>(info.info);

			const auto tlas_handle = tlas_info.tlas->handle();

			VkWriteDescriptorSetAccelerationStructureKHR vk_tlas_info{};
			vk_tlas_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			vk_tlas_info.accelerationStructureCount = 1;
			vk_tlas_info.pAccelerationStructures = &tlas_handle;

			descriptor_write.descriptorCount = 1;
			descriptor_write.pNext = &vk_tlas_info;

			vkUpdateDescriptorSets(Context::get()->get_device(), 1, &descriptor_write, 0, nullptr);

			break;
		}

		}
	}



	DescriptorPool DescriptorPoolBuilder::build() const {
		DescriptorPool pool;

		pool._pipeline_layout = _pipeline_layout;

		const auto& all_set_layouts = _pipeline_layout->descriptor_set_layouts();
		std::vector<VkDescriptorSetLayout> set_layout_handles{};
		set_layout_handles.reserve(all_set_layouts.size());

		std::map<VkDescriptorType, uint32_t> size_map{};

		for (const auto& set_layout : all_set_layouts) {
			set_layout_handles.push_back(set_layout->handle());
			for (const auto& binding : set_layout->bindings()) {
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
		pool_info.maxSets = static_cast<uint32_t>(all_set_layouts.size());

		if (vkCreateDescriptorPool(Context::get()->get_device(), &pool_info, nullptr, &*pool.descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("Descriptor pool creation failed!");
		}

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = *pool.descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(set_layout_handles.size());
		alloc_info.pSetLayouts = set_layout_handles.data();

		pool._sets.resize(set_layout_handles.size());
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