#include "Handle.h"

#include "Context.h"

#define IMPL_DESTROY(name)													\
template<> void vk::destroy_handle<Vk##name>(Vk##name handle) {				\
	const auto p_context = Context::get_noexcept();							\
	if (p_context == nullptr) return;										\
	vkDestroy##name(p_context->get_device(), handle, nullptr);				\
}

IMPL_DESTROY(SwapchainKHR)

IMPL_DESTROY(Fence)
IMPL_DESTROY(Semaphore)

IMPL_DESTROY(RenderPass)
IMPL_DESTROY(Framebuffer)
IMPL_DESTROY(ShaderModule)
IMPL_DESTROY(DescriptorSetLayout)
IMPL_DESTROY(PipelineLayout)
IMPL_DESTROY(Pipeline)

IMPL_DESTROY(Buffer)
IMPL_DESTROY(BufferView)
IMPL_DESTROY(Image)
IMPL_DESTROY(ImageView)

template<>
void vk::destroy_handle<vk::ImageHandle>(vk::ImageHandle handle) {
	if (!handle.owns) {
		MOVE_LOG("image destruction ignored");
		return;
	}
	const auto p_context = Context::get_noexcept();
	if (p_context == nullptr) return;
	vkDestroyImage(p_context->get_device(), handle, nullptr);
}

template<>
void vk::destroy_handle<VmaAllocation>(VmaAllocation handle) {
	const auto p_context = Context::get_noexcept();
	if (p_context == nullptr) return;
	vmaFreeMemory(p_context->get_allocator(), handle);
}

template<>
void vk::destroy_handle<int*>(int* handle) {
	delete handle;
}