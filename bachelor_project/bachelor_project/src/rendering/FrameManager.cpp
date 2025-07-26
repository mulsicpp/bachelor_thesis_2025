#include "FrameManager.h"

#include "vk_core/Context.h"

#include <utility>

vk::Attachment FrameManager::get_swapchain_attachment() const {
	return vk::Attachment().from_swapchain(&swapchain);
}


void FrameManager::bind_rasterizer(Rasterizer&& rasterizer) {
	bind_rasterizer(std::move(rasterizer).to_shared());
}

void FrameManager::bind_rasterizer(const ptr::Shared<Rasterizer>& rasterizer) {
	renderer = rasterizer;

	frames = rasterizer->create_frames(max_frames_in_flight);

	submit_infos.resize(max_frames_in_flight);
	command_buffers.resize(max_frames_in_flight);

	for (uint32_t i = 0; i < max_frames_in_flight; i++) {
		vk::SubmitInfo info{};

		info.add_wait_semaphore(vk::Semaphore::create(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		info.add_signal_semaphore(vk::Semaphore::create());

		submit_infos[i] = std::move(info);

		command_buffers[i] = vk::CommandBufferBuilder(rasterizer->get_queue_type()).single_use(false).build();
	}

	create_framebuffers();
}

void FrameManager::draw_frame() {
	if (command_buffers.size() == 0) {
		throw std::runtime_error("Failed to draw frame! No rasterizer was selected");
	}

	const auto& context = *vk::Context::get();

	const auto device = context.get_device();

	command_buffers[in_flight_index].wait();

	uint32_t image_index;
	auto result = vkAcquireNextImageKHR(device, swapchain.handle(), UINT64_MAX, submit_infos[in_flight_index].wait_semaphores[0].handle(), VK_NULL_HANDLE, &image_index);


	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Swapchain image acquisition failed!");
	}


	auto draw_recorder = [this, image_index](vk::ReadyCommandBuffer cmd_buf) {
		renderer->cmd_draw_frame(cmd_buf, &frames[in_flight_index], &framebuffers[image_index]);
		};

	command_buffers[in_flight_index]
		.record(draw_recorder)
		.submit(submit_infos[in_flight_index]);


	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore present_wait_semaphore = submit_infos[in_flight_index].signal_semaphores[0].handle();
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

	in_flight_index = (in_flight_index + 1) % max_frames_in_flight;
}

void FrameManager::create_framebuffers() {
	framebuffers.resize(swapchain.image_count());
	const auto& swapchain_images = swapchain.images();

	for (int i = 0; i < framebuffers.size(); i++) {
		framebuffers[i] = vk::FramebufferBuilder()
			.render_pass(renderer->get_render_pass())
			.add_image(swapchain_images[i])
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

	frame_manager.max_frames_in_flight = std::min(frame_manager.swapchain.image_count(), _max_frames_in_flight);

	return frame_manager;
}