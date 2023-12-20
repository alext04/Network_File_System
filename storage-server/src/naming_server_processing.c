#include "../inc/base.h"
#include "../inc/storageserver.h"
#include "../inc/namingserver.h"
#include "../inc/client.h"

uint8_t nm_buffer[MAX_READ];

char *getpath()
{

    // Extract path length (4 bytes after the operation code)
    uint32_t pathLength;
    memcpy(&pathLength, nm_buffer + 1, sizeof(pathLength));

    // Convert path length to host byte order
    pathLength = (pathLength);

    // Extract the path
    char *path = malloc(pathLength + 1); // +1 for null-terminator
    if (path == NULL)
    {
        perror("Failed to allocate memory for path");
        return NULL;
    }
    memcpy(path, nm_buffer + 5, pathLength);
    path[pathLength] = '\0'; // Null-terminate the string

    // Check for null byte in buffer
    if (nm_buffer[5 + pathLength] != '\0')
    {
        fprintf(stderr, "Path data does not end with a null byte\n");
        free(path);
        return NULL;
    }

    return path;
}

int createFirstDir(const char *path)
{
    char *prefix;
    struct stat st = {0};

    // Find the position of the first '/'
    char *slashPos = strchr(path, '/');
    if (slashPos == NULL)
    {
        printf("Client ID not in Path\n");
        return -1;
    }

    // Allocate memory and copy the prefix
    prefix = (char *)malloc(slashPos - path + 1);
    if (prefix == NULL)
    {
        perror("Failed to allocate memory");
        return -1;
    }
    strncpy(prefix, path, slashPos - path);
    prefix[slashPos - path] = '\0'; // Null-terminate the string

    // Check if directory exists
    if (stat(prefix, &st) == -1)
    {
        // Directory does not exist, create it
        if (mkdir(prefix, 0700) == -1)
        { // 0700 permissions - owner can read, write, and execute
            perror("Failed to create directory");
            free(prefix);
            return -1;
        }
    }

    free(prefix);
    return 0;
}

int deleteDirectory(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("Failed to open directory");
        return -1;
    }

    struct dirent *entry;
    int ret = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char *fullPath = malloc(strlen(path) + strlen(entry->d_name) + 2);
        sprintf(fullPath, "%s/%s", path, entry->d_name);

        struct stat path_stat;
        stat(fullPath, &path_stat);

        if (S_ISDIR(path_stat.st_mode))
        {
            // It's a directory, delete its contents recursively
            if (deleteDirectory(fullPath) != 0)
            {
                ret = -1;
                break;
            }
            if (rmdir(fullPath) != 0)
            {
                perror("Failed to remove directory");
                ret = -1;
                break;
            }
        }
        else
        {
            // It's a file, delete it
            if (unlink(fullPath) != 0)
            {
                perror("Failed to remove file");
                ret = -1;
                break;
            }
        }

        free(fullPath);
    }

    closedir(dir);

    // delete the directory itself
    if (ret == 0 && rmdir(path) != 0)
    {
        perror("Failed to remove directory");
        ret = -1;
    }

    return ret;
}
CopyOperationData *processCopyOperationInput()
{
    CopyOperationData *data = malloc(sizeof(CopyOperationData));

    size_t offset = 1;

    // Extract the length of the source path
    uint32_t srcPathLength;
    memcpy(&srcPathLength, nm_buffer + 1, sizeof(srcPathLength));
    srcPathLength = srcPathLength; // Convert to host byte order
    offset += sizeof(srcPathLength);

    // Extract the source path
    data->srcPath = malloc(srcPathLength + 1);

    memcpy(data->srcPath, nm_buffer + offset, srcPathLength);
    data->srcPath[srcPathLength] = '\0';
    offset += srcPathLength + 1; // +1 for null byte
    printf("++++  %s  \n", data->srcPath);
    // Extract IP address of Destination SS
    memcpy(&data->destIP, nm_buffer + offset, sizeof(data->destIP));
    data->destIP = data->destIP; // Convert to host byte order
    offset += sizeof(data->destIP);

    // Extract port of Destination SS
    memcpy(&data->destPort, nm_buffer + offset, sizeof(data->destPort));
    data->destPort = data->destPort; // Convert to host byte order
    offset += sizeof(data->destPort);

    // Extract the length of the destination path
    uint32_t destPathLength;
    memcpy(&destPathLength, nm_buffer + offset, sizeof(destPathLength));
    destPathLength = destPathLength; // Convert to host byte order
    offset += sizeof(destPathLength);

    printf("++++  %d  \n", destPathLength);
    // Extract the destination path
    data->destPath = malloc(destPathLength + 1);

    memcpy(data->destPath, nm_buffer + offset, destPathLength);
    data->destPath[destPathLength] = '\0';
    return data;
}

