#include "../inc/base.h"
#include "../inc/storageserver.h"
#include "../inc/namingserver.h"
#include "../inc/client.h"

int CLIENT_PORT = 5432; // Default port for Client Connection
int NM_PORT = 5555;     // Default port for Naming Server Connection
int INTER_SERVER_PORT = 6000;

int move_to_ss_dir(const char *subfolderName)
{
    const char *baseDir = "../data/SS_";
    char subfolderPath[1024];

    // Construct the full path for the subfolder
    snprintf(subfolderPath, sizeof(subfolderPath), "%s%s", baseDir, subfolderName);
    printf("%s\n", subfolderPath);
    // Check if the subfolder exists
    struct stat st;
    if (stat(subfolderPath, &st) == -1)
    {
        // Subfolder doesn't exist, so create it
        if (mkdir(subfolderPath, 0777) != 0)
        {
            perror("Failed to create subfolder");
            return -1;
        }
    }

    // Change the working directory to the subfolder
    if (chdir(subfolderPath) != 0)
    {
        perror("Failed to change working directory");
        return -1;
    }

    printf("Working directory set to '%s'\n", subfolderPath);
    return 0;
}

int main(int argc, char *argv[])
{
    // Expecting at least 4 arguments (including the program name)
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <directory> [client_port] [naming_server_port] [inter_server_port]\n", argv[0]);
        return 1;
    }

    const char *directory = argv[1]; // The directory

    // Override ports if provided
    if (argc > 2)
    {
        CLIENT_PORT = atoi(argv[2]);
    }
    if (argc > 3)
    {
        NM_PORT = atoi(argv[3]);
    }
    if (argc > 4)
    {
        INTER_SERVER_PORT = atoi(argv[4]);
    }

    if (move_to_ss_dir(directory) != 0)
    {
        printf("[-]Could not setup storage server");
        return 1; // Exit if the directory setup fails
    }
    // printf("Directory: %s\n", directory);
    // printf("Using client port: %d\n", CLIENT_PORT);
    // printf("Using naming server port: %d\n", NM_PORT);

    // printf("%s %d %d \n",directory,CLIENT_PORT,NM_PORT);
    // return 0;

    StorageServer *server = InitializeStorageServer();

    // TODO: Handle client requests and file operations
    fd_set readfds;
    int max_sd;
    // Assume server and client sockets are set up and listening

    while (1)
    {
        FD_ZERO(&readfds);

        // Add naming server socket to set
        FD_SET(server->nmSocket, &readfds);
        max_sd = server->nmSocket;

        // Add client socket to set
        FD_SET(server->clientSocket, &readfds);
        if (server->clientSocket > max_sd)
        {
            max_sd = server->clientSocket;
        }

        FD_SET(server->storageSocket, &readfds);
        if (server->storageSocket > max_sd)
        {
            max_sd = server->storageSocket;
        }

        // Wait for an activity on any of the sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 1) && (errno != EINTR))
        {
            perror("Select error");
            continue; // Continue the loop instead of exiting
        }

        // If something happened on the naming server socket
        if (FD_ISSET(server->nmSocket, &readfds))
        {

            struct sockaddr_in client_addr;
            socklen_t addr_size;
            int newSocket = accept(server->nmSocket, (struct sockaddr *)&client_addr, &addr_size);
            handleNamingServerRequest(newSocket);
        }

        // If something happened on the client socket
        if (FD_ISSET(server->clientSocket, &readfds))
        {

            struct sockaddr_in client_addr;
            socklen_t addr_size;
            int newSocket = accept(server->clientSocket, (struct sockaddr *)&client_addr, &addr_size);
            if (newSocket < 0)
            {
                perror("Accept failed");
            }
            else
            {

                handleClientRequest(newSocket);
            }
        }

        if (FD_ISSET(server->storageSocket, &readfds))
        {

            struct sockaddr_in client_addr;
            socklen_t addr_size;
            int newSocket = accept(server->storageSocket, (struct sockaddr *)&client_addr, &addr_size);
            if (newSocket < 0)
            {
                perror("Accept failed");
            }
            else
            {

                handleStorageServerRequest(newSocket);
            }
        }
    }
    // Cleanup
    close(server->clientSocket);
    close(server->nmSocket);
    free(server);

    return 0;
}
