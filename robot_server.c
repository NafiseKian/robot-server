#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define CONFIG_FILE "config.txt"
#define BUFFER_SIZE 1024

char latest_location[256] = "";
pthread_mutex_t location_mutex;

void *handle_client(void *arg) {

    printf("a client is connected\n");
    int client_socket = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    ssize_t read;

    // Initial message to determine client type
    memset(buffer, 0, BUFFER_SIZE);
    read = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (read <= 0) {
        perror("Failed to receive initial client type");
        close(client_socket);
        return NULL;
    }

    // Determine if publisher or subscriber
    if (strcmp(buffer, "ROBOT") == 0) {
        // Handle robot publishing location
        while ((read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            pthread_mutex_lock(&location_mutex);
            strcpy(latest_location, buffer);
            printf("latest location is --> %s",latest_location);
            pthread_mutex_unlock(&location_mutex);
        }
    } else if (strcmp(buffer, "APP") == 0) {
        // Handle app subscribing to location
        while (1) {
            pthread_mutex_lock(&location_mutex);
            strcpy(buffer, latest_location);
            pthread_mutex_unlock(&location_mutex);
            send(client_socket, buffer, strlen(buffer), 0);
            sleep(1);  // Send update every second
        }
    }

    close(client_socket);
    return NULL;
}

void readConfig(char *ip, int *port) {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        perror("Failed to open config file");
        exit(1);
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "IP_ADDRESS=", 11) == 0) {
            strcpy(ip, line + 11);
            ip[strcspn(ip, "\n")] = 0; // Remove newline character
        } else if (strncmp(line, "PORT=", 5) == 0) {
            *port = atoi(line + 5);
        }
    }
    fclose(fp);
}

int main() {
    char ip[20];
    int port;
    readConfig(ip, &port); // Read IP address and port from config file

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Failed to listen");
        close(server_socket);
        exit(1);
    }

    pthread_mutex_init(&location_mutex, NULL);

    // Main loop to accept clients
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket < 0) {
            perror("Failed to accept");
            continue;
        }

        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, pclient) != 0) {
            perror("Failed to create thread");
            free(pclient);
        }
        pthread_detach(thread);  // Don't wait for thread on main thread
    }

    // Cleanup
    pthread_mutex_destroy(&location_mutex);
    close(server_socket);
    return 0;
}
