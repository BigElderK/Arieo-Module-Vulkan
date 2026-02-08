#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
#include "../common/vulkan_utility.h"
#include <vk_mem_alloc.h>
namespace Arieo
{
    class VulkanBuffer final
        : public Interface::RHI::IBuffer
    {
    public:
        VulkanBuffer(VkBuffer&& vk_buffer, VmaAllocator& vma_alloator, VmaAllocation&& vma_allocation)
            : m_vk_buffer(std::move(vk_buffer)), 
            m_vma_alloator(vma_alloator),
            m_vma_allocation(std::move(vma_allocation))
        {

        }

        void* mapMemory(size_t offset, size_t size) override
        {
            void* mapped_ptr = nullptr;
            VkResult vk_result = vmaMapMemory(m_vma_alloator, m_vma_allocation, &mapped_ptr);
            if(vk_result != VK_SUCCESS)
            {
                Core::Logger::error("Map buffer memory failed: {}", VulkanUtility::covertVkResultToString(vk_result));
                return nullptr;
            }
            return mapped_ptr;
        }

        void unmapMemory() override
        {
            vmaUnmapMemory(m_vma_alloator, m_vma_allocation);
        }
    private:
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;
        friend class VulkanDescriptorSet;
        friend class VulkanImage;

        VkBuffer m_vk_buffer;

        VmaAllocator m_vma_alloator;
        VmaAllocation m_vma_allocation;
    };
}