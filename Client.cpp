#include <iostream>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8080

void receiveMessages(int sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;
        std::cout << buffer << std::flush;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
    std::cout << "✅ Connected to server!\n";

    std::thread(receiveMessages, sock).detach();

    std::string msg;
    while (true) {
        std::getline(std::cin, msg);
        if (msg == "exit") break;
        msg += "\n"; // Add newline so it looks clean
        send(sock, msg.c_str(), msg.length(), 0);
    }

    close(sock);
    return 0;
}
