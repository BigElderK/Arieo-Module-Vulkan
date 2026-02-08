#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
#include "../command/vulkan_command.h"
namespace Arieo
{
    class VulkanRenderCommandQueue final
        : public Interface::RHI::IRenderCommandQueue
    {
    public:
        friend class VulkanDevice;
        VulkanRenderCommandQueue(VkDevice& vk_device, std::uint32_t queue_family_index, VkQueue&& vk_queue)
            : m_vk_device(vk_device), 
            m_queue_family_index(queue_family_index),
            m_vk_queue(std::move(vk_queue))
        {
        }

        Interface::RHI::ICommandPool* createCommandPool() override
        {
            VkCommandPoolCreateInfo pool_info{};
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_info.queueFamilyIndex = m_queue_family_index;

            VkCommandPool vk_command_pool;
            if (vkCreateCommandPool(m_vk_device, &pool_info, nullptr, &vk_command_pool) != VK_SUCCESS) 
            {
                Core::Logger::error("failed to create command pool");
                return nullptr;
            }

            return Base::newT<VulkanCommandPool>(m_vk_device, std::move(vk_command_pool));
        }

        void destroyCommandPool(Interface::RHI::ICommandPool* command_pool) override
        {
            VulkanCommandPool* vulkan_command_pool = Base::castInterfaceToInstance<VulkanCommandPool>(command_pool);
            vkDestroyCommandPool(m_vk_device, vulkan_command_pool->m_vk_command_pool, nullptr);
            
            return Base::deleteT(vulkan_command_pool);
        }

        void waitIdle() override
        {
            vkQueueWaitIdle(m_vk_queue);
        }

        void submitCommand(Interface::RHI::ICommandBuffer* command_buffer, Interface::RHI::IFence* fence, Interface::RHI::ISemaphore* wait_semaphore, Interface::RHI::ISemaphore* signal_semaphore) override;
        void submitCommand(Interface::RHI::ICommandBuffer* command_buffer) override;
    private:
        std::uint32_t m_queue_family_index;
        VkDevice& m_vk_device;
        VkQueue m_vk_queue;
    };
}