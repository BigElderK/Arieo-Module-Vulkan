#pragma once
#include "interface/rhi/rhi.h"
#include "../image/vulkan_image.h"
#include <vulkan.h>
namespace Arieo
{
    class VulkanSwapchain final
        : public Interface::RHI::ISwapchain
    {
    public:
        VulkanSwapchain(
            VkDevice& vk_device, 
            VkSwapchainCreateInfoKHR& vk_swapchain_khr_create_info, 
            VkSwapchainKHR&& vk_swapchain_khr)
            : m_vk_device(vk_device),
            m_vk_swapchain_khr(std::move(vk_swapchain_khr)),
            m_extent(0, 0, vk_swapchain_khr_create_info.imageExtent.width, vk_swapchain_khr_create_info.imageExtent.height)
        {
        }
        
        std::uint32_t acquireNextImageIndex(Interface::RHI::ISemaphore* semaphore) override
        {
            VulkanSemaphore* vulkan_semaphore = Base::castInterfaceToInstance<VulkanSemaphore>(semaphore);
            std::uint32_t image_index = std::numeric_limits<std::uint32_t>::max();

            VkResult result = vkAcquireNextImageKHR(
                m_vk_device, 
                m_vk_swapchain_khr, 
                UINT64_MAX, 
                vulkan_semaphore->m_vk_semaphore, 
                VK_NULL_HANDLE, &image_index);
            if(result != VK_SUCCESS)
            {
                if(result == VK_ERROR_OUT_OF_DATE_KHR)
                {
                    Core::Logger::warn("Swapchain lost: {}", VulkanUtility::covertVkResultToString(result));
                    m_is_lost = true;
                    return std::numeric_limits<uint32_t>::max();
                }
                else if(result == VK_SUBOPTIMAL_KHR)
                {
                    m_is_lost = true;
                    Core::Logger::warn("Swapchain changed: {}", VulkanUtility::covertVkResultToString(result));
                    return image_index;
                }
                else
                {
                    Core::Logger::error("Failed to acquire next image khr {}", VulkanUtility::covertVkResultToString(result));
                    return std::numeric_limits<uint32_t>::max();
                }
            }
            return image_index;
        }
        
        std::vector<Interface::RHI::IImageView*>& getImageViews() override
        {
            return m_image_view_array;
        }

        Base::Math::Rect<size_t>& getExtent() override
        {
            return m_extent;
        }

        bool isLost() override
        {
            return m_is_lost;
        }
    private:
        friend class VulkanDevice;
        friend class VulkanPresentCommandQueue;
        friend class VulkanCommandBuffer;

        bool m_is_lost = false;

        Base::Math::Rect<size_t> m_extent;

        VkDevice& m_vk_device;
        VkSwapchainKHR m_vk_swapchain_khr;

        std::vector<Interface::RHI::IImage*> m_image_resource_array;
        std::vector<Interface::RHI::IImageView*> m_image_view_array;
    };
}