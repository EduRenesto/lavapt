#include <string.h>
#include <iostream>

#include <vulkan/vulkan.h>

static bool m_Initialized;

VkResult lavapt_vkCreateInstance(
        const VkInstanceCreateInfo *pInfo,
        const VkAllocationCallbacks *pAllocator,
        VkInstance *pInstance)
{
    return VK_SUCCESS;
}

VkResult lavapt_unimplemented() 
{
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(
            VkInstance pInstance,
            const char* pName)
{
    if(strcmp(pName, "vkCreateInstance") == 0) 
    {
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_vkCreateInstance);
    } else 
    {
        std::cout << pName << std::endl;
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_unimplemented);
    }
}
