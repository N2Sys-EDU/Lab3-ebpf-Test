#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <atomic>
#include <fstream>

std::atomic<int> counter(0);

void startServer(int port) {
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock == -1) {
        perror("socket");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_sock);
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;
    counter += 1;

    char tmp[32];
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_data_len = recvfrom(server_sock, tmp, sizeof(tmp), 0, (struct sockaddr *)&client_addr, &client_addr_len);

        const char *message = "Hello World";
        sendto(server_sock, message, strlen(message), 0, (struct sockaddr *)&client_addr, client_addr_len);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port1> <port2> ..." << std::endl;
        return 1;
    }

    std::vector<std::thread> server_threads;

    for (int i = 1; i < argc; ++i) {
        int port = std::stoi(argv[i]);
        server_threads.emplace_back(startServer, port);
    }

    while (counter < argc - 1) {
        usleep(3000);
    }

    std::ofstream file(".server_udp_started");
    file << "done";
    file.close();

    for (auto &t : server_threads) {
        t.join();
    }

    return 0;
}