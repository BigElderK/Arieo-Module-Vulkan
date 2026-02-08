#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan.h>
#include <vulkan_core.h>

#include "../vulkan_rhi.h"

#if defined(ARIEO_PLATFORM_ANDROID)
#include <vulkan/vulkan_android.h>
namespace Arieo
{
    void VulkanInstance::postProcessInstanceCreateInfo(VkInstanceCreateInfo&, std::vector<const char*>& extension_names)
    {
        extension_names.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
        extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    }

    void VulkanInstance::postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names)
    {
    }

    Interface::RHI::IRenderSurface* VulkanInstance::createSurface(Interface::Window::IWindow* window)
    {
        // Get the ANativeWindow handle from the IWindow interface
        ANativeWindow* native_window = reinterpret_cast<ANativeWindow*>(window->getWindowHandle());
        if (native_window == nullptr)
        {
            Core::Logger::fatal("Failed to get ANativeWindow from IWindow");
            return nullptr;
        }

        VkAndroidSurfaceCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.window = native_window;  // ANativeWindow obtained from AndroidWindow

        VkSurfaceKHR surface;
        VkResult result = vkCreateAndroidSurfaceKHR(m_vk_instance, &create_info, nullptr, &surface);

        if (result != VK_SUCCESS) 
        {
            Core::Logger::fatal("Failed to create Android surface.");
            return nullptr;
        }

        return Base::newT<VulkanSurface>(std::move(surface), window);
    }
}
#endif