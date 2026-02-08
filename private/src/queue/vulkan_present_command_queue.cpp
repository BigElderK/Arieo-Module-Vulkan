#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan.h>
#include <vulkan_core.h>

#include "../vulkan_rhi.h"

namespace Arieo
{
    void VulkanPresentCommandQueue::present(Interface::RHI::ISwapchain* swapchain, std::uint32_t swapchain_image_index, Interface::RHI::IFramebuffer* framebuffer, Interface::RHI::ISemaphore* signal_semaphore)
    {
        VulkanSemaphore* vulkan_signal_semaphore = Base::castInterfaceToInstance<VulkanSemaphore>(signal_semaphore);
        VulkanFramebuffer* vulkan_framebuffer = Base::castInterfaceToInstance<VulkanFramebuffer>(framebuffer);
        VulkanSwapchain* vulkan_swapchain = Base::castInterfaceToInstance<VulkanSwapchain>(swapchain);

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &vulkan_signal_semaphore->m_vk_semaphore;

        VkSwapchainKHR swap_chains[] = {vulkan_swapchain->m_vk_swapchain_khr};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swap_chains;
        present_info.pImageIndices = &swapchain_image_index;        

        present_info.pResults = nullptr; // Optional

        vkQueuePresentKHR(m_vk_queue, &present_info); 
    }
}