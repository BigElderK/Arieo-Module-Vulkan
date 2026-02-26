
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
        Base::Interface<Interface::RHI::IRenderSurface> createSurface(Base::Interface<Interface::Window::IWindowManager> window_manager, Base::Interface<Interface::Window::IWindow> window) override;
        void destroySurface(Base::Interface<Interface::RHI::IRenderSurface>) override;

        Base::Interface<Interface::RHI::IRenderDevice> createDevice(size_t hardware_index, Base::Interface<Interface::RHI::IRenderSurface> surface) override;
        void destroyDevice(Base::Interface<Interface::RHI::IRenderDevice> device) override;
    private:
        std::vector<std::string> m_hardware_information_array;

        void postProcessInstanceCreateInfo(VkInstanceCreateInfo&, std::vector<const char*>& extension_names);
        void postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names);

        VkInstance m_vk_instance;
    };
}