#include "FrameManager.h"

#include "vk_core/Context.h"
#include "vk_core/format.h"

#include <utility>

vk::Attachment FrameManager::get_swapchain_attachment() const {
	return vk::Attachment().from_swapchain(&swapchain);
}

vk::Attachment FrameManager::get_depth_attachment() const {
	return vk::Attachment()
		.depth()
		.set_format(vk::find_depth_format())
		.set_final_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.set_load_op(VK_ATTACHMENT_LOAD_OP_CLEAR)
		.set_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE);
}


void FrameManager::bind_rasterizer(Rasterizer&& rasterizer) {
	bind_rasterizer(std::move(rasterizer).to_shared());
}

void FrameManager::bind_rasterizer(const ptr::Shared<Rasterizer>& rasterizer) {
	renderer = rasterizer;

	submit_info = vk::SubmitInfo{};
	submit_info.add_wait_semaphore(vk::Semaphore::create(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	submit_info.add_signal_semaphore(vk::Semaphore::create());

	command_buffer = vk::CommandBufferBuilder(rasterizer->get_queue_type()).single_use(false).build();

	create_framebuffers();
}

void FrameManager::draw_frame(const Frame& frame) {
	if (!renderer) {
		throw std::runtime_error("Failed to draw frame! No rasterizer was selected");
	}

	const auto& context = *vk::Context::get();

	const auto device = context.get_device();

	command_buffer.wait();

	uint32_t image_index;
	auto result = vkAcquireNextImageKHR(device, swapchain.handle(), UINT64_MAX, submit_info.wait_semaphores[0].handle(), VK_NULL_HANDLE, &image_index);


	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Swapchain image acquisition failed!");
	}


	auto draw_recorder = [this, image_index, &frame](vk::ReadyCommandBuffer cmd_buf) {
		renderer->cmd_draw_frame(cmd_buf, frame, &framebuffers[image_index]);
		};

	command_buffer
		.record(draw_recorder)
		.submit(submit_info);


	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore present_wait_semaphore = submit_info.signal_semaphores[0].handle();
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &present_wait_semaphore;

	VkSwapchainKHR swapchains[] = { swapchain.handle() };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;

	present_info.pImageIndices = &image_index;

	result = vkQueuePresentKHR(vk::Context::get()->get_command_manager()[vk::QueueType::Present].queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resize_signaled) {
		resize_signaled = false;
		recreate();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Image presentation failed!");
	}
}

void FrameManager::create_framebuffers() {
	depth_image = vk::ImageBuilder()
		.extent(swapchain.extent())
		.format(get_depth_attachment().format)
		.usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		.aspect(VK_IMAGE_ASPECT_DEPTH_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.tiling(VK_IMAGE_TILING_OPTIMAL)
		.add_queue_type(vk::QueueType::Graphics)
		.build().to_shared();

	framebuffers.resize(swapchain.image_count());
	const auto& swapchain_images = swapchain.images();

	for (int i = 0; i < framebuffers.size(); i++) {
		framebuffers[i] = vk::FramebufferBuilder()
			.render_pass(renderer->get_render_pass())
			.add_image(swapchain_images[i])
			.add_image(depth_image)
			.build();
	}
}

void FrameManager::recreate() {
	swapchain = vk::SwapchainBuilder().rebuild(std::move(swapchain));

	if (renderer) {
		create_framebuffers();
	}
}


FrameManager FrameManagerBuilder::build() {
	FrameManager frame_manager;

	frame_manager.swapchain = _swapchain_builder.build();

	return frame_manager;
}