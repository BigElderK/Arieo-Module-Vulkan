#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>
namespace Arieo
{
    class VulkanPipeline final
        : public Interface::RHI::IPipeline
    {
    public:
        friend class VulkanDevice;
        VulkanPipeline(VkPipeline&& vk_pipeline, VkPipelineLayout&& vk_pipeline_layout, VkDescriptorSetLayout vk_descriptor_set_layout, VkRenderPass&& vk_render_pass, VkExtent3D vk_framebuffer_extent)
            : m_vk_pipeline(std::move(vk_pipeline)), 
            m_vk_pipeline_layout(std::move(vk_pipeline_layout)),
            m_vk_render_pass(std::move(vk_render_pass)),
            m_vk_descriptor_set_layout(std::move(vk_descriptor_set_layout)),
            m_vk_framebuffer_extent(vk_framebuffer_extent)
        {

        }
    private:
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;
        friend class VulkanDescriptorPool;

        VkPipeline m_vk_pipeline;
        VkPipelineLayout m_vk_pipeline_layout;
        VkRenderPass m_vk_render_pass;
        VkExtent3D m_vk_framebuffer_extent;
        VkDescriptorSetLayout m_vk_descriptor_set_layout;
    };
}