// TCP Server with Custom Response

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

// Function declarations
int createServerSocket();
void acceptClientConnections(int server_sock);
void handleClient(int client_sock);

// int main() {
//     int server_sock = createServerSocket();

//     // Accept client connections and handle messages
//     acceptClientConnections(server_sock);

//     close(server_sock);
//     return 0;
// }

// Function to create a server socket
int createServerSocket() {
    int server_sock;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    return server_sock;
}

// Function to accept client connections and handle messages
void acceptClientConnections(int server_sock) {
    int addrlen = sizeof(struct sockaddr_in);
    int client_sock;
    struct sockaddr_in client_address;

    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected. IP: %s, Port: %d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Handle messages for each client in a new process
        if (fork() == 0) {
            close(server_sock);  // Close in child process
            handleClient(client_sock);
            exit(0);  // Exit child process after handling the client
        } else {
            close(client_sock);  // Close in parent process
        }
    }
}

// Function to handle messages for a specific client
void handleClient(int client_sock) {
    char buffer[MAX_BUFFER_SIZE];
    const char *response = "Server received your message. Acknowledgment sent.";

    while (1) {
        // Receive message from client
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_sock, buffer, MAX_BUFFER_SIZE, 0) <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        // Send a custom acknowledgment message back to the client
        printf("Received message from client: %s\n", buffer);
        send(client_sock, response, strlen(response), 0);
    }

    close(client_sock);
}
