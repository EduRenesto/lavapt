#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>

#include <vulkan/vulkan.h>

static bool *m_Initialized = new bool(false);
static int m_Socket;

void init() 
{
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

    std::cout << handshake << std::endl;

    *m_Initialized = true;
}

VkResult lavapt_vkCreateInstance(
        const VkInstanceCreateInfo *pInfo,
        const VkAllocationCallbacks *pAllocator,
        VkInstance *pInstance)
{
    char *payload = (char*) malloc(sizeof(VkInstanceCreateInfo));
    memcpy(payload, pInfo, sizeof(VkInstanceCreateInfo));

    size_t func_len = strlen("vkCreateInstance");
    send(m_Socket, reinterpret_cast<char*>(&func_len), sizeof(size_t), 0);
    send(m_Socket, "vkCreateInstance", func_len, 0);
    
    size_t payload_len = sizeof(VkInstanceCreateInfo);
    send(m_Socket, reinterpret_cast<char*>(&payload_len), sizeof(size_t), 0);
    send(m_Socket, payload, payload_len, 0);

    char ret_len[sizeof(size_t)] = {0};
    read(m_Socket, ret_len, sizeof(size_t));

    char ret[*reinterpret_cast<size_t*>(ret_len)] = {0};
    read(m_Socket, ret, *reinterpret_cast<size_t*>(ret_len));

    char result_len[sizeof(size_t)] = {0};
    read(m_Socket, result_len, sizeof(size_t));
    
    char result[*reinterpret_cast<size_t*>(result_len)] = {0};
    read(m_Socket, result, *reinterpret_cast<size_t*>(result_len));

    VkResult return_value = *reinterpret_cast<VkResult*>(result);

    memcpy(pInstance, ret, sizeof(VkInstance));

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

    std::cout << "flags " << (int) flags << std::endl;

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

    std::cout << *pPropertyCount << std::endl;

    if((flags & use_pProperties) != 0) 
    {
        char ret_pProperties[sizeof(VkExtensionProperties) * *pPropertyCount] = {0};
        read(m_Socket, ret_pProperties, sizeof(VkExtensionProperties) * *pPropertyCount);

        memcpy(ret_pProperties, pProperties, sizeof(VkExtensionProperties) * *pPropertyCount);
    }

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
    }
    else 
    {
        std::cout << pName << std::endl;
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_unimplemented);
    }
}
