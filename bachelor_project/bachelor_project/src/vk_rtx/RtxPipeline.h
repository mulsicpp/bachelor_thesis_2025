#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_pipeline/Shader.h"
#include "vk_pipeline/PipelineLayout.h"

#include "external/volk.h"

namespace vk {

    class RtxPipelineBuilder;

    class RtxPipeline : public utils::Move, public ptr::ToShared<RtxPipeline> {
        friend class RtxPipelineBuilder;
    private:
        Handle<VkPipeline> pipeline;

    public:
        RtxPipeline() = default;

		inline VkPipeline handle() const { return *pipeline; }
    };

    class RtxPipelineBuilder {
	public:
		using Ref = RtxPipelineBuilder&;

	private:
		std::vector<ptr::Shared<const Shader>> _shaders{};
		ptr::Shared<const PipelineLayout> _layout{};

	public:
		RtxPipelineBuilder() = default;

		inline Ref shaders(const std::vector<ptr::Shared<const Shader>>& shaders) { _shaders = shaders; return *this; }
		inline Ref add_shader(Shader&& shader) { _shaders.push_back(std::move(shader).to_shared()); return *this; }
		inline Ref add_shader(const ptr::Shared<const Shader>& shader) { _shaders.push_back(shader); return *this; }

		inline Ref layout(PipelineLayout&& layout) { _layout = std::move(layout).to_shared(); return *this; }
		inline Ref layout(const ptr::Shared<const PipelineLayout>& layout) { _layout = layout; return *this; }

		RtxPipeline build() const;
	};
}