// 0x03
void handleCreateFile(int socket)
{
    char *path = getpath();

    createFirstDir(path);
    FILE *file = fopen(path, "w");

    uint8_t response[2];
    response[0] = 0x83; // Protocol code for Create File response
    if (file == NULL)
    {
        perror("[-]Error Creating file");
        response[1] = -1;
    }
    else
    {
        printf("File created successfully: %s\n", path);
        response[1] = 0;
        fclose(file); // Close the file
    }

    send(socket, response, sizeof(response), 0);
    free(path);
}

// 0x04
void handleCreateDir(int socket)
{

    char *path = getpath();
    createFirstDir(path);

    uint8_t response[2];
    response[0] = 0x84; // Protocol code for Create File response

    if (mkdir(path, 0777) != 0)
    { // 0777 will be modified by the process's umask

        response[1] = -1;
    }
    else
    {
        printf("Directory created successfully: %s\n", path);
        response[1] = 0;
    }

    send(socket, response, sizeof(response), 0);

    free(path);
}

// 0x05
void handleDeleteFile(int socket)
{
    // Implementation is similar to handleCreateFile
    char *path = getpath();

    uint8_t response[2];
    response[0] = 0x85; // Protocol code for Delete File response

    if (unlink(path) != 0)
    {
        // If the file deletion fails
        response[1] = -1;
    }
    else
    {
        // File deleted successfully
        printf("File deleted successfully: %s\n", path);
        response[1] = 0;
    }

    // Send the response back to the client
    send(socket, response, sizeof(response), 0);

    free(path);
}

// 0x06
void handleDeleteDir(int socket)
{

    // Implementation is similar to handleCreateFile
    char *path = getpath();

    uint8_t response[2];
    response[0] = 0x86; // Protocol code for Delete Directory response

    if (deleteDirectory(path) != 0)
    {
        // If directory deletion fails
        response[1] = -1;
    }
    else
    {
        // Directory deleted successfully
        printf("Directory deleted successfully: %s\n", path);
        response[1] = 0;
    }

    // Send the response back to the client
    send(socket, response, sizeof(response), 0);

    free(path);
}

// look into this more

