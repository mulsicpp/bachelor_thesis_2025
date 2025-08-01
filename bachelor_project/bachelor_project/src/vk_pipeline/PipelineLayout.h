#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include <vector>

namespace vk {

	struct DescriptorSetLayoutBinding {
		using Ref = DescriptorSetLayoutBinding&;

		uint32_t binding{ UINT32_MAX };
		VkDescriptorType type{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };
		uint32_t count{ 1 };
		VkShaderStageFlags stage_flags{ 0 };

		DescriptorSetLayoutBinding() = default;

		inline Ref set_binding(uint32_t binding) { this->binding = binding; return *this; }
		inline Ref set_type(VkDescriptorType type) { this->type = type; return *this; }
		inline Ref set_count(uint32_t count) { this->count = count; return *this; }

		inline Ref set_stage_flags(VkShaderStageFlags stage_flags) { this->stage_flags = stage_flags; return *this; }
		inline Ref add_stage_flag(VkShaderStageFlagBits stage_flag) { this->stage_flags |= stage_flag; return *this; }

		VkDescriptorSetLayoutBinding as_vk_struct() const;
	};

	class DescriptorSetLayoutBuilder;

	class DescriptorSetLayout : public utils::Move, public ptr::ToShared<DescriptorSetLayout> {
		friend class DescriptorSetLayoutBuilder;
	private:
		Handle<VkDescriptorSetLayout> descriptor_set_layout{};
		std::vector<DescriptorSetLayoutBinding> _bindings{};

	public:
		DescriptorSetLayout() = default;

		inline VkDescriptorSetLayout handle() const { return *descriptor_set_layout; }
		inline const std::vector<DescriptorSetLayoutBinding> bindings() const { return _bindings; }
	};

	class DescriptorSetLayoutBuilder {
	public:
		using Ref = DescriptorSetLayoutBuilder&;

	private:
		std::vector<DescriptorSetLayoutBinding> _bindings{};

	public:
		DescriptorSetLayoutBuilder() = default;

		inline Ref bindings(const std::vector<DescriptorSetLayoutBinding>& bindings) { _bindings = bindings; return *this; }
		inline Ref add_binding(const DescriptorSetLayoutBinding& binding) { _bindings.push_back(binding); return *this; }

		DescriptorSetLayout build() const;
	};

	struct PushConstant {
		using Ref = PushConstant&;

		uint32_t size{ 0 };
		VkShaderStageFlags stage_flags{ 0 };

		PushConstant() = default;

		inline Ref set_size(uint32_t size) { this->size = size; return *this; }
		inline Ref set_stage_flags(VkShaderStageFlags stage_flags) { this->stage_flags = stage_flags; return *this; }
		inline Ref add_stage_flag(VkShaderStageFlagBits stage_flag) { this->stage_flags |= stage_flag; return *this; }

		VkPushConstantRange as_vk_struct() const;
	};

	class PipelineLayoutBuilder;

	class PipelineLayout : public utils::Move, public ptr::ToShared<PipelineLayout> {
		friend class PipelineLayoutBuilder;
	private:
		Handle<VkPipelineLayout> pipeline_layout{};
		std::vector<ptr::Shared<const DescriptorSetLayout>> _descriptor_set_layouts{};
		PushConstant _push_constant{};

	public:
		PipelineLayout() = default;

		inline VkPipelineLayout handle() const { return *pipeline_layout; }
		inline const std::vector<ptr::Shared<const DescriptorSetLayout>>& descriptor_set_layouts() const { return _descriptor_set_layouts; }
		inline const PushConstant& push_constant() const { return _push_constant; }
	};

	class PipelineLayoutBuilder {
	public:
		using Ref = PipelineLayoutBuilder&;

	private:
		std::vector<ptr::Shared<const DescriptorSetLayout>> _layouts{};
		PushConstant _push_constant{};

	public:
		PipelineLayoutBuilder() = default;

		inline Ref layouts(const std::vector<ptr::Shared<const DescriptorSetLayout>>& layouts) { _layouts = layouts; return *this; }
		inline Ref add_layout(DescriptorSetLayout&& layout) { _layouts.push_back(std::move(layout).to_shared()); return *this; }
		inline Ref add_layout(const ptr::Shared<const DescriptorSetLayout>& layout) { _layouts.push_back(layout); return *this; }

		inline Ref push_constant(const PushConstant& push_constant) { _push_constant = push_constant; return *this; }

		PipelineLayout build() const;
	};
}