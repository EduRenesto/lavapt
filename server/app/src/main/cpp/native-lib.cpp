#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include <jni.h>
#include <string>

#include <android/log.h>

#include "vulkan_wrapper.h"
#include <vulkan/vulkan.h>

#define PORT 13000

static int m_ServerSocket;
static bool m_Run = true;

void init_server()
{
    struct sockaddr_in addr;
    int opt = 1;
    int addrlen = sizeof(addr);

    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(m_ServerSocket, SOL_SOCKET, SO_KEEPALIVE | SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if(bind(m_ServerSocket, (struct sockaddr*) &addr, addrlen) < 0) {
        __android_log_print(android_LogPriority::ANDROID_LOG_ERROR, "lavapt_server", "failed to bind to port %d", PORT);
    }

    listen(m_ServerSocket, 3);
}

void listen_commands()
{
    int clientSocket;
    struct sockaddr_in clientAddr;
    size_t addrlen = sizeof(clientAddr);

    clientSocket = accept(m_ServerSocket, (struct sockaddr*) &clientAddr, (socklen_t*) &addrlen);

    send(clientSocket, "tranquilao_truta", strlen("tranquilao truta") + 1, 0);

    char cIp[INET_ADDRSTRLEN]= {0};
    inet_ntop(AF_INET, &clientAddr, cIp, INET_ADDRSTRLEN);

    __android_log_print(android_LogPriority::ANDROID_LOG_DEBUG, "lavapt-server", "Connected to %s", cIp);

    while(m_Run)
    {
        char funcLenBuf[sizeof(size_t)];
        int k = read(clientSocket, funcLenBuf, sizeof(size_t));

        if(k == 0)
        {
            m_Run = false;
            break;
        }

        size_t funcLen = *reinterpret_cast<size_t*>(funcLenBuf);

        char funcName[funcLen];
        read(clientSocket, funcName, funcLen);

        __android_log_print(android_LogPriority::ANDROID_LOG_DEBUG, "lavapt_server", "%s", funcName);

        if(strcmp(funcName, "vkEnumerateInstanceExtensionProperties") == 0) 
        {
            char *pLayerName;
            uint32_t *pPropertyCount;
            VkExtensionProperties *pProperties;

            char flags_buf[sizeof(unsigned char)] = {0};
            read(clientSocket, flags_buf, sizeof(unsigned char));

            unsigned char flags = *reinterpret_cast<unsigned char*>(flags_buf);

            unsigned char use_pLayerName = 0x1;
            unsigned char use_pPropertyCount = 0x2;
            unsigned char use_pProperties = 0x4;

            if((flags & use_pLayerName) != 0) 
            {
                // lets test doing this faster
                size_t len = 0;
                read(clientSocket, &len, sizeof(size_t));
                
                pLayerName = (char*) malloc(len);
                read(clientSocket, pLayerName, len);
            } else 
            {
                pLayerName = nullptr;
            }

            pPropertyCount = (uint32_t*) malloc(sizeof(uint32_t));
            if((flags & use_pPropertyCount) != 0)
            {
                read(clientSocket, pPropertyCount, sizeof(uint32_t));
            }
            else
            {
                *pPropertyCount = 0;
            }

            if((flags & use_pProperties) != 0) 
            {
                pProperties = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties) * *pPropertyCount);
            }
            else
            {
                pProperties = nullptr;
            }

            vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);

            send(clientSocket, pPropertyCount, sizeof(uint32_t), 0);
            
            if((flags & use_pProperties) != 0)
            {
                send(clientSocket, pProperties, sizeof(VkExtensionProperties) * *pPropertyCount, 0);
            }
        }
    }
}

extern "C" JNIEXPORT void JNICALL Java_me_renesto_lavapt_1server_MainActivity_lavapt_1main(JNIEnv *_env, jobject _this) {
    __android_log_print(android_LogPriority::ANDROID_LOG_DEBUG, "lavapt-server", "eae");

    InitVulkan();

    init_server();
    listen_commands();
}
