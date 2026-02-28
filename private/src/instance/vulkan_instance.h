
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
        Base::Interop::RawRef<Interface::RHI::IRenderSurface> createSurface(Base::Interop::RawRef<Interface::Window::IWindowManager> window_manager, Base::Interop::RawRef<Interface::Window::IWindow> window) override;
        void destroySurface(Base::Interop::RawRef<Interface::RHI::IRenderSurface>) override;

        Base::Interop::RawRef<Interface::RHI::IRenderDevice> createDevice(size_t hardware_index, Base::Interop::RawRef<Interface::RHI::IRenderSurface> surface) override;
        void destroyDevice(Base::Interop::RawRef<Interface::RHI::IRenderDevice> device) override;
    private:
        std::vector<std::string> m_hardware_information_array;

        void postProcessInstanceCreateInfo(VkInstanceCreateInfo&, std::vector<const char*>& extension_names);
        void postProcessDeviceCreateInfo(VkDeviceCreateInfo&, std::vector<const char*>& extension_names);

        VkInstance m_vk_instance;
    };
}




