#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan.h>
#include <vulkan_core.h>

#include "../vulkan_rhi.h"

#if defined(ARIEO_PLATFORM_APPLE)
#include <vulkan/vulkan.h>  // Core Vulkan
#include <vulkan/vulkan_metal.h>  // For Metal surface extension
#include <vulkan/vulkan_macos.h>

namespace Arieo
{
    void VulkanInstance::postProcessInstanceCreateInfo(VkInstanceCreateInfo& vk_instance_create_info, std::vector<const char*>& extension_names)
    {
        vk_instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        extension_names.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        //extension_names.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
        extension_names.emplace_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    }

    void VulkanInstance::postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names)
    {
        extension_names.emplace_back("VK_KHR_portability_subset");
    }

    Interface::RHI::IRenderSurface* VulkanInstance::createSurface(Interface::Window::IWindow* window)
    {
        VkSurfaceKHR surface;

        CAMetalLayer* meta_layer = reinterpret_cast<CAMetalLayer*>(window->getWindowHandle()); 
        VkMetalSurfaceCreateInfoEXT sci = {};
        sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        sci.pLayer = meta_layer;

        VkResult result = vkCreateMetalSurfaceEXT(m_vk_instance, &sci, nullptr, &surface);
        if (result != VK_SUCCESS) 
        {
            Core::Logger::error("Failed to create window surface: {}", VulkanUtility::covertVkResultToString(result));
            return nullptr;
        }

        return Base::newT<VulkanSurface>(std::move(surface), window);
    }
}
#endif