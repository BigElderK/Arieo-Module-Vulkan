#pragma once
#include "interface/rhi/rhi.h"
#include <vulkan.h>

namespace Arieo
{
    class VulkanUtility
    {
    public:
        static const char* covertVkResultToString(VkResult result);
    };
}