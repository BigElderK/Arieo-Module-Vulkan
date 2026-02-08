
#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
namespace Arieo
{
    class VulkanFramebuffer final
        : public Interface::RHI::IFramebuffer
    {
    public:
        VulkanFramebuffer(VkFramebuffer&& vk_framebuffer)
            :
            m_vk_framebuffer(std::move(vk_framebuffer))
        {

        }
    private:
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;
        friend class VulkanPresentCommandQueue;

        VkFramebuffer m_vk_framebuffer;
    };
}