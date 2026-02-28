
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
        Base::InteropOld<Interface::RHI::IRenderSurface> createSurface(Base::InteropOld<Interface::Window::IWindowManager> window_manager, Base::InteropOld<Interface::Window::IWindow> window) override;
        void destroySurface(Base::InteropOld<Interface::RHI::IRenderSurface>) override;

        Base::InteropOld<Interface::RHI::IRenderDevice> createDevice(size_t hardware_index, Base::InteropOld<Interface::RHI::IRenderSurface> surface) override;
        void destroyDevice(Base::InteropOld<Interface::RHI::IRenderDevice> device) override;
    private:
        std::vector<std::string> m_hardware_information_array;

        void postProcessInstanceCreateInfo(VkInstanceCreateInfo&, std::vector<const char*>& extension_names);
        void postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names);

        VkInstance m_vk_instance;
    };
}