#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan.h>
#include <vulkan_core.h>

#include "../vulkan_rhi.h"

#if defined(ARIEO_PLATFORM_WINDOWS)
#include <windows.h>
#include <vulkan/vulkan_win32.h>

namespace Arieo
{
    void VulkanInstance::postProcessInstanceCreateInfo(VkInstanceCreateInfo&, std::vector<const char*>& extension_names)
    {
        extension_names.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    }

    void VulkanInstance::postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names)
    {

    }

    Base::Interface<Interface::RHI::IRenderSurface> VulkanInstance::createSurface(Base::Interface<Interface::Window::IWindowManager> window_manager, Base::Interface<Interface::Window::IWindow> window)
    {
        VkWin32SurfaceCreateInfoKHR create_info{};

        create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        create_info.hinstance = reinterpret_cast<HMODULE>(window_manager->getDisplay());
        create_info.hwnd = reinterpret_cast<HWND>(window->getWindowHandle());

        VkSurfaceKHR surface;
        VkResult result = vkCreateWin32SurfaceKHR(m_vk_instance, &create_info, nullptr, &surface);
        if (result != VK_SUCCESS) 
        {
            Core::Logger::error("Failed to create Win32 surface. {}", VulkanUtility::covertVkResultToString(result));
            return nullptr;
        }

        return Base::Interface<Interface::RHI::IRenderSurface>::createAs<VulkanSurface>(std::move(surface), window);
    }
}
#endif