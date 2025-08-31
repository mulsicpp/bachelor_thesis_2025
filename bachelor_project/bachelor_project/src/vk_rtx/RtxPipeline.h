#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_pipeline/PipelineLayout.h"

#include "external/volk.h"

#include "ShaderGroup.h"

namespace vk {

    class RtxPipelineBuilder;

    class RtxPipeline : public utils::Move, public ptr::ToShared<RtxPipeline> {
        friend class RtxPipelineBuilder;
    private:
		std::vector<ShaderGroup> _shader_groups{};
		ptr::Shared<const PipelineLayout> _layout{};
		uint32_t _max_ray_recursion_depth{};
        Handle<VkPipeline> pipeline;

    public:
        RtxPipeline() = default;

		inline VkPipeline handle() const { return *pipeline; }
    };

    class RtxPipelineBuilder {
	public:
		using Ref = RtxPipelineBuilder&;

	private:
		std::vector<ShaderGroup> _shader_groups{};
		ptr::Shared<const PipelineLayout> _layout{};
		uint32_t _max_ray_recursion_depth{ 1 };

	public:
		RtxPipelineBuilder() = default;

		inline Ref shader_groups(const std::vector<ShaderGroup>& shader_groups) { _shader_groups = shader_groups; return *this; }
		inline Ref add_shader_group(const ShaderGroup& shader_group) { _shader_groups.push_back(shader_group); return *this; }

		inline Ref layout(PipelineLayout&& layout) { _layout = std::move(layout).to_shared(); return *this; }
		inline Ref layout(const ptr::Shared<const PipelineLayout>& layout) { _layout = layout; return *this; }

		inline Ref max_ray_recursion_depth(uint32_t max_ray_recursion_depth) { _max_ray_recursion_depth = max_ray_recursion_depth; return *this; }

		RtxPipeline build() const;
	};
}