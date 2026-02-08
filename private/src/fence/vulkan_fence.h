#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
namespace Arieo
{
    class VulkanFence final
        : public Interface::RHI::IFence
    {
    public:
        VulkanFence(VkDevice& vk_device, VkFence&& vk_fence)
            : m_vk_device(vk_device), 
            m_vk_fence(std::move(vk_fence))
        {

        }

        void wait() override
        {
            vkWaitForFences(m_vk_device, 1, &m_vk_fence, VK_TRUE, UINT64_MAX);
        }

        void reset() override
        {
            vkResetFences(m_vk_device, 1, &m_vk_fence);
        }
    private:
        friend class VulkanDevice;
        friend class VulkanRenderCommandQueue;

        VkDevice& m_vk_device;
        VkFence m_vk_fence;
    };
}