void handleCopyFile(int socket1)
{
    CopyOperationData *opData = processCopyOperationInput();

    if (opData->destIP == inet_addr("127.0.0.1") && opData->destPort == NM_PORT)
    {
        printf("1\n");
        uint8_t response_tonm[2];
        response_tonm[0] = 0x87;
        response_tonm[1] = 0;
        // make local copy
        FILE *srcFile = fopen(opData->srcPath, "rb");
        if (srcFile == NULL)
        {
            response_tonm[1] = -1;
            send(socket1, response_tonm, 2 * sizeof(uint8_t), 0);
            perror("Error opening source file");
            return;
        }

        FILE *destFile = fopen(opData->destPath, "wb");
        if (destFile == NULL)
        {
            response_tonm[1] = -1;
            send(socket1, response_tonm, 2 * sizeof(uint8_t), 0);
            perror("Error opening/creating destination file");
            fclose(srcFile); // Close the source file before returning
            return;
        }

        char buffer[BUFFER_SIZE];
        size_t bytesRead;

        // Read from source and write to destination
        while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, srcFile)) > 0)
        {
            fwrite(buffer, 1, bytesRead, destFile);
        }

        // Close the files
        fclose(srcFile);
        fclose(destFile);
        send(socket1, response_tonm, 2 * sizeof(uint8_t), 0);
        return;
    }

    int sock;
    struct sockaddr_in destAddr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return;
    }

    // Set up the destination address
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = (opData->destPort);
    destAddr.sin_addr.s_addr = (opData->destIP);

    // Connect to the destination server
    if (connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0)
    {
        perror("Connection to the server failed");
        close(sock);
        return;
    }

    // Read the source file
    FILE *file = fopen(opData->srcPath, "rb");
    if (file == NULL)
    {
        perror("Failed to open source file");
        close(sock);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent = malloc(fileSize);
    if (fileContent == NULL)
    {
        perror("Memory allocation for file content failed");
        fclose(file);
        close(sock);
        return;
    }

    fread(fileContent, 1, fileSize, file);
    fclose(file);

    // Prepare the message

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    uint32_t destPathLength = strlen(opData->destPath);
    uint32_t destPathLengthNet = destPathLength;
    uint32_t fileSizeNet = fileSize;
    size_t messageSize = 1 + sizeof(destPathLengthNet) + destPathLength + 1 + sizeof(fileSizeNet) + fileSize + 1;
    uint8_t *message = malloc(messageSize);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    if (message == NULL)
    {
        perror("Memory allocation for message failed");
        free(fileContent);
        close(sock);
        return;
    }

    // add the file name to the destination path ?  /// not needed

    size_t offset = 0;
    message[offset++] = 0x19; // Protocol code
    memcpy(message + offset, &destPathLengthNet, sizeof(destPathLengthNet));
    offset += sizeof(destPathLengthNet);
    memcpy(message + offset, opData->destPath, destPathLength);
    offset += destPathLength;
    message[offset++] = '\0'; // Null byte for path

    memcpy(message + offset, &fileSizeNet, sizeof(fileSizeNet));
    offset += sizeof(fileSizeNet);
    memcpy(message + offset, fileContent, fileSize);
    offset += fileSize;
    message[offset] = '\0'; // Null byte for content

    // Send the message

    // redirect message

    if (send(sock, message, messageSize, 0) < 0)
    {
        perror("Failed to send message");
    }
    else
    {
        printf("Message sent successfully\n");
    }

    uint8_t response[2];
    uint8_t status = 0;
    ssize_t received = recv(sock, &response, sizeof(response), 0);
    if (received < 0)
    {
        perror("Failed to receive response");
    }
    else if (received == 0)
    {
        printf("Server closed the connection\n");
    }
    else
    {
        // Check the protocol code and status
        uint8_t protocolCode = response[0];
        status = response[1];

        if (protocolCode == 0x99)
        {
            if (status == 0)
            {
                printf("File transfer successful\n");
            }
            else
            {
                printf("File transfer failed with status code: %d\n", status);
            }
        }
        else
        {
            printf("Unexpected protocol code received: %d\n", protocolCode);
        }
    }
    uint8_t response_tonm[2];
    response_tonm[0] = 0x87;
    response_tonm[1] = status;
    send(socket1, response_tonm, sizeof(response_tonm), 0);
    free(message);
    free(fileContent);
    close(sock);

    free(opData);
}

void create_tar(const char *directory, const char *tar_name)
{
    char command[1024];

    // Ensure that the directory and tar_name are safely escaped
    // to prevent command injection vulnerabilities.

    // Create the command string
    snprintf(command, sizeof(command), "tar -czf %s %s", tar_name, directory);

    // Execute the command
    int status = system(command);

    if (status == -1)
    {
        // Handle error in system call
        perror("Failed to execute command");
    }
}

