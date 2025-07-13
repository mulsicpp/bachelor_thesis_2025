#include "Context.h"

#include "utils/defines.h"

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
	Context::Context(GLFWwindow* window, const char* app_name) {

		m_window = window;

		vkb::InstanceBuilder instance_builder;

		instance_builder.set_app_name(app_name);

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

		QueueInfo queue_info = QueueInfo::get_from_physical_device(m_physical_device);



		vkb::DeviceBuilder device_builder(m_physical_device);

		device_builder.custom_queue_setup(queue_info.get_custom_queue_descriptions());

		auto device_result = device_builder.build();
		if (!device_result) {
			throw std::runtime_error("Device creation failed! " + device_result.error().message());
		}

		m_device = device_result.value();

		m_swapchain = Swapchain::create(m_device);
		m_command_manager = CommandManager::create(m_device, queue_info);

		dbg_log("created");
	}

	Context::~Context() {
		CommandManager::destroy(m_command_manager);
		Swapchain::destroy(m_swapchain);

		vkb::destroy_device(m_device);
		if (m_instance.instance != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(m_instance.instance, m_surface, nullptr);
		vkb::destroy_instance(m_instance);

		dbg_log("destroyed");
	}
}