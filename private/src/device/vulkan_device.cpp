#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan.h>
#include <vulkan_core.h>


#include "../vulkan_rhi.h"

namespace Arieo
{
    Interface::RHI::Format VulkanDevice::findSupportedFormat(
        const std::vector<Interface::RHI::Format>& candidate_formats, 
        Interface::RHI::ImageTiling image_tiling,
        Interface::RHI::FormatFeatureFlags feature_flags)
    {
        VkImageTiling vk_image_tiling = Base::mapEnum<VkImageTiling>(image_tiling);
        VkFormatFeatureFlags vk_formate_feature_flags = Base::mapEnum<VkFormatFeatureFlags>(feature_flags);
        for(Interface::RHI::Format candidate_format : candidate_formats)
        {
            VkFormat vk_candidate_format = Base::mapEnum<VkFormat>(candidate_format);
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_vk_phys_device, vk_candidate_format, &props);

            if (vk_image_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & vk_formate_feature_flags) == vk_formate_feature_flags) 
            {
                return candidate_format;
            } 
            else if (vk_image_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & vk_formate_feature_flags) == vk_formate_feature_flags) 
            {
                return candidate_format;
            }
        }
        
        Core::Logger::error("failed to find supported format!");

        return Interface::RHI::Format::UNKNOWN;
    }

    Interface::RHI::ISwapchain* VulkanDevice::createSwapchain(Interface::RHI::IRenderSurface* render_surface)
    {
        VulkanSurface* vulkan_surface = Base::castInterfaceToInstance<VulkanSurface>(render_surface);

        // Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
        Core::Logger::trace("Querying surface capabilities");
        VkSurfaceCapabilitiesKHR surface_capabilities;
        {
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                m_vk_phys_device, 
                vulkan_surface->m_vk_surface_khr, 
                &surface_capabilities
            );
        }

        // Surface formats (pixel format, color space)
        Core::Logger::trace("Querying surface formats");
        std::vector<VkSurfaceFormatKHR> surface_format_array;
        {
            uint32_t format_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                m_vk_phys_device,
                vulkan_surface->m_vk_surface_khr,
                &format_count,
                nullptr
            );

            if(format_count > 0)
            {
                surface_format_array.resize(format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    m_vk_phys_device,
                    vulkan_surface->m_vk_surface_khr,
                    &format_count,
                    surface_format_array.data()
                );
            }
        }

        // Available presentation modes
        Core::Logger::trace("Querying surface present modes");
        std::vector<VkPresentModeKHR> present_mode_array;
        {
            uint32_t present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                m_vk_phys_device, 
                vulkan_surface->m_vk_surface_khr, 
                &present_mode_count, 
                nullptr
            );

            if (present_mode_count != 0) 
            {
                present_mode_array.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    m_vk_phys_device, 
                    vulkan_surface->m_vk_surface_khr, 
                    &present_mode_count, 
                    present_mode_array.data() 
                );
            } 
        }

        // Select suitable settings.
        Core::Logger::trace("Selecting surface format, present mode and extent");
        VkSurfaceFormatKHR* selected_surface_format = nullptr;
        VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
        VkExtent2D fixed_extent{};
        uint32_t image_count = 0;
        {
            // select surface formalt
            for(VkSurfaceFormatKHR& surface_format : surface_format_array) 
            {
                //if(surface_format.format == VK_FORMAT_B8G8R8A8_SRGB && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                Core::Logger::trace("Checking surface format {} {}", (std::uint32_t)surface_format.format, (std::uint32_t)surface_format.colorSpace);
                if((surface_format.format == VK_FORMAT_B8G8R8A8_UNORM || surface_format.format == VK_FORMAT_R8G8B8A8_UNORM)
                    && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    selected_surface_format = &surface_format;
                    break;
                }
            }
            if(selected_surface_format == nullptr)
            {
                Core::Logger::error("Cannot found suitable surface format");
                return nullptr;
            }

            // select present mode
            for(VkPresentModeKHR present_mode : present_mode_array)
            {
                if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    selected_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
            }
            if(selected_present_mode == VK_PRESENT_MODE_MAX_ENUM_KHR)
            {
                Core::Logger::warn("Cannot found suitable present mode, fallback to default");
                selected_present_mode = present_mode_array[0];
            }

            // check extent 2D
            if(surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                fixed_extent.width = surface_capabilities.currentExtent.width;
                fixed_extent.height = surface_capabilities.currentExtent.height;
                Core::Logger::trace("Fixed extent from capability {} {}", fixed_extent.width, fixed_extent.height);
            }
            else
            {
                Interface::Window::IWindow* surface_window = render_surface->getAttachedWindow();
                Base::Math::Vector<uint32_t, 2> frame_buffer_size = surface_window->getFramebufferSize();

                fixed_extent.width = std::clamp(frame_buffer_size.x, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
                fixed_extent.height = std::clamp(frame_buffer_size.y, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

                Core::Logger::trace("Fixed extent from window {} {}", fixed_extent.width, fixed_extent.height);
            }

            // choose image count
            image_count = surface_capabilities.minImageCount + 1;

        }

        Core::Logger::trace("Creating swapchain");
        VkSwapchainCreateInfoKHR swapchain_create_info = {};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = vulkan_surface->m_vk_surface_khr;
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = selected_surface_format->format;
        swapchain_create_info.imageColorSpace = selected_surface_format->colorSpace;
        swapchain_create_info.imageExtent = fixed_extent;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        std::uint32_t indices[2] = {m_graphic_queue_index, m_present_queue_index};
        if (m_graphic_queue_index != m_present_queue_index) 
        {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2;
            swapchain_create_info.pQueueFamilyIndices = indices;
        } 
        else 
        {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 0; // Optional
            swapchain_create_info.pQueueFamilyIndices = nullptr; // Optional
        }

        swapchain_create_info.preTransform = surface_capabilities.currentTransform; 
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        swapchain_create_info.presentMode = selected_present_mode;
        swapchain_create_info.clipped = VK_TRUE;

        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

        VkSwapchainKHR vk_swapchain;
        VkResult result = vkCreateSwapchainKHR(m_vk_device, &swapchain_create_info, nullptr, &vk_swapchain);
        if(result != VK_SUCCESS) 
        {
            Core::Logger::fatal("Failed to create swap chain!");
            return nullptr;
        }

        Core::Logger::trace("swapchain created");

        VulkanSwapchain* vulkan_swapchain = Base::newT<VulkanSwapchain>(
            m_vk_device, 
            swapchain_create_info, 
            std::move(vk_swapchain)
        );

        // create image view
        Core::Logger::trace("Creating swapchain image views");
        {
            std::vector<VkImage> vk_swapchain_image_array;

            uint32_t image_count;
            vkGetSwapchainImagesKHR(m_vk_device, vulkan_swapchain->m_vk_swapchain_khr, &image_count, nullptr);
            vk_swapchain_image_array.resize(image_count);
            vkGetSwapchainImagesKHR(m_vk_device, vulkan_swapchain->m_vk_swapchain_khr, &image_count, vk_swapchain_image_array.data());           
            Core::Logger::trace("swapchain image retrieved {}", image_count);

            for(VkImage& vk_swapchain_image : vk_swapchain_image_array)
            {
                // Create ImageView
                VkImageView vk_swapchain_image_view;
                {
                    VkImageViewCreateInfo vk_image_view_create_info{};
                    vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    vk_image_view_create_info.image = vk_swapchain_image;

                    vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    vk_image_view_create_info.format = swapchain_create_info.imageFormat; 

                    vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                    vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                    vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                    vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                    vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    vk_image_view_create_info.subresourceRange.baseMipLevel = 0;
                    vk_image_view_create_info.subresourceRange.levelCount = 1;
                    vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
                    vk_image_view_create_info.subresourceRange.layerCount = 1;

                    VkResult result = vkCreateImageView(m_vk_device, &vk_image_view_create_info, nullptr, &vk_swapchain_image_view);
                    if(result != VK_SUCCESS)
                    {
                        Core::Logger::fatal("failed to create image view");
                        return nullptr;
                    }
                }
                
                VulkanImage* vulkan_image = Base::newT<VulkanImage>(    
                    m_vk_device,                
                    std::move(vk_swapchain_image),
                    std::move(vk_swapchain_image_view),
                    VK_NULL_HANDLE, 
                    VK_NULL_HANDLE,
                    VmaAllocationInfo{},
                    VkExtent3D(swapchain_create_info.imageExtent.width, swapchain_create_info.imageExtent.height, 1),
                    swapchain_create_info.imageFormat
                );

                vulkan_swapchain->m_image_resource_array.emplace_back(
                    vulkan_image
                );
                vulkan_swapchain->m_image_view_array.emplace_back(vulkan_image->getImageView());

                Core::Logger::trace("swapchain image view created");
            }
        }

        return vulkan_swapchain;
    }

    void VulkanDevice::destroySwapchain(Interface::RHI::ISwapchain* swapchain)
    {
        VulkanSwapchain* vulkan_swapchain = Base::castInterfaceToInstance<VulkanSwapchain>(swapchain);

        // Destroy image resource
        for(Interface::RHI::IImage* swapchain_image : vulkan_swapchain->m_image_resource_array)
        {
            VulkanImage* vulkan_swapchain_image = Base::castInterfaceToInstance<VulkanImage>(swapchain_image);
            // Destroy image view.
            vkDestroyImageView(m_vk_device, vulkan_swapchain_image->m_vulkan_image_view.m_vk_image_view, nullptr);
            Base::deleteT(vulkan_swapchain_image);
        }

        vkDestroySwapchainKHR(m_vk_device, vulkan_swapchain->m_vk_swapchain_khr, nullptr);
        Base::deleteT(vulkan_swapchain);
        return;
    }

    Interface::RHI::IFramebuffer* VulkanDevice::createFramebuffer(
        Interface::RHI::IPipeline* pipeline, 
        Interface::RHI::ISwapchain* swapchain,
        const std::vector<Interface::RHI::IImageView*>& attachment_array
    )
    {
        VulkanPipeline* vulkan_pipeline = Base::castInterfaceToInstance<VulkanPipeline>(pipeline);

        std::vector<VkImageView> vk_attachment_array;
        for(Interface::RHI::IImageView* attach_image_view : attachment_array)
        {
            vk_attachment_array.emplace_back(Base::castInterfaceToInstance<VulkanImageView>(attach_image_view)->m_vk_image_view);
        };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.attachmentCount = vk_attachment_array.size();
        framebuffer_info.pAttachments = vk_attachment_array.data();
        framebuffer_info.renderPass = vulkan_pipeline->m_vk_render_pass;
        framebuffer_info.width =  swapchain->getExtent().size.x,
        framebuffer_info.height = swapchain->getExtent().size.y,
        framebuffer_info.layers = 1;

        VkFramebuffer vk_framebuffer;
        VkResult result = vkCreateFramebuffer(m_vk_device, &framebuffer_info, nullptr, &vk_framebuffer); 
        if (result != VK_SUCCESS) 
        {
            Core::Logger::fatal("failed to create framebuffer: {}", VulkanUtility::covertVkResultToString(result));
            return nullptr;
        }

        Core::Logger::trace("swapchain framebuffer created");
        return Base::newT<VulkanFramebuffer>(std::move(vk_framebuffer));
    }

    void VulkanDevice::destroyFramebuffer(Interface::RHI::IFramebuffer* framebuffer)
    {
        VulkanFramebuffer* vulkan_framebuffer = Base::castInterfaceToInstance<VulkanFramebuffer>(framebuffer);
        vkDestroyFramebuffer(m_vk_device, vulkan_framebuffer->m_vk_framebuffer, nullptr);
        Base::deleteT<VulkanFramebuffer>(std::move(vulkan_framebuffer));
    }

    Interface::RHI::IShader* VulkanDevice::createShader(void* buf, size_t buf_size)
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = buf_size;
        create_info.pCode = reinterpret_cast<const uint32_t*>(buf);

        VkShaderModule shader_module;
        VkResult result = vkCreateShaderModule(m_vk_device, &create_info, nullptr, &shader_module);
        if(result != VK_SUCCESS)
        {
            Core::Logger::fatal("failed to create shader: {}", VulkanUtility::covertVkResultToString(result));
            return nullptr;
        }

        Core::Logger::trace("shader created");
        return Base::newT<VulkanShader>(std::move(shader_module));
    }

    void VulkanDevice::destroyShader(Interface::RHI::IShader* shader)
    {
        VulkanShader* vulkan_shader = Base::castInterfaceToInstance<VulkanShader>(shader);
        vkDestroyShaderModule(m_vk_device, vulkan_shader->m_vk_shader_module, nullptr);

        Base::deleteT(vulkan_shader);
    }

    Interface::RHI::IPipeline* VulkanDevice::createPipeline(
        Interface::RHI::IShader* vert_shader, 
        Interface::RHI::IShader* frag_shader, 
        Interface::RHI::IImageView* target_color_attachment,
        Interface::RHI::IImageView* target_depth_attachment
    )
    {
        VulkanImageView* target_color_image_view = Base::castInterfaceToInstance<VulkanImageView>(target_color_attachment);
        VulkanImageView* target_depth_image_view = Base::castInterfaceToInstance<VulkanImageView>(target_depth_attachment);

        // Set shaders
        Core::Logger::trace("Set shaders");
        VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
        vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_stage_info.module = Base::castInterfaceToInstance<VulkanShader>(vert_shader)->m_vk_shader_module;
        vert_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
        frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_stage_info.module = Base::castInterfaceToInstance<VulkanShader>(frag_shader)->m_vk_shader_module;
        frag_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vert_shader_stage_info, frag_shader_stage_info};

        // Dynamic state configs
        Core::Logger::trace("Dynamic state config");
        VkDynamicState dynamic_states[]
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>(sizeof(dynamic_states) / sizeof(dynamic_states[0]));
        dynamic_state.pDynamicStates = dynamic_states;

        //Viewport and scissor
        Core::Logger::trace("Viewport and scissor");
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(target_color_image_view->m_vulkan_image.m_vk_image_extent.width);
        viewport.height = static_cast<float>(target_color_image_view->m_vulkan_image.m_vk_image_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;        

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {target_color_image_view->m_vulkan_image.m_vk_image_extent.width, target_color_image_view->m_vulkan_image.m_vk_image_extent.height};

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;        

        //TODO pass this from parameters.
        struct Vertex
        {
            Base::Math::Vector3 pos;
            Base::Math::Vector3 color;
            Base::Math::Vector2 tex_coord;
        };
        VkVertexInputBindingDescription binding_desc{};
        binding_desc.binding = 0;
        binding_desc.stride = sizeof(Vertex);
        binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        std::array<VkVertexInputAttributeDescription, 3> attribute_desc_array;
        {
            attribute_desc_array[0].binding = 0;
            attribute_desc_array[0].location = 0;
            attribute_desc_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_desc_array[0].offset = offsetof(Vertex, pos);

            attribute_desc_array[1].binding = 0;
            attribute_desc_array[1].location = 1;
            attribute_desc_array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_desc_array[1].offset = offsetof(Vertex, color);

            attribute_desc_array[2].binding = 0;
            attribute_desc_array[2].location = 2;
            attribute_desc_array[2].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_desc_array[2].offset = offsetof(Vertex, tex_coord);
        };

        //TODO pass this from parameter
        VkDescriptorSetLayout vk_descriptor_set_layout;
        {
            VkDescriptorSetLayoutBinding desc_layout_binding{};
            desc_layout_binding.binding = 0;
            desc_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            desc_layout_binding.descriptorCount = 1;
            desc_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding sample_layout_binding{};
            sample_layout_binding.binding = 1;
            sample_layout_binding.descriptorCount = 1;
            sample_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            sample_layout_binding.pImmutableSamplers = nullptr;
            sample_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            std::array<VkDescriptorSetLayoutBinding, 2> bindings = {desc_layout_binding, sample_layout_binding};

            VkDescriptorSetLayoutCreateInfo desc_layout_create_info{};
            desc_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            desc_layout_create_info.bindingCount = bindings.size();
            desc_layout_create_info.pBindings = bindings.data();
            
            if (vkCreateDescriptorSetLayout(m_vk_device, &desc_layout_create_info, nullptr, &vk_descriptor_set_layout) != VK_SUCCESS) 
            {
                Core::Logger::error("failed to create descriptor set layout!");
                return nullptr;
            }
        }
        
        // Vertex input
        Core::Logger::trace("Vertex input");
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.pVertexBindingDescriptions = &binding_desc; // Optional
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc_array.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_desc_array.data(); // Optional

        // Input assembly
        Core::Logger::trace("input assembly");
        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;        

        // Rasterizer
        Core::Logger::trace("rasterizer");
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        // Color blending
        Core::Logger::trace("color blending");
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f; // Optional
        color_blending.blendConstants[1] = 0.0f; // Optional
        color_blending.blendConstants[2] = 0.0f; // Optional
        color_blending.blendConstants[3] = 0.0f; // Optional

        // Pipeline layout
        Core::Logger::trace("pipeline layout");
        VkPipelineLayout vk_pipeline_layout;
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1; // Optional
        pipeline_layout_info.pSetLayouts = &vk_descriptor_set_layout; // Optional
        pipeline_layout_info.pushConstantRangeCount = 0; // Optional
        pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(m_vk_device, &pipeline_layout_info, nullptr, &vk_pipeline_layout) != VK_SUCCESS) 
        {
            Core::Logger::error("Failed to create pipeline layout");
        }

        // create render pass
        VkRenderPass vk_render_pass;
        {
            Core::Logger::trace("create render pass");
            VkAttachmentDescription color_attachment{};
            color_attachment.format = target_color_image_view->m_vulkan_image.m_vk_image_format;
            color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentDescription depth_attachment{};
            depth_attachment.format = target_depth_image_view->m_vulkan_image.m_vk_image_format;
            depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference color_attachment_ref{};
            color_attachment_ref.attachment = 0;
            color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depth_attachment_ref{};
            depth_attachment_ref.attachment = 1;
            depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment_ref;
            subpass.pDepthStencilAttachment = &depth_attachment_ref;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;        

            std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};
            VkRenderPassCreateInfo render_pass_info{};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_info.attachmentCount = attachments.size();
            render_pass_info.pAttachments = attachments.data();
            render_pass_info.subpassCount = 1;
            render_pass_info.pSubpasses = &subpass;
            render_pass_info.dependencyCount = 1;
            render_pass_info.pDependencies = &dependency;

            VkResult result = vkCreateRenderPass(m_vk_device, &render_pass_info, nullptr, &vk_render_pass); 
            if (result != VK_SUCCESS) 
            {
                Core::Logger::fatal("failed to create pipeline: {}", VulkanUtility::covertVkResultToString(result));
                return nullptr;
            }
            Core::Logger::trace("renderpass created");
        }

        // Depth stencil
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.minDepthBounds = 0.0f; // Optional
        depth_stencil.maxDepthBounds = 1.0f; // Optional
        depth_stencil.stencilTestEnable = VK_FALSE;
        depth_stencil.front = {}; // Optional
        depth_stencil.back = {}; // Optional

        // Create pipeline
        VkGraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = 2;
        pipeline_create_info.pStages = shaderStages;
        pipeline_create_info.pVertexInputState = &vertex_input_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly;
        pipeline_create_info.pViewportState = &viewportState;
        pipeline_create_info.pRasterizationState = &rasterizer;
        pipeline_create_info.pMultisampleState = &multisampling;
        pipeline_create_info.pDepthStencilState = &depth_stencil; // Optional
        pipeline_create_info.pColorBlendState = &color_blending;
        pipeline_create_info.pDynamicState = &dynamic_state;        
        pipeline_create_info.layout = vk_pipeline_layout;
        pipeline_create_info.renderPass = vk_render_pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipeline_create_info.basePipelineIndex = -1; // Optional        

        VkPipeline vk_pipeline;
        VkResult result = vkCreateGraphicsPipelines(m_vk_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &vk_pipeline);
        if (result != VK_SUCCESS)
        {
            Core::Logger::fatal("failed to create pipeline: {}", VulkanUtility::covertVkResultToString(result));
            return nullptr;
        }

        Core::Logger::trace("Vulkan pipeline created");
        return Base::newT<VulkanPipeline>(
            std::move(vk_pipeline), 
            std::move(vk_pipeline_layout), 
            std::move(vk_descriptor_set_layout),
            std::move(vk_render_pass),
            target_color_image_view->m_vulkan_image.m_vk_image_extent
        );
    }

    void VulkanDevice::destroyPipeline(Interface::RHI::IPipeline* pipeline)
    {
        VulkanPipeline* vulkan_pipeline = Base::castInterfaceToInstance<VulkanPipeline>(pipeline);

        vkDestroyDescriptorSetLayout(m_vk_device, vulkan_pipeline->m_vk_descriptor_set_layout, nullptr);
        vkDestroyRenderPass(m_vk_device, vulkan_pipeline->m_vk_render_pass, nullptr);
        vkDestroyPipelineLayout(m_vk_device, vulkan_pipeline->m_vk_pipeline_layout, nullptr);
        vkDestroyPipeline(m_vk_device, vulkan_pipeline->m_vk_pipeline, nullptr);

        Base::deleteT(vulkan_pipeline);
    }
    
    Interface::RHI::IFence* VulkanDevice::createFence()
    {
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;        
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkFence vk_fence;
        if(vkCreateFence(m_vk_device, &fence_info, nullptr, &vk_fence) != VK_SUCCESS)
        {
            Core::Logger::error("failed create fence");
            return nullptr;
        }

        return Base::newT<VulkanFence>(m_vk_device, std::move(vk_fence));
    }

    void VulkanDevice::destroyFence(Interface::RHI::IFence* fence)
    {
        VulkanFence* vulkan_fence = Base::castInterfaceToInstance<VulkanFence>(fence);       
        vkDestroyFence(m_vk_device, vulkan_fence->m_vk_fence, nullptr);
        Base::deleteT(vulkan_fence);
    }

    Interface::RHI::ISemaphore* VulkanDevice::createSemaphore()
    {
        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkSemaphore vk_semaphore;
        if(vkCreateSemaphore(m_vk_device, &semaphore_info, nullptr, &vk_semaphore) != VK_SUCCESS)
        {
            Core::Logger::error("failed create fence");
            return nullptr;
        }

        return Base::newT<VulkanSemaphore>(m_vk_device, std::move(vk_semaphore));
    }

    void VulkanDevice::destroySemaphore(Interface::RHI::ISemaphore* semaphore)
    {
        VulkanSemaphore* vulkan_semaphore = Base::castInterfaceToInstance<VulkanSemaphore>(semaphore);       
        vkDestroySemaphore(m_vk_device, vulkan_semaphore->m_vk_semaphore, nullptr);
        Base::deleteT(vulkan_semaphore);
    }

    Interface::RHI::IBuffer* VulkanDevice::createBuffer(
        size_t size, 
        Interface::RHI::BufferUsageBitFlags buffer_usage, 
        Interface::RHI::BufferAllocationFlags allocation_flag, 
        Interface::RHI::MemoryUsage memory_usage)
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = Base::mapEnum<VkBufferUsageFlagBits>(buffer_usage);
        //buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = Base::mapEnum<VmaMemoryUsage>(memory_usage);
        //alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        alloc_info.flags = Base::mapEnum<VmaAllocationCreateFlags>(allocation_flag);

        VkBuffer vk_buffer;
        VmaAllocation vma_allocation;
        VkResult result = vmaCreateBuffer(m_vma_allocator, &buffer_info, &alloc_info, &vk_buffer, &vma_allocation, nullptr);
        if(result != VK_SUCCESS)
        {
            Core::Logger::error("Create buffer failed: {}", VulkanUtility::covertVkResultToString(result));
        }
        Core::Logger::trace("Buffer created {}", size);
        return Base::newT<VulkanBuffer>(std::move(vk_buffer), m_vma_allocator, std::move(vma_allocation));
    }

    void VulkanDevice::destroyBuffer(Interface::RHI::IBuffer* buffer)
    {
        VulkanBuffer* vulkan_buffer = Base::castInterfaceToInstance<VulkanBuffer>(buffer);
        vmaDestroyBuffer(m_vma_allocator, vulkan_buffer->m_vk_buffer, vulkan_buffer->m_vma_allocation);
        Base::deleteT<VulkanBuffer>(vulkan_buffer);
    }

    Interface::RHI::IDescriptorPool* VulkanDevice::createDescriptorPool(size_t capacity)
    {
        std::array<VkDescriptorPoolSize, 2> pool_sizes{};
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount = static_cast<uint32_t>(capacity);
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = static_cast<uint32_t>(capacity);

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = pool_sizes.size();
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = static_cast<uint32_t>(capacity);

        VkDescriptorPool descriptor_pool;
        VkResult result = vkCreateDescriptorPool(m_vk_device, &pool_info, nullptr, &descriptor_pool);
        if (result != VK_SUCCESS) 
        {
            Core::Logger::error("Create descriptor failed failed: {}", VulkanUtility::covertVkResultToString(result));
        }
        Core::Logger::trace("Descriptor Pool created {}", capacity);

        return Base::newT<VulkanDescriptorPool>(
            m_vk_device,
            std::move(descriptor_pool)
        );
    }

    void VulkanDevice::destroyDescriptorPool(Interface::RHI::IDescriptorPool* descriptor_pool)
    {
        VulkanDescriptorPool* vulkan_descriptor_pool = Base::castInterfaceToInstance<VulkanDescriptorPool>(descriptor_pool);
        vkDestroyDescriptorPool(m_vk_device, vulkan_descriptor_pool->m_vk_descriptor_pool, nullptr);
        Base::deleteT<VulkanDescriptorPool>(vulkan_descriptor_pool);
    }

    Interface::RHI::IImage* VulkanDevice::createImage(
        std::uint32_t width, 
        std::uint32_t height, 
        Interface::RHI::Format format, 
        Interface::RHI::ImageAspectFlags aspect,
        Interface::RHI::ImageTiling tiling, 
        Interface::RHI::ImageUsageFlags usage,
        Interface::RHI::MemoryUsage mem_usage)
    {
        Core::Logger::trace("Prepare for creating image {}x{}", width, height);
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = Base::mapEnum<VkFormat>(format);

        image_create_info.extent = {width, height, 1};
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = Base::mapEnum<VkImageTiling>(tiling);

        //image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        image_create_info.usage = Base::mapEnum<VkImageUsageFlags>(usage);

        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // Allocation create info
        VmaAllocationCreateInfo mem_alloc_info = {};
        mem_alloc_info.usage = Base::mapEnum<VmaMemoryUsage>(mem_usage); // Memory will be only on GPU

        VkImage vk_image;
        VmaAllocation vk_image_allocation;
        VmaAllocationInfo vk_image_allocation_info;

        Core::Logger::trace("Creating image {}x{}", width, height);
        VkResult result = vmaCreateImage(
            m_vma_allocator,
            &image_create_info,
            &mem_alloc_info,
            &vk_image,
            &vk_image_allocation,
            &vk_image_allocation_info
        );
        if(result != VK_SUCCESS)
        {
            Core::Logger::error("Image create failed: {}", VulkanUtility::covertVkResultToString(result));
            return nullptr;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_vk_device, vk_image, &memRequirements);

        // Create Image View
        Core::Logger::trace("Creating image view for image");
        VkImageView vk_image_view;
        {
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = vk_image;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = image_create_info.format;
            
            //view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.aspectMask = Base::mapEnum<VkImageAspectFlags>(aspect);

            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(
                m_vk_device,
                &view_info,
                nullptr,
                &vk_image_view
            );
            if(result != VK_SUCCESS)
            {
                Core::Logger::error("ImageView create failed: {}", VulkanUtility::covertVkResultToString(result));
                return nullptr;
            }
        };

        // Create Sampler
        Core::Logger::trace("Creating image sampler for image");
        VkSampler vk_sampler;
        {
            VkSamplerCreateInfo sampler_create_info{};
            sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_create_info.magFilter = VK_FILTER_LINEAR;
            sampler_create_info.minFilter = VK_FILTER_LINEAR;

            sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

            sampler_create_info.anisotropyEnable = VK_TRUE;
            sampler_create_info.maxAnisotropy = m_vk_phys_device_properties.limits.maxSamplerAnisotropy;

            sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            sampler_create_info.unnormalizedCoordinates = VK_FALSE;

            sampler_create_info.compareEnable = VK_FALSE;
            sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;

            sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler_create_info.mipLodBias = 0.0f;
            sampler_create_info.minLod = 0.0f;
            sampler_create_info.maxLod = 0.0f;

            VkResult result = vkCreateSampler(m_vk_device, &sampler_create_info, nullptr, &vk_sampler); 
            if(result != VK_SUCCESS)
            {
                Core::Logger::error("Image sampler create failed: {}", VulkanUtility::covertVkResultToString(result));
                return nullptr;
            }
        }

        return Base::newT<VulkanImage>(
            m_vk_device, 
            std::move(vk_image),
            std::move(vk_image_view),
            std::move(vk_sampler),
            std::move(vk_image_allocation),
            std::move(vk_image_allocation_info),
            image_create_info.extent, 
            image_create_info.format
        );
    }

    void VulkanDevice::destroyImage(Interface::RHI::IImage* image)
    {
        VulkanImage* vulkan_image = Base::castInterfaceToInstance<VulkanImage>(image);
        vkDestroySampler(m_vk_device, vulkan_image->m_vulkan_image_sampler.m_vk_image_sampler, nullptr);
        vkDestroyImageView(m_vk_device, vulkan_image->m_vulkan_image_view.m_vk_image_view, nullptr);
        vmaDestroyImage(m_vma_allocator, vulkan_image->m_vk_image, vulkan_image->m_vma_allocation);
    }

    void VulkanDevice::waitIdle()
    {
        VkResult result = vkDeviceWaitIdle(m_vk_device);
        if(result != VK_SUCCESS)
        {
            Core::Logger::error("DeviceWaitIdle failed: {}", VulkanUtility::covertVkResultToString(result));
        }
    }
}