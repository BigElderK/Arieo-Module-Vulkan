#include "base/prerequisites.h"
#include "core/core.h"

#include <vulkan/vulkan.h>
#include <vulkan.h>
#include <vulkan_core.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "../vulkan_rhi.h"

namespace Arieo
{
    void VulkanInstance::initialize()
    {
        assert(m_vk_instance == VK_NULL_HANDLE);
        
        VkApplicationInfo vk_app_info
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Application",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Arieo Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0
        };

        std::vector<const char*> extension_names;

        VkInstanceCreateInfo vk_instance_create_info
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &vk_app_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = {},
            .enabledExtensionCount = 0,
            .ppEnabledExtensionNames = {} 
        };

        // Add debug layers
        std::vector<const char*> validation_layer_names;
        {
            if(Base::StringUtility::toLower(Core::SystemUtility::Environment::getEnvironmentValue("ENABLE_VULKAN_VALIDATION_LAYER")) == "true")
            {
                validation_layer_names.emplace_back("VK_LAYER_KHRONOS_validation");
            }
            
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            std::vector<VkLayerProperties> available_layers(layer_count);
            vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

            // Print available_layers
            for (const auto& layer_properties : available_layers) 
            {
                Core::Logger::trace("Valid layer: {}", layer_properties.layerName);
            }

            for (const char* layer_name : validation_layer_names) 
            {
                bool layer_found = false;

                for (const auto& layer_properties : available_layers) 
                {
                    if (strcmp(layer_name, layer_properties.layerName) == 0) 
                    {
                        layer_found = true;
                        break;
                    }
                }

                if (layer_found == false) 
                {
                    Core::Logger::fatal("Cannot found validation_layer {}", layer_name);
                }
            }
        }
        
        postProcessInstanceCreateInfo(vk_instance_create_info, extension_names);
        vk_instance_create_info.enabledExtensionCount = extension_names.size();
        vk_instance_create_info.ppEnabledExtensionNames = extension_names.data(); 
        vk_instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layer_names.size());
        vk_instance_create_info.ppEnabledLayerNames = validation_layer_names.data();

        VkResult result = vkCreateInstance(&vk_instance_create_info, nullptr, &m_vk_instance);
        if(result == VK_SUCCESS)
        {
            Core::Logger::trace("Create vulkan instance OK");
        }
        else
        {
            Core::Logger::fatal("Create vulkan instance Failed! {}", VulkanUtility::covertVkResultToString(result));
        }
    }

    void VulkanInstance::finalize()
    {
        if(m_vk_instance != VK_NULL_HANDLE)
        {
            // TODO: crash here:
            // vkDestroyInstance(m_vk_instance, nullptr);
            m_vk_instance = VK_NULL_HANDLE;

            m_hardware_information_array.clear();
        }
    }        
    
    std::vector<std::string>& VulkanInstance::getHardwareInfomations()
    {
        assert(m_vk_instance != VK_NULL_HANDLE);
        if(m_hardware_information_array.size() == 0)
        {
            uint32_t vk_phys_device_count = 0;
            vkEnumeratePhysicalDevices(m_vk_instance, &vk_phys_device_count, nullptr);

            std::vector<VkPhysicalDevice> vk_phys_devices(vk_phys_device_count);
            vkEnumeratePhysicalDevices(m_vk_instance, &vk_phys_device_count, vk_phys_devices.data());

            // Select a suitable device (check for graphics queue family with present support)
            for (const auto& vk_phys_device : vk_phys_devices) 
            {
                // Device properties
                VkPhysicalDeviceProperties device_properties;
                vkGetPhysicalDeviceProperties(vk_phys_device, &device_properties);

                // Device features
                VkPhysicalDeviceFeatures device_features;
                vkGetPhysicalDeviceFeatures(vk_phys_device, &device_features);

                std::string device_type;
                switch (device_properties.deviceType) 
                {
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                        device_type = "Integrated GPU"; break;
                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                        device_type = "Discrete GPU"; break;
                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                        device_type = "Virtual GPU"; break;
                    case VK_PHYSICAL_DEVICE_TYPE_CPU:
                        device_type = "CPU"; break;
                    default:
                        device_type = "Other"; break;
                }

                std::string basic_info_string;
                {
                    basic_info_string = Base::StringUtility::format("Device Name:{}\n", device_properties.deviceName);
                    basic_info_string += Base::StringUtility::format("\tAPI Version: {}.{}.{}\n", 
                        VK_VERSION_MAJOR(device_properties.apiVersion), 
                        VK_VERSION_MINOR(device_properties.apiVersion), 
                        VK_VERSION_PATCH(device_properties.apiVersion));
                    basic_info_string += Base::StringUtility::format("\tDriver Version: {}\n", device_properties.driverVersion);
                    basic_info_string += Base::StringUtility::format("\tVender ID: {}\n", device_properties.vendorID);
                    basic_info_string += Base::StringUtility::format("\tDevice ID: {}\n", device_properties.deviceID);
                    basic_info_string += Base::StringUtility::format("\tDevice Type: {}\n", device_type);
                    basic_info_string += Base::StringUtility::format("\tSupported Features:\n");
                    basic_info_string += Base::StringUtility::format("\t\tGeometry Shader: {}\n", (device_features.geometryShader ? "Yes" : "No"));
                    basic_info_string += Base::StringUtility::format("\t\tTessellation Shader: {}\n", (device_features.tessellationShader ? "Yes" : "No"));
                    basic_info_string += Base::StringUtility::format("\t\tMultiViewport: {}\n", (device_features.multiViewport ? "Yes" : "No"));
                }

                // Memory properties
                std::string device_memory_info_string;
                {
                    VkPhysicalDeviceMemoryProperties memory_properties;
                    vkGetPhysicalDeviceMemoryProperties(vk_phys_device, &memory_properties);
                
                    device_memory_info_string += Base::StringUtility::format("\tMemory Heaps: {}\n", memory_properties.memoryHeapCount);
                    for (uint32_t i = 0; i < memory_properties.memoryHeapCount; i++) 
                    {
                        device_memory_info_string += Base::StringUtility::format("\t\tHeap: size = {} MB", 
                            memory_properties.memoryHeaps[i].size / (1024 * 1024),
                            memory_properties.memoryHeapCount);
                        if (memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) 
                        {
                            device_memory_info_string += " (Device Local)";
                        }
                        device_memory_info_string += "\n";
                    }
                }

                // Queue families
                std::string queue_family_info_string;
                {
                    uint32_t queue_family_count = 0;
                    vkGetPhysicalDeviceQueueFamilyProperties(vk_phys_device, &queue_family_count, nullptr);
                    
                    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                    vkGetPhysicalDeviceQueueFamilyProperties(vk_phys_device, &queue_family_count, queue_families.data());

                    queue_family_info_string = Base::StringUtility::format("\tQueue Families {}:\n", queue_family_count);
                    
                    for (uint32_t i = 0; i < queue_family_count; i++) 
                    {
                        queue_family_info_string += Base::StringUtility::format("\t\tQueue Family {}: Count={}, Flags={}\n",
                            i,
                            queue_family_count,
                            queue_families[i].queueFlags
                        );
                    }
                }

                std::string device_extensions_info_string;
                {
                    uint32_t extension_count = 0;
                    vkEnumerateDeviceExtensionProperties(vk_phys_device, nullptr, &extension_count, nullptr);

                    std::vector<VkExtensionProperties> extensions(extension_count);
                    vkEnumerateDeviceExtensionProperties(vk_phys_device, nullptr, &extension_count, extensions.data());

                    device_extensions_info_string = Base::StringUtility::format("\tSupported Extensions {}:\n", extension_count);
                    for (const auto& extension : extensions) 
                    {
                        device_extensions_info_string += Base::StringUtility::format("\t\t {} (version {})\n",
                            extension.extensionName,
                            extension.specVersion
                        );
                    }
                }
                m_hardware_information_array.emplace_back(
                    basic_info_string
                    + device_memory_info_string 
                    + queue_family_info_string
                    + device_extensions_info_string
                );
            }
        }
        return m_hardware_information_array;
    }

    void VulkanInstance::destroySurface(Interface::RHI::IRenderSurface* surface)
    {
        VulkanSurface* vulkan_surface = Base::castInterfaceToInstance<VulkanSurface>(surface);
        vkDestroySurfaceKHR(m_vk_instance, vulkan_surface->m_vk_surface_khr, nullptr);
        Base::deleteT(vulkan_surface);
    }

    Interface::RHI::IRenderDevice* VulkanInstance::createDevice(size_t hardware_index, Interface::RHI::IRenderSurface* surface)
    {
        uint32_t vk_phys_device_count = 0;
        vkEnumeratePhysicalDevices(m_vk_instance, &vk_phys_device_count, nullptr);

        if(hardware_index >= vk_phys_device_count)
        {
            Core::Logger::fatal("The hardware index {} is invalid", hardware_index);
            return nullptr;
        }

        std::vector<VkPhysicalDevice> vk_phys_devices(vk_phys_device_count);
        vkEnumeratePhysicalDevices(m_vk_instance, &vk_phys_device_count, vk_phys_devices.data());

        VkPhysicalDevice vk_selected_phys_device = vk_phys_devices[hardware_index];

        // Create logical device
        VkDevice vk_device;
        // Find graphics and present queue families
        uint32_t graphics_queue_family_index = std::numeric_limits<uint32_t>::max();
        uint32_t present_queue_family_index = std::numeric_limits<uint32_t>::max();
        {
            {
                uint32_t queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(vk_selected_phys_device , &queue_family_count, nullptr);
                
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(vk_selected_phys_device , &queue_family_count, queue_families.data());

                for (uint32_t i = 0; i < queue_family_count; i++) 
                {
                    VulkanSurface* vulkan_surface = Base::castInterfaceToInstance<VulkanSurface>(surface);
                    if(vulkan_surface == nullptr)
                    {
                        Core::Logger::fatal("Surface is invalid.");
                    }
                    if(vulkan_surface->m_vk_surface_khr == nullptr)
                    {
                        Core::Logger::fatal("Surface khr is invalid.");
                    }

                    VkBool32 is_graphics_support = (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? VK_TRUE : VK_FALSE;
                    Core::Logger::debug("Graphics support for queue family {}: {}", i, is_graphics_support ? "Yes" : "No");
                    if (is_graphics_support) 
                    {
                        graphics_queue_family_index = i;
                    }

                    VkBool32 is_present_support = false;
                    {
                        if(vkGetPhysicalDeviceSurfaceSupportKHR(vk_selected_phys_device, i, vulkan_surface->m_vk_surface_khr, &is_present_support) == VK_SUCCESS)
                        {
                            Core::Logger::debug("Surface support for queue family {}: {}", i, is_present_support ? "Yes" : "No");
                        }
                        else
                        {
                            Core::Logger::error("Failed to query for present support of queue family {}", i);
                        }
                    }

                    if (is_present_support) 
                    {
                        present_queue_family_index = i;
                    }

                    if(graphics_queue_family_index != std::numeric_limits<uint32_t>::max()
                    && present_queue_family_index != std::numeric_limits<uint32_t>::max())
                    {
                        break;
                    }
                }
            }

            // Queue create info
            float queue_priority = 1.0f;

            std::vector<VkDeviceQueueCreateInfo> queue_create_info_array;
            {
                if(graphics_queue_family_index != std::numeric_limits<uint32_t>::max())
                {
                    Core::Logger::trace("Found graphics family queue {}", graphics_queue_family_index);
                    VkDeviceQueueCreateInfo queue_create_info{};
                    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queue_create_info.queueFamilyIndex = graphics_queue_family_index;
                    queue_create_info.queueCount = 1;
                    queue_create_info.pQueuePriorities = &queue_priority;
                    queue_create_info_array.push_back(queue_create_info);
                }
                else
                {
                    Core::Logger::error("Cannot found graphics queue family on device");
                    return nullptr;
                }
                
                if(present_queue_family_index != std::numeric_limits<uint32_t>::max())
                {
                    Core::Logger::trace("Found present family queue {}", present_queue_family_index);

                    if(present_queue_family_index != graphics_queue_family_index)
                    {
                        VkDeviceQueueCreateInfo queue_create_info{};
                        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                        queue_create_info.queueFamilyIndex = present_queue_family_index;
                        queue_create_info.queueCount = 1;
                        queue_create_info.pQueuePriorities = &queue_priority;
                        queue_create_info_array.push_back(queue_create_info);
                    }
                }
                else
                {
                    Core::Logger::fatal("Cannot found present queue family on device");
                    return nullptr;
                }
            }

            // Specify device features
            VkPhysicalDeviceFeatures device_features{};
            device_features.samplerAnisotropy = VK_TRUE;

            // Required device extensions
            std::vector<const char*> device_extensions = 
            {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };

            // Create device
            VkDeviceCreateInfo device_create_info{};
            device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info_array.size());
            device_create_info.pQueueCreateInfos = queue_create_info_array.data();
            device_create_info.pEnabledFeatures = &device_features;

            postProcessDeviceCreateInfo(device_create_info, device_extensions);

            device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
            device_create_info.ppEnabledExtensionNames = device_extensions.data();

            // ... setup queue families, extensions, features
            VkResult result = vkCreateDevice(vk_selected_phys_device, &device_create_info, nullptr, &vk_device);
            if(result != VK_SUCCESS)
            {
                Core::Logger::error("Vulkan CreateDevice failed: {}", VulkanUtility::covertVkResultToString(result));
                return nullptr;
            }
            else
            {
                Core::Logger::trace("Vulkan CreateDevice ok");
            }
        }

        VkQueue graphics_queue;
        VkQueue present_queue;
        vkGetDeviceQueue(vk_device, graphics_queue_family_index, 0, &graphics_queue);
        vkGetDeviceQueue(vk_device, present_queue_family_index, 0, &present_queue);

        // Create vma
        ::VmaAllocator vma_allocator;
        {
            VmaAllocatorCreateInfo allocator_info{};
            //allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;
            allocator_info.physicalDevice = vk_selected_phys_device;
            allocator_info.device = vk_device;
            allocator_info.instance = m_vk_instance;

            VkResult result = vmaCreateAllocator(&allocator_info, &vma_allocator);
            if(result != VK_SUCCESS)
            {
                Core::Logger::error("Vulkan VMA Create failed: {}", VulkanUtility::covertVkResultToString(result));
                return nullptr;
            }
            else
            {
                Core::Logger::trace("Vulkan VMA Create ok");
            }
        }

        return Base::newT<VulkanDevice>(
            std::move(vk_selected_phys_device),
            std::move(vk_device),
            std::move(vma_allocator),
            graphics_queue_family_index,
            present_queue_family_index, 
            std::move(graphics_queue), 
            std::move(present_queue)
        );
    }

    void VulkanInstance::destroyDevice(Interface::RHI::IRenderDevice* device)
    {
        VulkanDevice* vulkan_device = Base::castInterfaceToInstance<VulkanDevice>(device);

        vmaDestroyAllocator(vulkan_device->m_vma_allocator);

        vkDestroyDevice(vulkan_device->m_vk_device, nullptr);
        Base::deleteT(vulkan_device);
    }
}