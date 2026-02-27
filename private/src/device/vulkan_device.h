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
            : m_vk_device(vk_device),
            m_vma_allocator(std::move(vma_allocator)),
            m_vk_phys_device(vk_phys_device),
            m_graphics_queue(m_vk_device, vk_graphics_queue_index, std::move(vk_graphics_queue)),
            m_present_queue(m_vk_device, vk_present_queue_index, std::move(vk_present_queue)),
            m_graphic_queue_index(vk_graphics_queue_index),
            m_present_queue_index(vk_present_queue_index)
        {
            vkGetPhysicalDeviceProperties(m_vk_phys_device, &m_vk_phys_device_properties);
        }

        Interface::RHI::Format findSupportedFormat(const std::vector<Interface::RHI::Format>& candidate_formats, Interface::RHI::ImageTiling, Interface::RHI::FormatFeatureFlags) override;

        Base::Interop<Interface::RHI::IRenderCommandQueue> getGraphicsCommandQueue() override
        {
            return m_graphics_queue.queryInterface<Interface::RHI::IRenderCommandQueue>();
        }
        Base::Interop<Interface::RHI::IPresentCommandQueue> getPresentCommandQueue() override
        {
            return m_present_queue.queryInterface<Interface::RHI::IPresentCommandQueue>();
        }

        Base::Interop<Interface::RHI::ISwapchain> createSwapchain(Base::Interop<Interface::RHI::IRenderSurface>) override;
        void destroySwapchain(Base::Interop<Interface::RHI::ISwapchain>) override;

        Base::Interop<Interface::RHI::IFramebuffer> createFramebuffer(Base::Interop<Interface::RHI::IPipeline>, Base::Interop<Interface::RHI::ISwapchain> swapchain, std::vector<Base::Interop<Interface::RHI::IImageView>>& attachment_array) override;
        void destroyFramebuffer(Base::Interop<Interface::RHI::IFramebuffer>) override;

        Base::Interop<Interface::RHI::IShader> createShader(void* buf, size_t buf_size) override;
        void destroyShader(Base::Interop<Interface::RHI::IShader>) override;

        Base::Interop<Interface::RHI::IPipeline> createPipeline(Base::Interop<Interface::RHI::IShader> vert_shader, Base::Interop<Interface::RHI::IShader> frag_shader, Base::Interop<Interface::RHI::IImageView> target_color_attachment, Base::Interop<Interface::RHI::IImageView> target_depth_attachment) override;
        void destroyPipeline(Base::Interop<Interface::RHI::IPipeline>) override;

        Base::Interop<Interface::RHI::IFence> createFence() override;
        void destroyFence(Base::Interop<Interface::RHI::IFence>) override;

        Base::Interop<Interface::RHI::ISemaphore> createSemaphore() override;
        void destroySemaphore(Base::Interop<Interface::RHI::ISemaphore>) override;

        Base::Interop<Interface::RHI::IBuffer> createBuffer(size_t size, Interface::RHI::BufferUsageBitFlags buffer_usage, Interface::RHI::BufferAllocationFlags allocation_flag, Interface::RHI::MemoryUsage memory_usage) override;
        void destroyBuffer(Base::Interop<Interface::RHI::IBuffer>) override;

        Base::Interop<Interface::RHI::IDescriptorPool> createDescriptorPool(size_t capacity) override;
        void destroyDescriptorPool(Base::Interop<Interface::RHI::IDescriptorPool>) override;

        Base::Interop<Interface::RHI::IImage> createImage(std::uint32_t width, std::uint32_t height, Interface::RHI::Format format, Interface::RHI::ImageAspectFlags aspect, Interface::RHI::ImageTiling tiling, Interface::RHI::ImageUsageFlags usage, Interface::RHI::MemoryUsage mem_usage) override;
        void destroyImage(Base::Interop<Interface::RHI::IImage>) override;

        void waitIdle() override;
    private:
        VkDevice m_vk_device;

        ::VmaAllocator m_vma_allocator;

        VkPhysicalDevice m_vk_phys_device; 
        Base::Instance<VulkanRenderCommandQueue> m_graphics_queue;
        Base::Instance<VulkanPresentCommandQueue> m_present_queue;

        std::uint32_t m_graphic_queue_index;
        std::uint32_t m_present_queue_index;

        VkPhysicalDeviceProperties m_vk_phys_device_properties{};
    };
}