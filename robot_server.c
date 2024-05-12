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

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    pthread_mutex_init(&location_mutex, NULL);

    // Existing setup code here...

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
    return 0;
}
