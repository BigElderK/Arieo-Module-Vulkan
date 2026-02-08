#include "base/prerequisites.h"
#include "core/core.h"

#include "vulkan_rhi.h"
namespace Arieo
{
    GENERATOR_MODULE_ENTRY_FUN()
    ARIEO_DLLEXPORT void ModuleMain()
    {
        Core::Logger::setDefaultLogger("vulkan_rhi");

        static struct DllLoader
        {
            VulkanInstance vulkan_instance;

            DllLoader()
            {
                vulkan_instance.initialize();
                Core::ModuleManager::registerInterface<Interface::RHI::IRenderInstance>(
                    "vulkan_instance", 
                    &vulkan_instance
                );
            }

            ~DllLoader()
            {
                Core::ModuleManager::unregisterInterface<Interface::RHI::IRenderInstance>(
                    &vulkan_instance
                );
                vulkan_instance.finalize();
            }
        } dll_loader;
    }
}