#include "../inc/base.h"
#include "../inc/storageserver.h"
#include "../inc/namingserver.h"
#include "../inc/client.h"

#define MAX_FILES 500
#define MAX_PATH_LENGTH 256

// Hashing for strings: MurmurOAAT_32
// Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
uint32_t hash_dir(char *str)
{
    // One-byte-at-a-time hash based on Murmur's mix
    int h = 0x12345678;
    for (; *str; ++str)
    {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

void sendInitialization(int nmSocket, uint16_t clientPort, uint16_t storagePort)
{
    uint8_t initMessage[11];
    initMessage[0] = 0x01;                          // Message type: Initialize connection
    initMessage[1] = 1;                             // Sender type: Storage Server
                                                    // *(uint32_t*)(initMessage + 3) = htonl(clientPort); // Client port (network byte order)
    *((uint32_t *)(initMessage + 2)) = storagePort; // Client port (network byte order)
    *((uint32_t *)(initMessage + 6)) = clientPort;  // Client port (network byte order)

    // Send the initialization message
    if (send(nmSocket, initMessage, sizeof(initMessage), 0) < 0)
    {
        perror("[-]Error in sending initialization message");
        exit(1);
    }
    printf("[+]Initialization message sent to Naming Server\n");

    // Wait for confirmation
    uint8_t confirmation[2];
    if (recv(nmSocket, confirmation, sizeof(confirmation), 0) < 0)
    {
        perror("[-]Error in receiving confirmation");
        exit(1);
    }

    // Check confirmation response
    if (confirmation[0] != 0x81 || confirmation[1] != 0x0)
    {
        printf("[-] Init Message: bad code received %d %d\n", confirmation[0], confirmation[1]);
        exit(1);
    }

    printf("[+]Initialization confirmed by Naming Server\n");
}

// Function to send the list of directories to the Naming Server

int list_files_recursive(const char *basepath, char file_paths[MAX_FILES][MAX_PATH_LENGTH], int *count)
{
    char path[MAX_PATH_LENGTH];
    struct dirent *dp;
    DIR *dir = opendir(basepath);

    // Unable to open directory stream
    if (!dir)
    {
        return 0;
    }

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {

            // Construct new path from base path
            // snprintf(path, MAX_PATH_LENGTH, "%s/%s", basepath, dp->d_name);
            if (strcmp(basepath, ".") == 0)
            {
                // Avoid adding './' for the root directory
                snprintf(path, MAX_PATH_LENGTH, "%s", dp->d_name);
            }
            else
            {
                snprintf(path, MAX_PATH_LENGTH, "%s/%s", basepath, dp->d_name);
            }

            // printf("%s\n", path); // Print the path for debugging

            struct stat path_stat;
            stat(path, &path_stat);

            if (!S_ISDIR(path_stat.st_mode))
            {
                if (*count < MAX_FILES)
                {
                    strncpy(file_paths[*count], path, MAX_PATH_LENGTH - 1);
                    file_paths[*count][MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-termination
                    (*count)++;
                }
            }
            else
            {
                if (*count < MAX_FILES)
                {
                    strncpy(file_paths[*count], path, MAX_PATH_LENGTH - 1);
                    file_paths[*count][MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-termination
                    (*count)++;
                }
                // It's a directory, so continue traversing
                list_files_recursive(path, file_paths, count);
            }
        }
    }

    closedir(dir);
    return 1;
}

void sendDirectoryList(int nmSocket)
{

    char filetemp[MAX_FILES][MAX_PATH_LENGTH];
    uint32_t fileList[MAX_FILES];
    int count = 0;
    if (list_files_recursive(".", filetemp, &count))
    {
        for (int i = 0; i < count; i++)
        {
            fileList[i] = hash_dir((filetemp[i]));
            printf("%s %d\n", filetemp[i], fileList[i]);
        }
    }
    else
    {
        printf("Error listing files\n");
    }

    // Calculate the total size of the message

    int totalSize = 50; // 1 byte for message type + 4 bytes for the number of entries

    totalSize += sizeof(fileList[0]) * count;
    // Construct the message
    uint8_t *listMessage = malloc(totalSize);
    int offset = 0;
    listMessage[offset++] = 0x02;                // Message type: List Directories
    *(uint32_t *)(listMessage + offset) = count; // Number of entries
    offset += 4;

    for (int i = 0; i < count; i++)
    {

        *(uint32_t *)(listMessage + offset) = fileList[i];

        offset += sizeof(uint32_t);
    }

    // Send the message to the Naming Server
    send(nmSocket, listMessage, totalSize, 0);
    free(listMessage);

    uint8_t confirmation[2];
    if (recv(nmSocket, confirmation, sizeof(confirmation), 0) < 0)
    {
        perror("[-]Error in receiving confirmation");
        exit(1);
    }

    // Check confirmation response
    if (confirmation[0] != 0x82 || confirmation[1] != 0x0)
    {
        printf("[-] Init Message: bad code received %d %d\n", confirmation[0], confirmation[1]);
        exit(1);
    }

    printf("[+]Directory list sent to Naming Server\n");

    return;
}

StorageServer *InitializeStorageServer()
{

    StorageServer *server = malloc(sizeof(StorageServer));

    // Create socket for Naming Server
    server->nmSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->nmSocket < 0)
    {
        perror("[-]Error in NM socket creation");
        exit(1);
    }

    // Setting up address for Naming Server
    server->nmAddr.sin_family = AF_INET;
    server->nmAddr.sin_port = NM_PORT;
    server->nmAddr.sin_addr.s_addr = inet_addr(NM_SERVER_ADDR);

    // Bind the socket to the Naming Server port
    if (bind(server->nmSocket, (struct sockaddr *)&server->nmAddr, sizeof(server->nmAddr)) < 0)
    {
        perror("[-]Bind to Naming Server Port failed");
        exit(1);
    }

    // Listen on the Naming Server port
    if (listen(server->nmSocket, 10) < 0)
    {
        perror("[-]Error in listening on naming server socket");
        exit(1);
    }
    // Create socket for Client
    server->clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->clientSocket < 0)
    {
        perror("[-]Error in Client socket creation");
        exit(1);
    }

    // Setting up address for Client
    server->clientAddr.sin_family = AF_INET;
    server->clientAddr.sin_port = CLIENT_PORT;
    server->clientAddr.sin_addr.s_addr = inet_addr(NM_SERVER_ADDR);

    int nm_init_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in nm_init_addr;
    memset(&nm_init_addr, '\0', sizeof(nm_init_addr));

    nm_init_addr.sin_family = AF_INET;
    nm_init_addr.sin_port = NM_INIT_PORT;
    nm_init_addr.sin_addr.s_addr = inet_addr(NM_SERVER_ADDR);
    // Bind the socket to the Client port
    if (bind(server->clientSocket, (struct sockaddr *)&server->clientAddr, sizeof(server->clientAddr)) < 0)
    {
        perror("[-]Binding to Client Port failed");
        exit(1);
    }

    // Start listening for clients
    if (listen(server->clientSocket, MAX_CLIENTS) < 0)
    {
        perror("[-]Error in listening on client socket");
        exit(1);
    }

    // Connect to Naming Server
    if (connect(nm_init_sock, (struct sockaddr *)&nm_init_addr, sizeof(nm_init_addr)) < 0)
    {
        perror("[-]Error in connecting to Naming Server");
        exit(1);
    }
    printf("[+]Connected to Naming Server\n");

    // Send Initialization message

    // why does the second work ?
    sendInitialization(nm_init_sock, CLIENT_PORT, NM_PORT);

    // // Send Directory list

    sendDirectoryList(nm_init_sock);

    // Initialize server socket for inter-server communication
    server->storageSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->storageSocket < 0)
    {
        perror("[-]Error in Server socket creation");
        exit(1);
    }

    // Setting up address for Storage Server communication
    server->storageAddr.sin_family = AF_INET;
    server->storageAddr.sin_port = (INTER_SERVER_PORT);
    server->storageAddr.sin_addr.s_addr = inet_addr(NM_SERVER_ADDR);

    // Bind the socket to the Server port
    if (bind(server->storageSocket, (struct sockaddr *)&server->storageAddr, sizeof(server->storageAddr)) < 0)
    {
        perror("[-]Bind failed for server socket");
        exit(1);
    }

    // Listen on the Server port
    if (listen(server->storageSocket, MAX_SS) < 0)
    {
        perror("[-]Error in listening on storage server socket");
        exit(1);
    }

    printf("[+]Storage Server initialized and listening for clients\n");

    return server;
}