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
double batteryLevel = 0.0;
double trashLevel = 0.0;  
void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    ssize_t read;

    printf("Client connected on socket %d\n", client_socket);

    memset(buffer, 0, BUFFER_SIZE);
    read = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (read <= 0) {
        perror("Failed to receive data from client");
        close(client_socket);
        return NULL;
    }

    printf("Received data: %s\n", buffer);

 if (strncmp(buffer, "ROBOT,", 6) == 0) {
        double lat, lon;
        sscanf(buffer + 6, "%lf,%lf,%lf,%lf", &batteryLevel, &trashLevel, &lat, &lon);
        snprintf(latest_location, sizeof(latest_location), "%.2lf,%.2lf", lat, lon);
        printf("Received updates - Battery: %.1f, Trash: %.1f, Location: %s\n", batteryLevel, trashLevel, latest_location);
    } else if (strncmp(buffer, "APP,STATUS", 10) == 0) {
        char response[512];
        snprintf(response, sizeof(response), "{\"battery\": %.1f, \"trash\": %.1f, \"location\": \"%s\"}",
                 batteryLevel, trashLevel, latest_location);
        send(client_socket, response, strlen(response), 0);
        printf("Sent status to APP: %s\n", response);
    } else {
        printf("Unknown command received.\n");
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
            ip[strcspn(ip, "\n")] = 0;
        } else if (strncmp(line, "PORT=", 5) == 0) {
            *port = atoi(line + 5);
        }
    }
    fclose(fp);
}

int main() {
    char ip[20];
    int port;
    readConfig(ip, &port);

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

    printf("Server is running on IP %s at port %d\n", ip, port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket < 0) {
            perror("Failed to accept client");
            continue;
        }

        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, pclient);
        pthread_detach(thread);
    }

    pthread_mutex_destroy(&location_mutex);
    close(server_socket);
    return 0;
}
