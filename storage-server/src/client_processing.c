#include "../inc/base.h"
#include "../inc/storageserver.h"
#include "../inc/namingserver.h"
#include "../inc/client.h"

uint8_t cl_buffer[MAX_READ];
char *parsePathInput(int socket)
{

    // Extract path length (4 bytes after the operation code)
    uint32_t pathLength;
    memcpy(&pathLength, cl_buffer + 1, sizeof(pathLength));

    // Convert path length to host byte order
    pathLength = (pathLength);

    // Extract the path
    char *path = malloc(pathLength + 1); // +1 for null-terminator
    if (path == NULL)
    {
        perror("Failed to allocate memory for path");
        return NULL;
    }
    memcpy(path, cl_buffer + 5, pathLength);
    path[pathLength] = '\0'; // Null-terminate the string

    // Check for null byte in buffer
    if (cl_buffer[5 + pathLength] != '\0')
    {
        fprintf(stderr, "Path data does not end with a null byte\n");
        free(path);
        return NULL;
    }

    return path;
}

// 0x0B
void handleReadFile(int socket)
{
    char *path = parsePathInput(socket);

    // Open the file
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening file");
        uint8_t errorResponse[] = {1}; // Error status
        send(socket, errorResponse, sizeof(errorResponse), 0);
        free(path);
        return;
    }

    // Get file size
    struct stat statBuf;
    if (fstat(fd, &statBuf) < 0)
    {
        perror("Error getting file size");
        uint8_t errorResponse[] = {1}; // Error status
        send(socket, errorResponse, sizeof(errorResponse), 0);
        close(fd);
        free(path);
        return;
    }

    // Allocate buffer
    uint32_t fileSize = statBuf.st_size;
    size_t totalSize = 1 + 1 + 4 + fileSize + 1; // Status + Content Length + Content + Null Byte
    uint8_t *buffer = malloc(totalSize);

    // Fill the buffer
    buffer[0] = 0x8b;
    buffer[1] = 0;                          // OK status
    *((uint32_t *)(buffer + 2)) = fileSize; // Content length in network byte order
    read(fd, buffer + 6, fileSize);         // File content
    buffer[totalSize - 1] = 0;              // Null byte

    // Send the response
    send(socket, buffer, totalSize, 0);
    // Clean up
    close(fd);
    free(buffer);
    free(path);
}

// 0x0C
void handleWriteFile(int socket)
{
    // input parsing

    // move reading to another function for readability ?

    // Extract source path length (4 bytes after the operation code)
    uint8_t response[2];
    response[0] = 0x8c;
    response[1] = 0;
    uint32_t srcPathLength;
    memcpy(&srcPathLength, cl_buffer + 1, sizeof(srcPathLength));
    // srcPathLength = (srcPathLength); // Convert to host byte order

    // Extract the source path
    char *srcPath = malloc(srcPathLength + 1);

    memcpy(srcPath, cl_buffer + 5, srcPathLength);
    srcPath[srcPathLength] = '\0';

    // Extract content length
    uint32_t contentLength;
    memcpy(&contentLength, cl_buffer + srcPathLength + 6, sizeof(contentLength));
    // contentLength = contentLength; // Convert to host byte order

    // Extract the content
    char *content = malloc(contentLength + 1);

    memcpy(content, cl_buffer + srcPathLength + 6 + 4, contentLength);
    content[contentLength] = '\0';

    // Assuming it is rewritten
    // Open the file for writing
    FILE *file = fopen(srcPath, "w");
    if (file == NULL)
    {
        perror("Failed to open file for writing");
        response[1] = -1;
        send(socket, response, sizeof(response), 0);
        free(srcPath);
        free(content);
        return;
    }

    // Write the content to the file
    size_t written = fwrite(content, sizeof(char), contentLength, file);
    if (written < contentLength)
    {
        response[1] = -1;
        fprintf(stderr, "Failed to write the full content to the file\n");
    }
    else
    {
        printf("Content successfully written to %s\n", srcPath);
    }

    send(socket, response, sizeof(response), 0);

    // Close the file
    fclose(file);

    // Free the allocated memory
    free(srcPath);
    free(content);
}

// 0x0D
void handleGetFileInfo(int socket)
{
    char *path = parsePathInput(socket);
    // Implement the logic to get file info using 'path'

    struct stat fileInfo;
    uint8_t status;
    uint32_t fileSize;

    uint8_t response[6];
    response[0] = 0x8d;

    if (stat(path, &fileInfo) == 0)
    {
        // File found

        response[1] = 0;
        fileSize = (uint32_t)fileInfo.st_size; // Assuming the file size fits into uint32_t
    }
    else
    {
        // Error in getting file info, set status to error
        response[1] = -1;
        fileSize = 0;
    }

    // Prepare the response
    uint32_t fileSizeNet = fileSize; // Convert file size to network byte order
    memcpy(response + 2, &fileSizeNet, sizeof(fileSizeNet));
    send(socket, response, sizeof(response), 0);
    free(path);
}

void handleClientRequest(int socket)
{
    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    uint8_t operationCode;

    bytesRead = read(socket, cl_buffer, MAX_READ);

    operationCode = cl_buffer[0];

    switch (operationCode)
    {
    case OP_READ_FILE:
        handleReadFile(socket);
        break;
    case OP_WRITE_FILE:
        handleWriteFile(socket);
        break;
    case OP_GET_FILE_INFO:
        handleGetFileInfo(socket);
        break;

    default:
        printf("Unknown operation code.\n");
        break;
    }
}
