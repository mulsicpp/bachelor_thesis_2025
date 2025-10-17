#pragma once

#include "external/volk.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"
#include "vk_resources/ImageView.h"
#include "vk_resources/Sampler.h"

#include "vk_rtx/Tlas.h"

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
		{
		}

		inline BufferDescriptorInfo(const ptr::Shared<Buffer>& buffer, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
			: buffer{ buffer }
			, offset{ offset }
			, size{ size }
		{
		}

		VkDescriptorBufferInfo as_vk_struct() const;
	};

	struct ImageDescriptorInfo {
		using Ref = ImageDescriptorInfo&;

		ptr::Shared<ImageView> image_view{};
		ptr::Shared<Sampler> sampler{};
		VkImageLayout layout{ VK_IMAGE_LAYOUT_GENERAL };

		ImageDescriptorInfo() = default;

		inline ImageDescriptorInfo(ImageView&& image_view, VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
			: ImageDescriptorInfo{ std::move(image_view).to_shared(), layout }
		{
		}

		inline ImageDescriptorInfo(const ptr::Shared<ImageView>& image_view,  VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL, const ptr::Shared<Sampler> sampler = {})
			: image_view{ image_view }
			, layout{ layout }
			, sampler{ sampler}
		{
		}

		VkDescriptorImageInfo as_vk_struct() const;
	};

	struct TlasDescriptorInfo {
		using Ref = TlasDescriptorInfo&;

		ptr::Shared<Tlas> tlas{};

		TlasDescriptorInfo() = default;

		inline TlasDescriptorInfo(Tlas&& tlas)
			: TlasDescriptorInfo{ std::move(tlas).to_shared() }
		{
		}

		inline TlasDescriptorInfo(const ptr::Shared<Tlas>& tlas)
			: tlas{ tlas }
		{
		}
	};

	struct DescriptorInfo {
		std::variant<
			std::vector<BufferDescriptorInfo>,
			std::vector<ImageDescriptorInfo>,
			TlasDescriptorInfo
		> info{};

		DescriptorInfo() = default;

		inline DescriptorInfo(const std::vector<BufferDescriptorInfo>& buffer_infos) : info{ buffer_infos } {}
		inline DescriptorInfo(const BufferDescriptorInfo& buffer_info) : info{ std::vector<BufferDescriptorInfo>{ buffer_info } } {}

		inline DescriptorInfo(const std::vector<ImageDescriptorInfo>& image_infos) : info{ image_infos } {}
		inline DescriptorInfo(const ImageDescriptorInfo& image_info) : info{ std::vector<ImageDescriptorInfo>{ image_info } } {}

		inline DescriptorInfo(const TlasDescriptorInfo& tlas_infos) : info{ tlas_infos } {}
	};

	struct DescriptorSetInfo {
		using Ref = DescriptorSetInfo&;

		uint32_t index{ 0 };
		std::map<uint32_t, DescriptorInfo> bindings{};

		DescriptorSetInfo() = default;

		inline Ref set_index(uint32_t index) { this->index = index; return *this; }

		inline Ref set_binding(uint32_t binding, const DescriptorInfo& descriptor_info) { bindings[binding] = descriptor_info; return *this; }
	};

	class DescriptorPoolBuilder;

	class DescriptorPool : public utils::Move, public ptr::ToShared<DescriptorPool> {
		friend class DescriptorPoolBuilder;
	private:
		ptr::Shared<const PipelineLayout> _pipeline_layout;

		Handle<VkDescriptorPool> descriptor_pool{};

		std::vector<VkDescriptorSet> _sets{};

	public:
		DescriptorPool() = default;

		inline VkDescriptorPool handle() const { return *descriptor_pool; }

		inline uint32_t set_count() const { return static_cast<uint32_t>(_sets.size()); }

		void cmd_bind_set(ReadyCommandBuffer cmd_buffer, uint32_t set_index, VkPipelineBindPoint bind_point, std::vector<uint32_t> offsets = {}) const;

		void update_set_binding(uint32_t set_index, uint32_t binding, const DescriptorInfo& info);
	};

	class DescriptorPoolBuilder {
	public:
		using Ref = DescriptorPoolBuilder&;

	private:
		ptr::Shared<const PipelineLayout> _pipeline_layout{};
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