int extract_tar_and_delete_killme(const char *destination_path, const char *tar_path)
{
    char command[1024];
    // Ensure destination_path exists and create it if not
    snprintf(command, sizeof(command), "mkdir -p %s", destination_path);
    system(command);

    // Extract the TAR file
    snprintf(command, sizeof(command), "tar -xzf %s -C %s", tar_path, destination_path);
    int status = system(command);
    if (status != 0)
    {
        perror("Failed to extract TAR file");
        return -1;
    }
    // Delete the temporary TAR file
    if (remove(tar_path) != 0)
    {
        perror("Failed to delete temporary TAR file");
    }
    return 0;
}
void handleCopyDir(int socket1)
{
    CopyOperationData *opData = processCopyOperationInput();
    if (opData->destIP == inet_addr("127.0.0.1") && opData->destPort == NM_PORT)
    {
        // make local copy

        uint8_t response_tonm[2];
        response_tonm[0] = 0x88;
        response_tonm[1] = 0;

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            printf("[-]Failed to get current working directory\n");
            response_tonm[1] = -1;
            send(socket1, response_tonm, 2 * sizeof(uint8_t), 0);
            return;
        }

        // Form the tar file path in the current working directory
        char tarFilePath[1024];
        snprintf(tarFilePath, sizeof(tarFilePath), "%s/temporary_tar_file.tar.gz", cwd);
        char command[1024];

        // Change to the source directory
        if (chdir(opData->srcPath) != 0)
        {
            printf("[-]Failed to change directory to source path\n");
            response_tonm[1] = -1;
            send(socket1, response_tonm, 2 * sizeof(uint8_t), 0);
            return;
        }

        create_tar(".", tarFilePath);

        // Change back to the original directory
        if (chdir(cwd) != 0)
        {
            printf("Failed to change back to original directory\n");
            response_tonm[1] = -1;
            send(socket1, response_tonm, 2 * sizeof(uint8_t), 0);
            return;
        }

        // Extract the tar file to the destination path
        if (extract_tar_and_delete_killme(opData->destPath, tarFilePath) != 0)
        {
            response_tonm[1] = -1;
        }
        send(socket1, response_tonm, 2 * sizeof(uint8_t), 0);
        return;
    }
    int sock;
    struct sockaddr_in destAddr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return;
    }

    // Set up the destination address
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = (opData->destPort);
    destAddr.sin_addr.s_addr = (opData->destIP);

    // Connect to the destination server
    if (connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0)
    {
        perror("Connection to the server failed");
        close(sock);
        return;
    }

    // Read the source file
    const char *tar_file_name = "archive_name";

    create_tar(opData->srcPath, tar_file_name);

    FILE *file = fopen(tar_file_name, "rb");
    if (file == NULL)
    {
        perror("Failed to open source file");
        close(sock);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent = malloc(fileSize);
    if (fileContent == NULL)
    {
        perror("Memory allocation for file content failed");
        fclose(file);
        close(sock);
        return;
    }

    fread(fileContent, 1, fileSize, file);
    fclose(file);

    // Prepare the message

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    uint32_t destPathLength = strlen(opData->destPath);
    uint32_t destPathLengthNet = destPathLength;
    uint32_t fileSizeNet = fileSize;
    size_t messageSize = 1 + sizeof(destPathLengthNet) + destPathLength + 1 + sizeof(fileSizeNet) + fileSize + 1;
    uint8_t *message = malloc(messageSize);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    if (message == NULL)
    {
        perror("Memory allocation for message failed");
        free(fileContent);
        close(sock);
        return;
    }

    // add the file name to the destination path ?  /// not needed

    size_t offset = 0;
    message[offset++] = 0x1a; // Protocol code
    memcpy(message + offset, &destPathLengthNet, sizeof(destPathLengthNet));
    offset += sizeof(destPathLengthNet);
    memcpy(message + offset, opData->destPath, destPathLength);
    offset += destPathLength;
    message[offset++] = '\0'; // Null byte for path

    memcpy(message + offset, &fileSizeNet, sizeof(fileSizeNet));
    offset += sizeof(fileSizeNet);
    memcpy(message + offset, fileContent, fileSize);
    offset += fileSize;
    message[offset] = '\0'; // Null byte for content

    // Send the message

    // redirect message

    if (send(sock, message, messageSize, 0) < 0)
    {
        perror("Failed to send message");
    }
    else
    {
        printf("Message sent successfully\n");
    }

    uint8_t response[2];
    uint8_t status = 0;
    ssize_t received = recv(sock, &response, sizeof(response), 0);
    if (received < 0)
    {
        perror("Failed to receive response");
    }
    else if (received == 0)
    {
        printf("Server closed the connection\n");
    }
    else
    {
        // Check the protocol code and status
        uint8_t protocolCode = response[0];
        status = response[1];

        if (protocolCode == 0x9a)
        {
            if (status == 0)
            {
                printf("Directory transfer successful\n");
            }
            else
            {
                printf("Directory transfer failed with status code: %d\n", status);
            }
        }
        else
        {
            printf("Unexpected protocol code received: %d\n", protocolCode);
        }
    }
    uint8_t response_tonm[2];
    response_tonm[0] = 0x88;
    response_tonm[1] = status;
    send(socket1, response_tonm, sizeof(response_tonm), 0);
    free(message);
    free(fileContent);
    close(sock);

    free(opData);
}

void ping(int socket1)
{
    uint8_t response;
    response = 0x9b;
    send(socket1, &response, sizeof(response), 0);
}

void handleNamingServerRequest(int socket)
{
    ssize_t bytesRead = 0;
    uint8_t operationCode;
    // while (totalBytesRead < MAX_READ)
    bytesRead = read(socket, nm_buffer, MAX_READ);

    if (bytesRead > 0)
    {
        operationCode = nm_buffer[0];
    }

    switch (operationCode)
    {
    case 0x03:
        handleCreateFile(socket);
        break;
    case 0x04:
        handleCreateDir(socket);
        break;
    case 0x05:
        handleDeleteFile(socket);
        break;
    case 0x06:
        handleDeleteDir(socket);
        break;
    case 0x07:
        handleCopyFile(socket);
        break;
    case 0x08:
        handleCopyDir(socket);
        break;
    case 0x1b:
        ping(socket);
        break;
    default:
        printf("Incorrect Protocol Code for Naming Server Request to Storage Server\n");
        break;
    }
}
