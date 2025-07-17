#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr_alias.h"

#include "RenderPass.h"
#include "Shader.h"


namespace vk {

	class PipelineBuilder;

	class Pipeline {
		friend class PipelineBuilder;
	private:
		VkPipeline pipeline;
		VkPipelineLayout layout;

		ptr::Shared<const RenderPass> render_pass;
		std::vector<ptr::Shared<const Shader>> shaders;

	public:
		Pipeline();

		inline VkPipeline handle() const { return pipeline; }

	private:
		void destroy();

		MOVE_SEMANTICS_VK_DEFAULT(Pipeline, pipeline)
	};

	class PipelineBuilder {
	public:
		using Ref = PipelineBuilder&;

	private:
		ptr::Shared<const RenderPass> _render_pass;
		std::vector<ptr::Shared<const Shader>> _shaders;

	public:
		PipelineBuilder();

		inline Ref render_pass(RenderPass&& render_pass) { _render_pass = ptr::make_shared<RenderPass>(std::move(render_pass)); return *this; }
		inline Ref render_pass(const ptr::Shared<const RenderPass>& render_pass) { _render_pass = render_pass; return *this; }

		inline Ref shaders(const std::vector<ptr::Shared<const Shader>>& shaders) { _shaders = shaders; return *this; }
		inline Ref add_shader(Shader&& shader) { _shaders.push_back(ptr::make_shared<Shader>(std::move(shader))); return *this; }
		inline Ref add_shader(const ptr::Shared<const Shader>& shader) { _shaders.push_back(shader); return *this; }

		Pipeline build();
	};
}