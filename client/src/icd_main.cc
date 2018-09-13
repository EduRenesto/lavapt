#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>

#include <vulkan/vulkan.h>

static bool m_Initialized;
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

VkResult lavapt_unimplemented() 
{
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(
            VkInstance pInstance,
            const char* pName)
{
    if(!m_Initialized) 
    {
        init();
    }

    if(strcmp(pName, "vkCreateInstance") == 0) 
    {
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_vkCreateInstance);
    } else 
    {
        std::cout << pName << std::endl;
        return reinterpret_cast<PFN_vkVoidFunction>(&lavapt_unimplemented);
    }
}
