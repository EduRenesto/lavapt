#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include <jni.h>
#include <string>

#include <android/log.h>

#define PORT 13000

static int m_ServerSocket;
static bool m_Run = true;

void init_server()
{
    struct sockaddr_in addr;
    int opt = 1;
    int addrlen = sizeof(addr);

    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(m_ServerSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

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

    char cIp[INET_ADDRSTRLEN]= {0};
    inet_ntop(AF_INET, &clientAddr, cIp, INET_ADDRSTRLEN);

    __android_log_print(android_LogPriority::ANDROID_LOG_DEBUG, "lavapt-server", "Connected to %s", cIp);

    while(m_Run)
    {
        char funcLenBuf[sizeof(size_t)];
        int k = read(clientSocket, funcLenBuf, sizeof(size_t));

        __android_log_print(android_LogPriority::ANDROID_LOG_DEBUG, "lavapt-server", "%i", k);

        //size_t funcLen = *reinterpret_cast<size_t*>(funcLenBuf);

        //char funcName[funcLen];
        //read(clientSocket, funcName, funcLen);

        //__android_log_print(android_LogPriority::ANDROID_LOG_DEBUG, "lavapt_server", "%s", funcName);
        m_Run = false;
    }
}

extern "C" JNIEXPORT void JNICALL Java_me_renesto_lavapt_1server_MainActivity_lavapt_1main(JNIEnv *_env, jobject _this) {
    __android_log_print(android_LogPriority::ANDROID_LOG_DEBUG, "lavapt-server", "eae");

    init_server();
    listen_commands();
}
