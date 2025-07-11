#include "Context.h"

#include "utils/defines.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720

#define APP_NAME "Raytracing App"

const std::vector<const char*> REQUIRED_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> REQUIRED_RAYTRACING_EXTENSIONS = {
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
};

namespace vk {
	Context::Context() {

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, nullptr, nullptr);

		vkb::InstanceBuilder instance_builder;

		instance_builder.set_app_name(APP_NAME);

		if (USE_VALIDATION_LAYERS) {
			instance_builder.request_validation_layers();
			instance_builder.use_default_debug_messenger();
		}

		auto instance_result = instance_builder.build();
		if (!instance_result) {
			throw std::runtime_error("Instance creation failed! " + instance_result.error().message());
		}

		m_instance = instance_result.value();

		if (glfwCreateWindowSurface(m_instance.instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
			throw std::runtime_error("Surface creation failed!");
		}

		vkb::PhysicalDeviceSelector selector(m_instance);
		selector.set_minimum_version(1, 2);
		selector.set_surface(m_surface);
		selector.add_required_extensions(REQUIRED_EXTENSIONS);

		if (USE_RAYTRACING) {
			selector.add_required_extensions(REQUIRED_RAYTRACING_EXTENSIONS);
		}

		auto physical_device_result = selector.select();
		if (!physical_device_result) {
			throw std::runtime_error("Physical device selection failed! " + physical_device_result.error().message());
		}

		m_physical_device = physical_device_result.value();

		vkb::DeviceBuilder device_builder(m_physical_device);

		auto device_result = device_builder.build();
		if (!device_result) {
			throw std::runtime_error("Device creation failed! " + physical_device_result.error().message());
		}

		m_device = device_result.value();

		vkb::SwapchainBuilder swapchain_builder(m_device);

		auto swapchain_result = swapchain_builder.build();
		if (!swapchain_result) {
			throw std::runtime_error("Swapchain creation failed! " + physical_device_result.error().message());
		}

		m_swapchain = swapchain_result.value();

		dbg_log("created");
	}

	Context::~Context() {
		vkb::destroy_swapchain(m_swapchain);
		vkb::destroy_device(m_device);
		if (m_instance.instance != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(m_instance.instance, m_surface, nullptr);
		vkb::destroy_instance(m_instance);
		if(m_window != nullptr)
			glfwDestroyWindow(m_window);

		dbg_log("destroyed");
	}
}