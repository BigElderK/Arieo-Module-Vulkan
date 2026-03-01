#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>

#include <vk_mem_alloc.h>
namespace Arieo
{
    class VulkanImage;
    class VulkanImageView final
        : public Interface::RHI::IImageView
    {
    public:
        VulkanImageView(VulkanImage& vulkan_image, VkImageView&& vk_image_view)
            : 
            m_vulkan_image(vulkan_image),
            m_vk_image_view(std::move(vk_image_view))
        {

        }
    private:
        friend class VulkanDevice;
        friend class VulkanDescriptorSet;
        VulkanImage& m_vulkan_image;
        VkImageView m_vk_image_view;
    };

    class VulkanImageSampler final
        : public Interface::RHI::IImageSampler
    {
    public:
        VulkanImageSampler(VkSampler&& vk_sampler)
            : 
            // m_vulkan_image(vulkan_image),
            m_vk_image_sampler(std::move(vk_sampler))
        {

        }
    private:
        friend class VulkanDevice;
        friend class VulkanDescriptorSet;
        // VulkanImage& m_vulkan_image;
        VkSampler m_vk_image_sampler;
    };

    class VulkanImage final
        : public Interface::RHI::IImage
    {
    public:
        VulkanImage(VkImage&& vk_image, VkImageView&& vk_image_view, VkSampler&& vk_sampler, VmaAllocation&& vma_allocation, VmaAllocationInfo&& vma_allocation_info, VkExtent3D image_extent, VkFormat image_format)
            : 
            // m_vk_device(vk_device),
            m_vma_allocation(std::move(vma_allocation)),
            m_vma_allocation_info(std::move(vma_allocation_info)),
            m_vk_image_extent(image_extent),
            m_vk_image_format(image_format),
            m_vk_image(std::move(vk_image)),
            m_vulkan_image_view(*this, std::move(vk_image_view)),
            m_vulkan_image_sampler(std::move(vk_sampler))
        {
            
        }

        size_t getMemorySize() override
        {
            return m_vma_allocation_info.size;
        }

        Base::Interop::RawRef<Interface::RHI::IImageView> getImageView() override
        {
            return m_vulkan_image_view.queryInterface<Interface::RHI::IImageView>();
        }

        Base::Interop::RawRef<Interface::RHI::IImageSampler> getImageSampler() override
        {
            return m_vulkan_image_sampler.queryInterface<Interface::RHI::IImageSampler>();
        }
    private:
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;
        friend class VulkanDescriptorSet;

        // VkDevice& m_vk_device;
        VmaAllocation m_vma_allocation;
        VmaAllocationInfo m_vma_allocation_info;

        VkExtent3D m_vk_image_extent;
        VkFormat m_vk_image_format;
        VkImage m_vk_image;
        
        Base::Interop::Instance<VulkanImageView> m_vulkan_image_view;
        Base::Interop::Instance<VulkanImageSampler> m_vulkan_image_sampler;
    };
}




