#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
namespace Arieo
{
    class VulkanSemaphore final
        : public Interface::RHI::ISemaphore
    {
    public:
        VulkanSemaphore(VkDevice& vk_device, VkSemaphore&& vk_semaphore)
            : m_vk_device(vk_device), 
            m_vk_semaphore(std::move(vk_semaphore))
        {
            
        }
    private:
        friend class VulkanDevice;
        friend class VulkanSwapchain;
        friend class VulkanRenderCommandQueue;
        friend class VulkanPresentCommandQueue;

        VkDevice& m_vk_device;
        VkSemaphore m_vk_semaphore;
    };
}