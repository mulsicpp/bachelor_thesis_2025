#include "DescriptorPool.h"

#include "vk_core/Context.h"

namespace vk {

	DescriptorPoolBuilder::Ref DescriptorPoolBuilder::from_pipeline_layout(const PipelineLayout* pipeline_layout) {
		const auto& set_layouts = pipeline_layout->descriptor_set_layouts();

		_max_sets = set_layouts.size();
		_sizes.clear();

		for (const auto& set_layout : set_layouts) {
			for (const auto& binding : set_layout->bindings()) {
				_sizes[binding.type] += binding.count;
			}
		}

		return *this;
	}

	DescriptorPool DescriptorPoolBuilder::build() const {
		DescriptorPool pool;

		std::vector<VkDescriptorPoolSize> sizes{};
		sizes.reserve(_sizes.size());

		for (const auto& [type, count] : _sizes) {
			sizes.push_back({ type, count });
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(sizes.size());
		pool_info.pPoolSizes = sizes.data();
		pool_info.maxSets = _max_sets;

		if (vkCreateDescriptorPool(Context::get()->get_device(), &pool_info, nullptr, &*pool.descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("Descriptor pool creation failed!");
		}

		pool._sizes = _sizes;
		pool._max_sets = _max_sets;

		return pool;
	}
}