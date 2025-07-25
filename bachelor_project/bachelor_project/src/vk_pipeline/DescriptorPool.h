#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "PipelineLayout.h"

#include <vector>
#include <map>

namespace vk {
	class DescriptorPoolBuilder;

	class DescriptorPool : public utils::Move, public ptr::ToShared<DescriptorPool> {
		friend class DescriptorPoolBuilder;
	private:
		Handle<VkDescriptorPool> descriptor_pool{};
		std::map<VkDescriptorType, uint32_t> _sizes{};
		uint32_t _max_sets{ 1 };

	public:
		DescriptorPool() = default;

		inline VkDescriptorPool handle() const { return *descriptor_pool; }

		inline const std::map<VkDescriptorType, uint32_t>& sizes() const { return _sizes; }
		inline uint32_t size_of(VkDescriptorType type) const { return _sizes.contains(type) ? _sizes.at(type) : 0; }

		inline uint32_t max_sets() const { return _max_sets; }
	};

	class DescriptorPoolBuilder {
	public:
		using Ref = DescriptorPoolBuilder&;

	private:
		std::map<VkDescriptorType, uint32_t> _sizes{};
		uint32_t _max_sets{ 1 };

	public:
		DescriptorPoolBuilder() = default;

		inline Ref sizes(const std::map<VkDescriptorType, uint32_t>& sizes) { _sizes = sizes; return *this; }
		inline Ref add_size(VkDescriptorType type, uint32_t count) { _sizes[type] += count; return *this; }

		inline Ref max_sets(uint32_t max_sets) { _max_sets = max_sets; return *this; }

		Ref from_pipeline_layout(const PipelineLayout* pipeline_layout);

		DescriptorPool build() const;
	};
}