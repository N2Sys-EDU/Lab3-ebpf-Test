#include <gtest/gtest.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <random>
#include <string>

static void changeThreadNs(const char *ns) {
    int fd = open(ns, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (setns(fd, CLONE_NEWNET) == -1) {
        perror("setns");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

std::tuple<bool, std::string> tryToConnect(const char* host, uint16_t port) {
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        return std::make_tuple(false, "Failed to create socket");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    auto flags = fcntl(client_sock, F_GETFL);
    if (fcntl(client_sock, F_SETFL, (O_NONBLOCK | flags)) == -1) {
        perror("fcntl");
        close(client_sock);
        return std::make_tuple(false, "Failed to set socket to non-blocking mode");
    }
    int status = connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (status == -1 && (errno == EINPROGRESS || errno == EAGAIN)) {
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(client_sock, &write_fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        status = select(client_sock + 1, NULL, &write_fds, NULL, &timeout);
        // get error code with getsockopt
        int err;
        socklen_t err_len = sizeof(err);
        getsockopt(client_sock, SOL_SOCKET, SO_ERROR, &err, &err_len);
        if (status == 0) {
            close(client_sock);
            return std::make_tuple(false, "Connection timeout 1");
        }
        if (err != 0) {
            close(client_sock);
            return std::make_tuple(false, "Connection failed");
        }
        char buff[128];
        int count = 0;
        while (recv(client_sock, buff, 128, 0) <= 0) {
            usleep(1000);
            count ++;
            if (count == 1000) {
                close(client_sock);
                return std::make_tuple(false, "Connection timeout 2");
            }
        }
        close(client_sock);
    } else if (status == 0) {
        char buff[128];
        int count = 0;
        while (recv(client_sock, buff, 128, 0) <= 0) {
            usleep(1000);
            count ++;
            if (count == 1000) {
                close(client_sock);
                return std::make_tuple(false, "Connection timeout 3");
            }
        }
        close(client_sock);
    } else {
        close(client_sock);
        return std::make_tuple(false, "Connection failed");
    }
    return std::make_tuple(true, "");
}

TEST(Lab3, NAT) {
    system("../scripts/load_ebpfs.sh");
    changeThreadNs("/var/run/netns/ns1");
    system("../scripts/stop_server.sh");
    system("../scripts/start_server.sh ns4 8888");
    usleep(1000);
    auto [success, msg] = tryToConnect("10.0.0.4", 8888);
    system("../scripts/stop_server.sh");
    ASSERT_TRUE(success) << msg;
}

TEST(Lab3, ProxyAdd) {
    system("../scripts/load_ebpfs.sh");
    changeThreadNs("/var/run/netns/ns1");
    system("../scripts/stop_server.sh");
    system("../scripts/clear_rules.sh");
    uint16_t baned_port = 8889;
    system((std::string("../scripts/add_rule.sh ") + std::to_string(baned_port)).c_str());
    system((std::string("../scripts/start_server.sh ns3 ") + std::to_string(baned_port)).c_str());
    usleep(1000);
    auto [success, msg] = tryToConnect("10.0.0.3", baned_port);
    system("../scripts/stop_server.sh");
    ASSERT_TRUE(success) << msg;
}

TEST(Lab3, ProxyAddDel) {
    system("../scripts/load_ebpfs.sh");
    changeThreadNs("/var/run/netns/ns1");
    system("../scripts/stop_server.sh");
    system("../scripts/clear_rules.sh");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> dis(10000, 20000);
    uint16_t baned_port = dis(gen);
    system((std::string("../scripts/add_rule.sh ") + std::to_string(baned_port)).c_str());
    system((std::string("../scripts/del_rule.sh ") + std::to_string(baned_port)).c_str());
    system((std::string("../scripts/start_server.sh ns3 ") + std::to_string(baned_port)).c_str());
    usleep(1000);
    auto [success, msg] = tryToConnect("10.0.0.3", baned_port);
    system("../scripts/stop_server.sh");
    ASSERT_TRUE(success) << msg;
}

TEST(Lab3, ProxyAddMulti) {
    system("../scripts/load_ebpfs.sh");
    changeThreadNs("/var/run/netns/ns1");
    system("../scripts/stop_server.sh");
    system("../scripts/clear_rules.sh");
    std::vector<uint16_t> baned_ports;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> dis(10000, 20000);
    for (int i = 0; i < 10; i ++)
        baned_ports.push_back(dis(gen));
    std::string cmd = "";
    for (auto port : baned_ports) {
        system((std::string("../scripts/add_rule.sh ") + std::to_string(port)).c_str());
        cmd += " " + std::to_string(port);
    }
    system((std::string("../scripts/start_server.sh ns3") + cmd).c_str());
    usleep(1000);
    for (auto port : baned_ports) {
        auto [success, msg] = tryToConnect("10.0.0.3", port);
        if (!success) {
            system("../scripts/stop_server.sh");
            ASSERT_TRUE(success) << msg;
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
