#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"
#include "vk_resources/ImageView.h"

#include "PipelineLayout.h"

#include <vector>
#include <map>
#include <variant>

namespace vk {

	struct BufferDescriptorInfo {
		using Ref = BufferDescriptorInfo&;

		ptr::Shared<Buffer> buffer{};
		VkDeviceSize offset{ 0 };
		VkDeviceSize size{ 0 };

		BufferDescriptorInfo() = default;

		inline BufferDescriptorInfo(Buffer&& buffer, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
			: BufferDescriptorInfo{ std::move(buffer).to_shared(), offset, size }
		{}

		inline BufferDescriptorInfo(const ptr::Shared<Buffer>& buffer, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
			: buffer{ buffer }
			, offset{ offset }
			, size{ size }
		{}

		VkDescriptorBufferInfo as_vk_struct() const;
	};

	struct DescriptorSetInfo {
		using Ref = DescriptorSetInfo&;

		uint32_t index{ 0 };
		std::map<uint32_t, std::vector<BufferDescriptorInfo>> bindings{};

		DescriptorSetInfo() = default;

		inline Ref set_index(uint32_t index) { this->index = index; return *this; }

		inline Ref set_binding(uint32_t binding, const BufferDescriptorInfo& buffer_descriptor_info) { bindings[binding] = std::vector<BufferDescriptorInfo>{ buffer_descriptor_info }; return *this; }
		inline Ref set_binding(uint32_t binding, const std::vector<BufferDescriptorInfo>& buffer_descriptor_infos) { bindings[binding] = buffer_descriptor_infos; return *this; }
	};

	class DescriptorPoolBuilder;

	class DescriptorPool : public utils::Move, public ptr::ToShared<DescriptorPool> {
		friend class DescriptorPoolBuilder;
	private:
		ptr::Shared<const PipelineLayout> _pipeline_layout;

		Handle<VkDescriptorPool> descriptor_pool{};

		std::vector<VkDescriptorSet> _sets{};
		std::vector<DescriptorSetInfo> _set_infos{};

	public:
		DescriptorPool() = default;

		inline VkDescriptorPool handle() const { return *descriptor_pool; }

		inline const std::vector<DescriptorSetInfo>& set_infos() const { return _set_infos; }
		inline uint32_t set_count() const { return static_cast<uint32_t>(_sets.size()); }

		void cmd_bind_set(ReadyCommandBuffer cmd_buffer, uint32_t set_index, std::vector<uint32_t> offsets = {});

		void update_set_binding(uint32_t set_index, uint32_t binding, const BufferDescriptorInfo& info);
		void update_set_binding(uint32_t set_index, uint32_t binding, const std::vector<BufferDescriptorInfo>& infos);
	};

	class DescriptorPoolBuilder {
	public:
		using Ref = DescriptorPoolBuilder&;

	private:
		ptr::Shared<const PipelineLayout> _pipeline_layout;
		std::vector<DescriptorSetInfo> _set_infos{};

	public:
		DescriptorPoolBuilder() = default;

		inline Ref pipeline_layout(PipelineLayout&& pipeline_layout) { _pipeline_layout = std::move(pipeline_layout).to_shared(); return *this; }
		inline Ref pipeline_layout(const ptr::Shared<PipelineLayout>& pipeline_layout) { _pipeline_layout = pipeline_layout; return *this; }

		inline Ref sets(const std::vector<DescriptorSetInfo>& set_infos) { _set_infos = set_infos; return *this; }
		inline Ref add_set(const DescriptorSetInfo& set_info) { _set_infos.push_back(set_info); return *this; }

		DescriptorPool build() const;
	};
}