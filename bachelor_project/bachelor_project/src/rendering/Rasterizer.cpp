#include "Rasterizer.h"

#include "utils/dbg_log.h"

#include "vk_core/Context.h"


void Rasterizer::draw() {

}

void Rasterizer::draw_triangle() {
	const auto& context = *vk::Context::get();

	const auto device = context.get_device();
	const auto swapchain = context.get_swapchain();

	render_cmd_buffer.wait();

	uint32_t image_index;
	vkAcquireNextImageKHR(device, swapchain.handle(), UINT64_MAX, submit_info.wait_semaphores[0].handle(), VK_NULL_HANDLE, &image_index);

	render_cmd_buffer.record(draw_triangle_recorder(image_index)).submit(submit_info);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore present_wait_semaphore = submit_info.signal_semaphores[0].handle();
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &present_wait_semaphore;

	VkSwapchainKHR swapchains[] = { swapchain.handle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;

	presentInfo.pImageIndices = &image_index;

	auto result = vkQueuePresentKHR(vk::Context::get()->get_command_manager()[vk::QueueType::Present].queue, &presentInfo);
}

vk::CommandRecorder Rasterizer::draw_triangle_recorder(uint32_t image_index) {
	return [this, image_index](vk::ReadyCommandBuffer cmd_buffer) -> void {
		framebuffers[image_index].cmd_begin_pass(cmd_buffer, pass_begin_info);

		pipeline.cmd_bind(cmd_buffer);
		vkCmdDraw(cmd_buffer.handle(), 3, 1, 0, 0);

		framebuffers[image_index].cmd_end_pass(cmd_buffer);
		};
}


RasterizerBuilder::RasterizerBuilder()
{}

Rasterizer RasterizerBuilder::build() {
	Rasterizer rasterizer;

	rasterizer.render_pass = vk::RenderPassBuilder()
		.add_attachment(vk::Attachment().color().from_swapchain())
		.build()
		.to_shared();

	dbg_log("created render pass");

	vk::Shader vertex_shader = vk::ShaderBuilder()
		.vertex()
		.load_spirv("assets/shaders/vert.spv")
		.build();
	dbg_log("loaded vertex shader");

	vk::Shader fragment_shader = vk::ShaderBuilder()
		.fragment()
		.load_spirv("assets/shaders/frag.spv")
		.build();
	dbg_log("loaded fragment shader");

	rasterizer.pipeline = vk::PipelineBuilder()
		.render_pass(rasterizer.render_pass)
		.add_shader(std::move(vertex_shader))
		.add_shader(std::move(fragment_shader))
		.build();

	dbg_log("created pipeline");

	const auto& context = *vk::Context::get();

	const auto& swapchain_images = context.get_swapchain().images();

	for (const auto& image : swapchain_images) {
		rasterizer.framebuffers.emplace_back(
			vk::FramebufferBuilder()
			.render_pass(rasterizer.render_pass)
			.add_image(image)
			.build()
		);
	}

	rasterizer.pass_begin_info = vk::PassBeginInfo()
		.add_clear_value(vk::ClearValue::color(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f }));

	rasterizer.render_cmd_buffer = vk::CommandBufferBuilder(vk::QueueType::Graphics).single_use(false).build();


	rasterizer.submit_info = {};
	rasterizer.submit_info.add_wait_semaphore(vk::Semaphore::create(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
		.add_signal_semaphore(vk::Semaphore::create());

	return rasterizer;
}