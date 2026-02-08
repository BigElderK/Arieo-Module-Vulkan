#pragma once
#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
#include "../image/vulkan_image.h"
namespace Arieo
{
    class VulkanDescriptorSet final
        : public Interface::RHI::IDescriptorSet
    {
    public:
        VulkanDescriptorSet(VkDevice& vk_device, VkDescriptorSet&& vk_descriptor_set)
            : m_vk_device(vk_device),
            m_vk_descriptor_set(std::move(vk_descriptor_set))
        {
        }

        void bindBuffer(size_t bind_index, Interface::RHI::IBuffer* buffer, size_t offset, size_t size) override
        {
            VulkanBuffer* vulkan_buffer = Base::castInterfaceToInstance<VulkanBuffer>(buffer);
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vulkan_buffer->m_vk_buffer;
            bufferInfo.offset = offset;
            bufferInfo.range = size;

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_vk_descriptor_set;
            descriptor_write.dstBinding = bind_index;
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_vk_device, 1, &descriptor_write, 0, nullptr);
        }

        void bindImage(size_t bind_index, Interface::RHI::IImage* image) override
        {
            VulkanImage* vulkan_image = Base::castInterfaceToInstance<VulkanImage>(image);
            VkDescriptorImageInfo image_info{};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = vulkan_image->m_vulkan_image_view.m_vk_image_view;
            image_info.sampler = vulkan_image->m_vulkan_image_sampler.m_vk_image_sampler;

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_vk_descriptor_set;
            descriptor_write.dstBinding = bind_index;
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pImageInfo = &image_info;

            vkUpdateDescriptorSets(m_vk_device, 1, &descriptor_write, 0, nullptr);
        }
    private:
        friend class VulkanDescriptorPool;
        friend class VulkanCommandBuffer;
        VkDevice& m_vk_device;
        VkDescriptorSet m_vk_descriptor_set;
    };

    class VulkanDescriptorPool final
        : public Interface::RHI::IDescriptorPool
    {
    public:
        VulkanDescriptorPool(VkDevice& vk_device, VkDescriptorPool&& vk_descriptor_pool)
            : m_vk_device(vk_device),
            m_vk_descriptor_pool(std::move(vk_descriptor_pool))
        {

        }

        Interface::RHI::IDescriptorSet* allocateDescriptorSet(Interface::RHI::IPipeline* pipeline)
        {
            VulkanPipeline* vulkan_pipeline = Base::castInterfaceToInstance<VulkanPipeline>(pipeline);
            
            VkDescriptorSetAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = m_vk_descriptor_pool;
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = &vulkan_pipeline->m_vk_descriptor_set_layout;

            VkDescriptorSet vk_descriptor_set;
            VkResult result = vkAllocateDescriptorSets(m_vk_device, &alloc_info, &vk_descriptor_set);
            if (result != VK_SUCCESS) 
            {
                Core::Logger::error("Create allocate descriptor sets failed: {}", VulkanUtility::covertVkResultToString(result));
            }
            return Base::newT<VulkanDescriptorSet>(m_vk_device, std::move(vk_descriptor_set));
        }

        void freeDescriptorSet(Interface::RHI::IDescriptorSet* descriptor_set)
        {
            VulkanDescriptorSet* vulkan_desc_set = Base::castInterfaceToInstance<VulkanDescriptorSet>(descriptor_set);
            vkFreeDescriptorSets(m_vk_device, m_vk_descriptor_pool, 1, &vulkan_desc_set->m_vk_descriptor_set);
            Base::deleteT(vulkan_desc_set);
        }
    private:
        friend class VulkanDevice;
        VkDevice& m_vk_device;
        VkDescriptorPool m_vk_descriptor_pool;
    };
}