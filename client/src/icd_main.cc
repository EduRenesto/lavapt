#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

#include <vulkan/vulkan.h>

static bool *m_Initialized = new bool(false);
static int m_Socket;

void init() 
{
    const char *fd = getenv("LAVAPT_FD");

    if(fd != nullptr) 
    {
        m_Socket = atoi(fd);
        return;
    }

    const char* server_ip = getenv("LAVAPT_ADDRESS");
    const char* server_port = getenv("LAVAPT_PORT");

    struct sockaddr_in addr;
    struct sockaddr_in server_addr;

    // TODO error checking
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));

    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // TODO error checking
    if(connect(m_Socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        std::cerr << "failed to connect to " << server_ip << ":" << server_port << std::endl;
    }

    char handshake[strlen("tranquilao truta") + 1];
    read(m_Socket, handshake, strlen("tranquilao truta") + 1);

    *m_Initialized = true;
    setenv("LAVAPT_FD", std::to_string(m_Socket).c_str(), 0);
}

VkResult lavapt_vkCreateInstance(
        const VkInstanceCreateInfo *pInfo,
        const VkAllocationCallbacks *pAllocator,
        VkInstance *pInstance)
{
    size_t func_len = strlen("vkCreateInstance") + 1;
    send(m_Socket, &func_len, sizeof(size_t), 0);
    send(m_Socket, "vkCreateInstance", func_len, 0);

    // send the vkInstanceCreateInfo  struct
    send(m_Socket, &pInfo->flags, sizeof(VkInstanceCreateFlags), 0);

    // send the vkApplicationCreateInfo struct
    // pApplicationName
    size_t pApplicationName_len = strlen(pInfo->pApplicationInfo->pApplicationName) + 1;
    send(m_Socket, &pApplicationName_len, sizeof(size_t), 0);
    send(m_Socket, pInfo->pApplicationInfo->pApplicationName, pApplicationName_len, 0);
    // applicationVersion
    send(m_Socket, &pInfo->pApplicationInfo->applicationVersion, sizeof(uint32_t), 0);
    // pEngineName
    size_t pEngineName_len = strlen(pInfo->pApplicationInfo->pEngineName) + 1;
    send(m_Socket, &pEngineName_len, sizeof(size_t), 0);
    send(m_Socket, pInfo->pApplicationInfo->pEngineName, pEngineName_len, 0);
    // engineVersion
    send(m_Socket, &pInfo->pApplicationInfo->engineVersion, sizeof(uint32_t), 0);
    // apiVersion
    send(m_Socket, &pInfo->pApplicationInfo->apiVersion, sizeof(uint32_t), 0);
    // end vkApplicationCreateInfo

    // enabledLayerCount
    send(m_Socket, &pInfo->enabledLayerCount, sizeof(uint32_t), 0);
    // ppEnabledLayerNames
    size_t enabled_layer_names_len = 0;
    for(auto i = 0; i < pInfo->enabledLayerCount; i++) 
    {
        enabled_layer_names_len += strlen(pInfo->ppEnabledLayerNames[i]) + 1;
    }
    send(m_Socket, &enabled_layer_names_len, sizeof(size_t), 0);
    send(m_Socket, pInfo->ppEnabledLayerNames, enabled_layer_names_len, 0);
    
    // enabledExtensionCount
    send(m_Socket, &pInfo->enabledExtensionCount, sizeof(uint32_t), 0);
    // ppEnabledExtensionNames
    size_t enabled_extension_names_len = 0;
    for(auto i = 0; i < pInfo->enabledExtensionCount; i++) 
    {
        enabled_extension_names_len += strlen(pInfo->ppEnabledExtensionNames[i]) + 1;
    }
    send(m_Socket, &enabled_extension_names_len, sizeof(size_t), 0);
    send(m_Socket, pInfo->ppEnabledExtensionNames, enabled_extension_names_len, 0);

    VkResult return_value;
    read(m_Socket, pInstance, sizeof(VkInstance));
    read(m_Socket, &return_value, sizeof(VkResult));

    return return_value;
}

