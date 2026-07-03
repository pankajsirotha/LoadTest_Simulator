#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <vector>

// --- OS Compatibility Check ---
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    // Tell the compiler to link the Windows socket library
    #pragma comment(lib, "ws2_32.lib") 
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <cstring>
#endif
// ------------------------------

std::mutex matchmaking_mutex;

void handle_client(int client_socket) {
    char buffer[1024] = {0};
    
    // 1. Read the LOGIN command (using recv instead of read for Windows compatibility)
    recv(client_socket, buffer, 1024, 0);
    std::string response = "ACK_LOGIN\n";
    send(client_socket, response.c_str(), response.length(), 0);
    
    // 2. Read the MATCHMAKE command
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, 1024, 0);
    
    // --- THE BOTTLENECK ---
    matchmaking_mutex.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
    matchmaking_mutex.unlock();
    // ----------------------
    
    response = "ACK_MATCH\n";
    send(client_socket, response.c_str(), response.length(), 0);
    
    // Close the socket safely depending on the OS
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

int main() {
    // --- Windows Socket Initialization ---
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }
#endif
    // -------------------------------------

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // OS-agnostic socket options
#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1000) < 0) { 
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Mock Matchmaking Server running on port 8080...\n";

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        std::thread(handle_client, new_socket).detach();
    }
    
    // Cleanup Windows sockets when shutting down
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}