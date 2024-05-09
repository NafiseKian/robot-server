#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CONFIG_FILE "config.txt"
#define BUFFER_SIZE 1024

int main() {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char ip[20];
    int port;
    struct sockaddr_in server_addr;
    int server_socket, client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Read configuration from file
    fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        perror("Failed to open config file");
        return 1;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        if (strncmp(line, "IP_ADDRESS=", 11) == 0) {
            strcpy(ip, line + 11);
            ip[strcspn(ip, "\n")] = 0;  // Remove newline character
        } else if (strncmp(line, "PORT=", 5) == 0) {
            port = atoi(line + 5);
        }
    }
    fclose(fp);
    if (line)
        free(line);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Set up the address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind");
        close(server_socket);
        return 1;
    }

    // Listen
    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen");
        close(server_socket);
        return 1;
    }

    printf("Listening on %s:%d\n", ip, port);

    // Accept connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket < 0) {
            perror("Failed to accept");
            continue;
        }

        // Receive data
        memset(buffer, 0, BUFFER_SIZE);
        read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (read < 0) {
            perror("Failed to receive");
        } else if (read == 0) {
            printf("Client disconnected\n");
        } else {
            printf("Received: %s\n", buffer);
        }

        close(client_socket);
    }

    return 0;
}
