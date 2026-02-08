#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
#include "../pipeline/vulkan_pipeline.h"
#include "../framebuffer/vulkan_framebuffer.h"
#include "../buffer/vulkan_buffer.h"
#include "../descriptor/vulkan_descriptor.h"
#include "../image/vulkan_image.h"
namespace Arieo
{
    class VulkanCommandBuffer final
        : public Interface::RHI::ICommandBuffer
    {
    public:
        VulkanCommandBuffer(VkDevice& vk_device, VkCommandBuffer&& vk_command_buffer)
            : m_vk_device(vk_device),
            m_vk_command_buffer(std::move(vk_command_buffer))
        {

        }
    public:
        void reset() override
        {
            vkResetCommandBuffer(m_vk_command_buffer, 0);
        }

        void begin() override
        {
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = 0; // Optional
            begin_info.pInheritanceInfo = nullptr; // Optional

            if(vkBeginCommandBuffer(m_vk_command_buffer, &begin_info) != VK_SUCCESS)
            {
                Core::Logger::error("failed to begin recording command buffer");
            }
        }

        void end() override
        {
            if(vkEndCommandBuffer(m_vk_command_buffer) != VK_SUCCESS)
            {
                Core::Logger::error("failed to begin recording command buffer");
            }
        }

        void beginRenderPass(Interface::RHI::IPipeline* pipeline, Interface::RHI::IFramebuffer* frame_buffer) override
        {
            VulkanPipeline* vulkan_pipeline = Base::castInterfaceToInstance<VulkanPipeline>(pipeline);
            VulkanFramebuffer* vulkan_framebuffer = Base::castInterfaceToInstance<VulkanFramebuffer>(frame_buffer);

            VkRenderPassBeginInfo renderpass_info{};
            renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpass_info.renderPass = vulkan_pipeline->m_vk_render_pass;
            renderpass_info.framebuffer = vulkan_framebuffer->m_vk_framebuffer;

            renderpass_info.renderArea.offset = {0, 0};
            renderpass_info.renderArea.extent = {vulkan_pipeline->m_vk_framebuffer_extent.width, vulkan_pipeline->m_vk_framebuffer_extent.height};

            std::array<VkClearValue, 2> clear_values{};
            clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clear_values[1].depthStencil = {1.0f, 0};

            renderpass_info.clearValueCount = clear_values.size();
            renderpass_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(m_vk_command_buffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
        }
        
        void endRenderPass() override
        {
            vkCmdEndRenderPass(m_vk_command_buffer);
        }

        void bindPipeline(Interface::RHI::IPipeline* pipeline) override
        {
            VulkanPipeline* vulkan_pipeline = Base::castInterfaceToInstance<VulkanPipeline>(pipeline);
            vkCmdBindPipeline(m_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_pipeline->m_vk_pipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(vulkan_pipeline->m_vk_framebuffer_extent.width);
            viewport.height = static_cast<float>(vulkan_pipeline->m_vk_framebuffer_extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(m_vk_command_buffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = {vulkan_pipeline->m_vk_framebuffer_extent.width, vulkan_pipeline->m_vk_framebuffer_extent.height};
            vkCmdSetScissor(m_vk_command_buffer, 0, 1, &scissor);            
        }

        void bindVertexBuffer(Interface::RHI::IBuffer* vertext_buffer, uint32_t offset) override
        {
            VulkanBuffer* vulkan_buffer = Base::castInterfaceToInstance<VulkanBuffer>(vertext_buffer);

            VkBuffer vertex_buffers[] = {vulkan_buffer->m_vk_buffer};
            VkDeviceSize offsets[] = {offset};
            vkCmdBindVertexBuffers(
                m_vk_command_buffer, 0, 1, 
                vertex_buffers, 
                offsets
            );
        }

        void bindIndexBuffer(Interface::RHI::IBuffer* vertext_buffer, uint32_t offset) override
        {
            VulkanBuffer* vulkan_buffer = Base::castInterfaceToInstance<VulkanBuffer>(vertext_buffer);

            VkBuffer vertexBuffers[] = {vulkan_buffer->m_vk_buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindIndexBuffer(
                m_vk_command_buffer,
                vulkan_buffer->m_vk_buffer, 
                offset,
                VK_INDEX_TYPE_UINT16
            );
        }

        void draw(std::uint32_t vertex_count, std::uint32_t instance_count, std::uint32_t first_vertex, std::uint32_t first_instance) override
        {
            vkCmdDraw(m_vk_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
        }

        void drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override
        {
            vkCmdDrawIndexed(m_vk_command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
        }

        void copyBuffer(Interface::RHI::IBuffer* src_buffer, Interface::RHI::IBuffer* dest_buffer, uint32_t size) override
        {
            VulkanBuffer* vulkan_src_buffer = Base::castInterfaceToInstance<VulkanBuffer>(src_buffer);
            VulkanBuffer* vulkan_dest_buffer = Base::castInterfaceToInstance<VulkanBuffer>(dest_buffer);

            VkBufferCopy copy_info{};
            copy_info.srcOffset = 0; // Optional
            copy_info.dstOffset = 0; // Optional
            copy_info.size = size;
            vkCmdCopyBuffer(
                m_vk_command_buffer, 
                vulkan_src_buffer->m_vk_buffer, 
                vulkan_dest_buffer->m_vk_buffer, 
                1, &copy_info);
        }

        void copyBufferToImage(Interface::RHI::IBuffer* buffer, Interface::RHI::IImage* image) override
        {
            VulkanBuffer* vulkan_buffer = Base::castInterfaceToInstance<VulkanBuffer>(buffer);
            VulkanImage* vulkan_image = Base::castInterfaceToInstance<VulkanImage>(image);

            // Change Image layout befor copy
            {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.image = vulkan_image->m_vk_image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                vkCmdPipelineBarrier(
                    m_vk_command_buffer,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }

            // Copy image
            {
                VkBufferImageCopy region = {};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = {0, 0, 0};
                region.imageExtent = {vulkan_image->m_vk_image_extent.width, vulkan_image->m_vk_image_extent.height, 1};

                vkCmdCopyBufferToImage(
                    m_vk_command_buffer, 
                    vulkan_buffer->m_vk_buffer,
                    vulkan_image->m_vk_image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                    1, 
                    &region
                );
            }

            // Chanage Image layout after copy
            {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.image = vulkan_image->m_vk_image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(
                    m_vk_command_buffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }
        }

        void prepareDepthImage(Interface::RHI::IImage* depth_image) override
        {
            VulkanImage* vulkan_depth_image = Base::castInterfaceToInstance<VulkanImage>(depth_image);
            // Chanage Image layout after copy
            {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.image = vulkan_depth_image->m_vk_image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                if(vulkan_depth_image->m_vk_image_format == VK_FORMAT_D32_SFLOAT_S8_UINT 
                || vulkan_depth_image->m_vk_image_format == VK_FORMAT_D24_UNORM_S8_UINT 
                )
                {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }

                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                vkCmdPipelineBarrier(
                    m_vk_command_buffer,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }
        }

        void bindDescriptorSets(Interface::RHI::IPipeline* pipeline, Interface::RHI::IDescriptorSet* descriptor_set) override
        {
            VulkanPipeline* vulkan_pipeline = Base::castInterfaceToInstance<VulkanPipeline>(pipeline);
            VulkanDescriptorSet* vulkan_descriptor_set = Base::castInterfaceToInstance<VulkanDescriptorSet>(descriptor_set);

            vkCmdBindDescriptorSets(
                m_vk_command_buffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                vulkan_pipeline->m_vk_pipeline_layout, 
                0, 
                1, 
                &vulkan_descriptor_set->m_vk_descriptor_set, 
                0, 
                nullptr
            );
        }
    private:
        friend class VulkanCommandPool;
        friend class VulkanRenderCommandQueue;

        VkDevice& m_vk_device;
        VkCommandBuffer m_vk_command_buffer;
    };

    class VulkanCommandPool final
        : public Interface::RHI::ICommandPool
    {
    public:
        VulkanCommandPool(VkDevice& vk_device, VkCommandPool&& vk_command_pool)
            : m_vk_device(vk_device),
            m_vk_command_pool(std::move(vk_command_pool))
        {

        }

        Interface::RHI::ICommandBuffer* allocateCommandBuffer() override
        {
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = m_vk_command_pool;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandBufferCount = 1;

            VkCommandBuffer vk_command_buffer;
            if (vkAllocateCommandBuffers(m_vk_device, &alloc_info, &vk_command_buffer) != VK_SUCCESS) 
            {
                Core::Logger::error("failed to allocate command buffers!");
            } 
            
            return Base::newT<VulkanCommandBuffer>(m_vk_device, std::move(vk_command_buffer));
        }

        void freeCommandBuffer(Interface::RHI::ICommandBuffer* command_buffer) override
        {
            VulkanCommandBuffer* vulkan_command_buffer = Base::castInterfaceToInstance<VulkanCommandBuffer>(command_buffer);
            vkFreeCommandBuffers(m_vk_device, m_vk_command_pool, 1, &vulkan_command_buffer->m_vk_command_buffer);
        }
    private:
        friend class VulkanDevice;
        friend class VulkanRenderCommandQueue;
        friend class VulkanPresentCommandQueue;
        VkDevice& m_vk_device;
        VkCommandPool m_vk_command_pool;
    };

}