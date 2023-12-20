#include "base.h"

#define NM_SERVER_ADDR "127.0.0.1" // Naming Server IP

// #define INTER_SERVER_PORT 6000 

// #define NM_PORT 5555// Naming Server Port
#define NM_INIT_PORT 4444
// #define CLIENT_PORT 54321 // Port for Client Connection
#define MAX_CLIENTS 10 // Maximum number of clients
#define BUFFER_SIZE 1024

#define MAX_READ 1048576 

#define MAX_SS 10

extern int INTER_SERVER_PORT; 
typedef struct {
    int nmSocket;
    int clientSocket;
    int storageSocket;
    struct sockaddr_in nmAddr;
    struct sockaddr_in clientAddr;
    struct sockaddr_in storageAddr;
} StorageServer;

// void sendInitialization();

// Function to send the list of directories to the Naming Server
void sendDirectoryList(int nmSocket);



StorageServer* InitializeStorageServer();


void handleStorageServerRequest(int socket);

int extract_tar_and_delete(const char *destination_path);