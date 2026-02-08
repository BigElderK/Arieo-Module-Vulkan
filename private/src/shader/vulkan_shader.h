
#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>

namespace Arieo
{
    class VulkanShader final
        : public Interface::RHI::IShader
    {
    public:
        friend class VulkanDevice;
        VulkanShader(VkShaderModule&& vk_shader)
            : m_vk_shader_module(std::move(vk_shader))
        {

        }
    private:
        VkShaderModule m_vk_shader_module;
    };
}