#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <algorithm>

#define PORT 8080

std::vector<int> clients;
std::mutex clients_mutex;

// Broadcast message to all clients except sender
void broadcast(const std::string& message, int sender_fd = -1) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int client : clients) {
        if (client != sender_fd) {
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

// Handle individual client
void handleClient(int client_fd) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());
            close(client_fd);
            break;
        }

        std::string message = "[Client " + std::to_string(client_fd) + "]: " + buffer;
        std::cout << message << std::flush;
        broadcast(message, client_fd);
    }
}

// Server's own input thread
void serverInputThread() {
    std::string adminMsg;
    while (true) {
        std::getline(std::cin, adminMsg);
        if (adminMsg == "exit") break;

        std::string fullMsg = "[Server]: " + adminMsg + "\n";
        broadcast(fullMsg);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    std::cout << "🟢 Server is running on port " << PORT << "...\n";
    std::thread(serverInputThread).detach();  // Launch admin input thread

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_fd);
        std::thread(handleClient, client_fd).detach();
    }

    return 0;
}
