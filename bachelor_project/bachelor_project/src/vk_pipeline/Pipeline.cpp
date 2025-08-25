#include "Pipeline.h"

#include "vk_core/Context.h"

#include <stdexcept>

namespace vk {

    void Pipeline::cmd_bind(ReadyCommandBuffer cmd_buffer, const VkRect2D& viewport, const VkRect2D& scissor) const {
        if (!render_pass->active()) {
            throw std::runtime_error("Binding pipeline failed! Render pass is not active");
        }

        const auto& render_extent = render_pass->begin_info().render_area.extent;

        auto viewport_extent = viewport.extent;
        if (viewport_extent.width == UINT32_MAX) { viewport_extent.width = render_extent.width - viewport.offset.x; }
        if (viewport_extent.height == UINT32_MAX) { viewport_extent.height = render_extent.height - viewport.offset.x; }

        auto scissor_extent = scissor.extent;
        if (scissor_extent.width == UINT32_MAX) { scissor_extent.width = render_extent.width - scissor.offset.x; }
        if (scissor_extent.height == UINT32_MAX) { scissor_extent.height = render_extent.height - scissor.offset.x; }

        vkCmdBindPipeline(cmd_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get());

        VkViewport new_viewport{};
        new_viewport.x = static_cast<float>(viewport.offset.x);
        new_viewport.y = static_cast<float>(viewport.offset.y);
        new_viewport.width = static_cast<float>(viewport_extent.width);
        new_viewport.height = static_cast<float>(viewport_extent.height);
        new_viewport.minDepth = 0.0f;
        new_viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd_buffer.handle(), 0, 1, &new_viewport);

        VkRect2D new_scissor{};
        new_scissor.offset = scissor.offset;
        new_scissor.extent = scissor_extent;
        vkCmdSetScissor(cmd_buffer.handle(), 0, 1, &new_scissor);
    }

    void Pipeline::cmd_push_constant(ReadyCommandBuffer cmd_buffer, const void* value) {
        vkCmdPushConstants(cmd_buffer.handle(), layout->handle(), layout->push_constant().stage_flags, 0, layout->push_constant().size, value);
    }

    void Pipeline::cmd_bind_vertex_buffer(ReadyCommandBuffer cmd_buffer, uint32_t binding, const Buffer* buffer, VkDeviceSize offset) {
        VkBuffer vertex_buffer = buffer->handle();
        vkCmdBindVertexBuffers(cmd_buffer.handle(), binding, 1, &vertex_buffer, &offset);
    }

    void Pipeline::cmd_bind_index_buffer(ReadyCommandBuffer cmd_buffer, const Buffer* buffer, VkIndexType index_type, VkDeviceSize offset) {
        vkCmdBindIndexBuffer(cmd_buffer.handle(), buffer->handle(), offset, index_type);
    }

    void Pipeline::cmd_draw_indexed(ReadyCommandBuffer cmd_buffer, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {
        vkCmdDrawIndexed(cmd_buffer.handle(), index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void Pipeline::cmd_draw(ReadyCommandBuffer cmd_buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
        vkCmdDraw(cmd_buffer.handle(), vertex_count, instance_count, first_vertex, first_instance);
    }



	PipelineBuilder::PipelineBuilder()
		: _render_pass{}
		, _shaders{}
	{}

	Pipeline PipelineBuilder::build() {
		Pipeline pipeline;

		std::vector<VkPipelineShaderStageCreateInfo> shader_infos{};
		for (const auto& shader : _shaders) {
			shader_infos.push_back(shader->get_create_info());
		}


        VkPipelineVertexInputStateCreateInfo vertex_input_info = _vertex_input.as_create_info();

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamic_state.pDynamicStates = dynamicStates.data();


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shader_infos.size());
        pipelineInfo.pStages = shader_infos.data();
        pipelineInfo.pVertexInputState = &vertex_input_info;
        pipelineInfo.pInputAssemblyState = &input_assembly;
        pipelineInfo.pViewportState = &viewport_state;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depth_stencil;
        pipelineInfo.pColorBlendState = &color_blending;
        pipelineInfo.pDynamicState = &dynamic_state;
        pipelineInfo.layout = _layout->handle();
        pipelineInfo.renderPass = _render_pass->handle();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        const auto device = Context::get()->get_device();

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &*pipeline.pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Pipeline creation failed!");
        }

        pipeline.render_pass = _render_pass;
        pipeline.shaders = _shaders;
        pipeline.layout = _layout;


		return pipeline;
	}
}