#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
#include "../queue/vulkan_render_command_queue.h"
#include "../queue/vulkan_present_command_queue.h"

#include <vk_mem_alloc.h>

namespace Arieo
{
    class VulkanDevice final
        : public Interface::RHI::IRenderDevice
    {
    public:
        friend class VulkanInstance;
        VulkanDevice(
            VkPhysicalDevice&& vk_phys_device, 
            VkDevice&& vk_device,
            ::VmaAllocator&& vma_allocator,
            std::uint32_t vk_graphics_queue_index, 
            std::uint32_t vk_present_queue_index, 
            VkQueue&& vk_graphics_queue, 
            VkQueue&& vk_present_queue)
            :  m_vk_phys_device(vk_phys_device), 
            m_vma_allocator(std::move(vma_allocator)),
            m_vk_device(vk_device),
            m_graphic_queue_index(vk_graphics_queue_index),
            m_present_queue_index(vk_present_queue_index),
            m_graphics_queue(m_vk_device, vk_graphics_queue_index, std::move(vk_graphics_queue)), 
            m_present_queue(m_vk_device, vk_graphics_queue_index, std::move(vk_present_queue))
        {
            vkGetPhysicalDeviceProperties(m_vk_phys_device, &m_vk_phys_device_properties);
        }

        Interface::RHI::Format findSupportedFormat(const std::vector<Interface::RHI::Format>& candidate_formats, Interface::RHI::ImageTiling, Interface::RHI::FormatFeatureFlags) override;

        Interface::RHI::IRenderCommandQueue* getGraphicsCommandQueue() override
        {
            return &m_graphics_queue;
        }
        Interface::RHI::IPresentCommandQueue* getPresentCommandQueue() override
        {
            return &m_present_queue;
        }

        Interface::RHI::ISwapchain* createSwapchain(Interface::RHI::IRenderSurface*) override;
        void destroySwapchain(Interface::RHI::ISwapchain*) override;

        Interface::RHI::IFramebuffer* createFramebuffer(Interface::RHI::IPipeline*, Interface::RHI::ISwapchain* swapchain, const std::vector<Interface::RHI::IImageView*>& attachment_array) override;
        void destroyFramebuffer(Interface::RHI::IFramebuffer*) override;

        Interface::RHI::IShader* createShader(void* buf, size_t buf_size) override;
        void destroyShader(Interface::RHI::IShader*) override;

        Interface::RHI::IPipeline* createPipeline(Interface::RHI::IShader* vert_shader, Interface::RHI::IShader* frag_shader, Interface::RHI::IImageView* target_color_attachment, Interface::RHI::IImageView* target_depth_attachment) override;
        void destroyPipeline(Interface::RHI::IPipeline*) override;

        Interface::RHI::IFence* createFence() override;
        void destroyFence(Interface::RHI::IFence*) override;

        Interface::RHI::ISemaphore* createSemaphore() override;
        void destroySemaphore(Interface::RHI::ISemaphore*) override;

        Interface::RHI::IBuffer* createBuffer(size_t size, Interface::RHI::BufferUsageBitFlags buffer_usage, Interface::RHI::BufferAllocationFlags allocation_flag, Interface::RHI::MemoryUsage memory_usage) override;
        void destroyBuffer(Interface::RHI::IBuffer*) override;

        Interface::RHI::IDescriptorPool* createDescriptorPool(size_t capacity) override;
        void destroyDescriptorPool(Interface::RHI::IDescriptorPool*) override;

        Interface::RHI::IImage* createImage(std::uint32_t width, std::uint32_t height, Interface::RHI::Format format, Interface::RHI::ImageAspectFlags aspect, Interface::RHI::ImageTiling tiling, Interface::RHI::ImageUsageFlags usage, Interface::RHI::MemoryUsage mem_usage) override;
        void destroyImage(Interface::RHI::IImage*) override;

        void waitIdle() override;
    private:
        VkDevice m_vk_device;

        ::VmaAllocator m_vma_allocator;

        VkPhysicalDevice m_vk_phys_device; 
        VulkanRenderCommandQueue m_graphics_queue;
        VulkanPresentCommandQueue m_present_queue;

        std::uint32_t m_graphic_queue_index;
        std::uint32_t m_present_queue_index;

        VkPhysicalDeviceProperties m_vk_phys_device_properties{};
    };
}