#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#include <vector>

#define WIDTH 800
#define HEIGHT 600

#define THROW(x) throw std::runtime_error(x)

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#if NDEBUG 
#define VALIDATION_LAYERS false
#else
#define VALIDATION_LAYERS true
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

class TestApp {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		terminate();
	}

private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	void initWindow() {
		if (!glfwInit()) {
			THROW("Failed to initialize GLFW\n");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Test App", nullptr, nullptr);

		if (!window) {
			glfwTerminate();
			THROW("Failed to create window\n");
		}
	}

	void initVulkan() {
		createInstance();
#if VALIDATION_LAYERS
		setupDebugMessenger();
#endif
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if VALIDATION_LAYERS
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return extensions;
	}

	void createInstance() {
		if (VALIDATION_LAYERS && !checkValidationLayerSupport()) {
			THROW("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Test App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

#if VALIDATION_LAYERS
		createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
#endif

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			THROW("failed to create instance!");
		}
		printf("created instance\n");
	}

	void setupDebugMessenger() {
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			THROW("failed to set up debug messenger!");
		}
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void terminate() {
#if VALIDATION_LAYERS
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}
};

void printAvailableExtensions(void) {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	printf("available extensions:\n");

	for (const auto& extension : extensions) {
		printf("- %s\n", extension.extensionName);
	}
	printf("\n");
}

int main(void) {
	printAvailableExtensions();

	TestApp app;

	try {

		app.run();
	}
	catch (const std::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}

	return 0;
}