VkResult lavapt_vkEnumerateInstanceExtensionProperties(
        const char *pLayerName,
        uint32_t *pPropertyCount,
        VkExtensionProperties *pProperties) 
{
    size_t func_len = strlen("vkEnumerateInstanceExtensionProperties") + 1;
    send(m_Socket, reinterpret_cast<char*>(&func_len), sizeof(size_t), 0);
    send(m_Socket, "vkEnumerateInstanceExtensionProperties", func_len, 0);

    unsigned char flags = 0x0;

    unsigned char use_pLayerName = 0x1;
    unsigned char use_pPropertyCount = 0x2;
    unsigned char use_pProperties = 0x4;

    if(pLayerName != nullptr) 
    {
        flags |= use_pLayerName;
    }
    if(*pPropertyCount != 0) 
    {
        flags |= use_pPropertyCount;
    }
    if(pProperties != nullptr)
    {
        flags |= use_pProperties;
    }

    size_t flags_len = sizeof(unsigned char);
    send(m_Socket, reinterpret_cast<char*>(&flags), flags_len, 0);

    if((flags & use_pLayerName) != 0) 
    {
        size_t pLayerName_len = strlen(pLayerName) + 1;
        send(m_Socket, reinterpret_cast<char*>(&pLayerName_len), sizeof(size_t), 0);
        send(m_Socket, pLayerName, pLayerName_len, 0);
    }

    if((flags & use_pPropertyCount) != 0) 
    {
        send(m_Socket, pPropertyCount, sizeof(uint32_t), 0);
    }

    char ret_pPropertyCount[sizeof(uint32_t)] = {0};
    read(m_Socket, ret_pPropertyCount, sizeof(uint32_t));

    *pPropertyCount = *reinterpret_cast<size_t*>(ret_pPropertyCount);

    if((flags & use_pProperties) != 0) 
    {
        char ret_pProperties[sizeof(VkExtensionProperties) * *pPropertyCount] = {0};
        read(m_Socket, ret_pProperties, sizeof(VkExtensionProperties) * *pPropertyCount);

        memcpy(ret_pProperties, pProperties, sizeof(VkExtensionProperties) * *pPropertyCount);
    }

    return VK_SUCCESS;
}

VkResult lavapt_vkEnumerateInstanceVersion(
        uint32_t *pApiVersion)
{
    size_t func_len = strlen("vkEnumerateInstanceVersion") + 1;
    send(m_Socket, reinterpret_cast<char*>(&func_len), sizeof(size_t), 0);
    send(m_Socket, "vkEnumerateInstanceVersion", func_len, 0);

    read(m_Socket, pApiVersion, sizeof(uint32_t));

    return VK_SUCCESS;
}

void lavapt_vkDestroyInstance(
        VkInstance instance,
        const VkAllocationCallbacks *pAllocator)
{
    size_t func_len = strlen("vkDestroyInstance") + 1;
    send(m_Socket, &func_len, sizeof(size_t), 0);
    send(m_Socket, "vkDestroyInstance", func_len, 0);

    send(m_Socket, &instance, sizeof(VkInstance), 0);
}

VkResult lavapt_unimplemented() 
{
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(
            VkInstance pInstance,
            const char* pName)
{
    if(!*m_Initialized) 
    {
        init();
        *m_Initialized = true;
    }

    if(strcmp(pName, "vkCreateInstance") == 0) 
    {
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_vkCreateInstance);
    } else if(strcmp(pName, "vkEnumerateInstanceExtensionProperties") == 0) 
    {
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_vkEnumerateInstanceExtensionProperties);
    } else if(strcmp(pName, "vkEnumerateInstanceVersion") == 0)
    {
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_vkEnumerateInstanceVersion);
    } else if(strcmp(pName, "vkDestroyInstance") == 0)
    {
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_vkDestroyInstance);
    }
    else 
    {
        std::cout << pName << std::endl;
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_unimplemented);
    }
}
