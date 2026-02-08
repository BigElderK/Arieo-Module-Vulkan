
#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>

namespace Arieo
{
    class VulkanInstance final
        : public Interface::RHI::IRenderInstance
    {
    public:
        void initialize();
        void finalize();        
        
        std::vector<std::string>& getHardwareInfomations() override;
        Interface::RHI::IRenderSurface* createSurface(Interface::Window::IWindow* window) override;
        void destroySurface(Interface::RHI::IRenderSurface*) override;

        Interface::RHI::IRenderDevice* createDevice(size_t hardware_index, Interface::RHI::IRenderSurface* surface) override;
        void destroyDevice(Interface::RHI::IRenderDevice*) override;
    private:
        std::vector<std::string> m_hardware_information_array;

        void postProcessInstanceCreateInfo(VkInstanceCreateInfo&, std::vector<const char*>& extension_names);
        void postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names);

        VkInstance m_vk_instance;
    };
}