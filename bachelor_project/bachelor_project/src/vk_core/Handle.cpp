#include "Handle.h"

#include "Context.h"

#define IMPL_DESTROY(name) template<> void vk::destroy_handle<Vk##name>(Vk##name handle) { vkDestroy##name(Context::get()->get_device(), handle, nullptr); }

IMPL_DESTROY(Fence)
IMPL_DESTROY(Semaphore)

IMPL_DESTROY(RenderPass)
IMPL_DESTROY(ShaderModule)
IMPL_DESTROY(PipelineLayout)
IMPL_DESTROY(Pipeline)

IMPL_DESTROY(Buffer)
IMPL_DESTROY(BufferView)
IMPL_DESTROY(Image)
IMPL_DESTROY(ImageView)

template<>
void vk::destroy_handle<VmaAllocation>(VmaAllocation handle) {
	vmaFreeMemory(Context::get()->get_allocator(), handle);
}

template<>
void vk::destroy_handle<int*>(int* handle) {
	delete handle;
}