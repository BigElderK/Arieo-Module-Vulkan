#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan.h>
#include <vulkan_core.h>

#include "../vulkan_rhi.h"

namespace Arieo
{
    void VulkanRenderCommandQueue::submitCommand(Interface::RHI::ICommandBuffer* command_buffer, Interface::RHI::IFence* fence, Interface::RHI::ISemaphore* wait_semaphore, Interface::RHI::ISemaphore* signal_semaphore)
    {
        VulkanCommandBuffer* vulkan_command_buffer = Base::castInterfaceToInstance<VulkanCommandBuffer>(command_buffer);
        VulkanSemaphore* vulkan_wait_semaphore = Base::castInterfaceToInstance<VulkanSemaphore>(wait_semaphore);
        VulkanSemaphore* vulkan_signal_semaphore = Base::castInterfaceToInstance<VulkanSemaphore>(signal_semaphore);
        VulkanFence* vulkan_fence = Base::castInterfaceToInstance<VulkanFence>(fence);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {vulkan_wait_semaphore->m_vk_semaphore};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vulkan_command_buffer->m_vk_command_buffer;

        VkSemaphore signal_Semaphores[] = {vulkan_signal_semaphore->m_vk_semaphore};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_Semaphores;

        VkResult result = vkQueueSubmit(m_vk_queue, 1, &submit_info, vulkan_fence->m_vk_fence); 
        if (result != VK_SUCCESS) 
        {
            Core::Logger::error("failed to submit command buffer {}", VulkanUtility::covertVkResultToString(result));
        }
    }    

    void VulkanRenderCommandQueue::submitCommand(Interface::RHI::ICommandBuffer* command_buffer)
    {
        VkSubmitInfo submit_info{};
        VulkanCommandBuffer* vulkan_command_buffer = Base::castInterfaceToInstance<VulkanCommandBuffer>(command_buffer);
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vulkan_command_buffer->m_vk_command_buffer;

        VkResult result = vkQueueSubmit(m_vk_queue, 1, &submit_info, VK_NULL_HANDLE); 
        if (result != VK_SUCCESS) 
        {
            Core::Logger::error("failed to submit command buffer {}", VulkanUtility::covertVkResultToString(result));
        }
    }
}