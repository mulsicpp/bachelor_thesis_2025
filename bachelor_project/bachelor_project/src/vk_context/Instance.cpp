#include "Instance.h"

#include <cstdio>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include "../utils/defines.h"
#include "../utils/dbg_log.h"
#include "../vk_ext/debug_utils.h"

static const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

static const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MSG_CREATE_INFO = { 
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debugCallback
};

bool validation_supported() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : VALIDATION_LAYERS) {
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

namespace vk {
	Instance::Instance(const char* app_name) : ::utils::NoCopy{} {

        if (USE_VALIDATION_LAYERS && !validation_supported()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = app_name;
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        auto extensions = get_extensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        if (USE_VALIDATION_LAYERS) {
            create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DEBUG_MSG_CREATE_INFO;
        }
        else {
            create_info.enabledLayerCount = 0;

            create_info.pNext = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

        dbg_log("created instance");

        create_debug_messanger();
        dbg_log("created debug messanger");
	}

    Instance::~Instance() {
        debug_utils::vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messanger, nullptr);
        dbg_log("destroyed debug messanger");
        vkDestroyInstance(m_instance, nullptr);
        dbg_log("destroyed instance");
    }

    std::vector<const char*> Instance::get_extensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (USE_VALIDATION_LAYERS) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Instance::create_debug_messanger() {
        if (!USE_VALIDATION_LAYERS) return;

        debug_utils::load(m_instance);

        if (debug_utils::vkCreateDebugUtilsMessengerEXT(m_instance, &DEBUG_MSG_CREATE_INFO, nullptr, &m_debug_messanger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
}