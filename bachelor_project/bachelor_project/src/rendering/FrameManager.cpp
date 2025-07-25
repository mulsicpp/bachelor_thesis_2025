#include "FrameManager.h"

#include "vk_core/Context.h"

#include <utility>

vk::Attachment FrameManager::get_swapchain_attachment() const {
	return vk::Attachment().from_swapchain(&swapchain);
}

void FrameManager::bind_rasterizer(const Rasterizer* rasterizer) {
	submit_infos.resize(max_frames_in_flight);
	command_buffers.resize(max_frames_in_flight);
	descriptor_pools.resize(max_frames_in_flight);

	for (uint32_t i = 0; i < max_frames_in_flight; i++) {
		vk::SubmitInfo info{};

		info.add_wait_semaphore(vk::Semaphore::create(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		info.add_signal_semaphore(vk::Semaphore::create());

		submit_infos[i] = std::move(info);

		command_buffers[i] = vk::CommandBufferBuilder(rasterizer->get_queue_type()).single_use(false).build();
		descriptor_pools[i] = vk::DescriptorPoolBuilder().from_pipeline_layout(rasterizer->get_pipeline_layout().get()).build();
	}

	const auto& swapchain_images = swapchain.images();

	for (int i = 0; i < framebuffers.size(); i++) {
		framebuffers[i] = vk::FramebufferBuilder()
			.render_pass(rasterizer->get_render_pass())
			.add_image(swapchain_images[i])
			.build();
	}
}

void FrameManager::draw_next(Rasterizer* rasterizer, DrawRecorder draw_recorder) {
	if (command_buffers.size() == 0) {
		throw std::runtime_error("Failed to draw frame! No rasterizer was selected");
	}

	const auto& context = *vk::Context::get();

	const auto device = context.get_device();

	command_buffers[in_flight_index].wait();

	uint32_t image_index;
	vkAcquireNextImageKHR(device, swapchain.handle(), UINT64_MAX, submit_infos[in_flight_index].wait_semaphores[0].handle(), VK_NULL_HANDLE, &image_index);

	command_buffers[in_flight_index].record((rasterizer->*draw_recorder)(&framebuffers[image_index])).submit(submit_infos[in_flight_index]);

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore present_wait_semaphore = submit_infos[in_flight_index].signal_semaphores[0].handle();
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &present_wait_semaphore;

	VkSwapchainKHR swapchains[] = { swapchain.handle() };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;

	present_info.pImageIndices = &image_index;

	auto result = vkQueuePresentKHR(vk::Context::get()->get_command_manager()[vk::QueueType::Present].queue, &present_info);

	in_flight_index = (in_flight_index + 1) % max_frames_in_flight;
}


FrameManager FrameManagerBuilder::build() {
	FrameManager frame_manager;

	frame_manager.swapchain = _swapchain_builder.build();

	frame_manager.max_frames_in_flight = std::min(frame_manager.swapchain.image_count(), _max_frames_in_flight);

	frame_manager.framebuffers.resize(frame_manager.swapchain.image_count());

	return frame_manager;
}