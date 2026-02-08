#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
namespace Arieo
{
    class VulkanSurface final
        : public Interface::RHI::IRenderSurface
    {
    public:
        friend class VulkanInstance;
        friend class VulkanDevice;
        VulkanSurface(VkSurfaceKHR&& vk_surface_khr, Interface::Window::IWindow* attached_window)
            : m_vk_surface_khr(std::move(vk_surface_khr)),
            m_attached_window(attached_window)
        {
        }

        Interface::Window::IWindow* getAttachedWindow() override
        {
            return m_attached_window;
        }
    private:
        VkSurfaceKHR m_vk_surface_khr;
        Interface::Window::IWindow* m_attached_window = nullptr;
        Base::Math::Vector<std::uint32_t, 2> m_extent;
    };
}