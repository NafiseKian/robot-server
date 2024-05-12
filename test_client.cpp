#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

const std::string SERVER_IP = "34.165.89.174"; // Set the server IP address
const int SERVER_PORT = 3389; // Set the server port

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        return 1;
    }

    // Send the robot identifier to the server
    const char* msg = "ROBOT,10,20";
    send(sock, msg, strlen(msg), 0);

   

    close(sock);
    return 0;
}
