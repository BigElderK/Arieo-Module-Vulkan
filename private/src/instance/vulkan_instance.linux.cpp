#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan.h>
#include <vulkan_core.h>

#include "../vulkan_rhi.h"

#if defined(ARIEO_PLATFORM_LINUX)
#include "X11/Xlib.h"
#include <vulkan/vulkan_xlib.h>
namespace Arieo
{
    void VulkanInstance::postProcessInstanceCreateInfo(VkInstanceCreateInfo&, std::vector<const char*>& extension_names)
    {
        extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
        extension_names.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    }

    void VulkanInstance::postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names)
    {

    }

    Interface::RHI::IRenderSurface* VulkanInstance::createSurface(Interface::Window::IWindow* window)
    {
        if(window->getWindowPlatform() == Base::MakeStringID("x11"))
        {
            VkXlibSurfaceCreateInfoKHR create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            create_info.dpy = reinterpret_cast<Display*>(window->getWindowManager()->getDisplay());
            create_info.window = reinterpret_cast<Window>(window->getWindowHandle());

            VkSurfaceKHR surface;
            VkResult result = vkCreateXlibSurfaceKHR(m_vk_instance, &create_info, nullptr, &surface);  
            if (result != VK_SUCCESS) 
            {
                Core::Logger::fatal("Failed to create X11 surface. {}", VulkanUtility::covertVkResultToString(result));
                return nullptr;
            }

            return Base::newT<VulkanSurface>(std::move(surface), window);
        }
        else if(window->getWindowPlatform() == Base::MakeStringID("wayland"))
        {
            //TODO:
        }

        return nullptr;
    }
}
#endif