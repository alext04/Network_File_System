#include "../inc/base.h"
#include "../inc/storageserver.h"
#include "../inc/namingserver.h"
#include "../inc/client.h"

uint8_t ss_buffer[MAX_READ];

int write_tar_to_disk(const char *tar_data, uint32_t tar_size)
{
    const char *tar_path = "temporary_tar_file.tar.gz"; // Temp TAR file in the current directory
    FILE *tar_file = fopen(tar_path, "wb");
    if (tar_file == NULL)
    {
        perror("Failed to open TAR file for writing");
        return -1;
    }

    size_t written = fwrite(tar_data, 1, tar_size, tar_file);
    if (written != tar_size)
    {
        perror("Failed to write the complete TAR file");
        fclose(tar_file);
        return -1;
    }
    return 0;
    fclose(tar_file);
}

int extract_tar_and_delete(const char *destination_path)
{
    char command[1024];
    const char *tar_path = "temporary_tar_file.tar.gz";
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

// 0x19
void PasteFile(int socket)
{
    size_t offset = 1;

    // Ensure ss_buffer size is adequate

    // Extract the length of the destination path
    uint32_t destPathLength;
    memcpy(&destPathLength, ss_buffer + offset, sizeof(destPathLength));
    destPathLength = destPathLength; // Convert to host byte order
    offset += sizeof(destPathLength);

    char *destPath = malloc(destPathLength + 1);
    if (destPath == NULL)
    {
        perror("Failed to allocate memory for destination path");
        return;
    }
    memcpy(destPath, ss_buffer + offset, destPathLength);
    destPath[destPathLength] = '\0';
    offset += destPathLength + 1; // Move past null byte

    // Extract the content length

    uint32_t contentLength;
    memcpy(&contentLength, ss_buffer + offset, sizeof(contentLength));
    contentLength = ntohl(contentLength); // Convert to host byte order
    offset += sizeof(contentLength);

    // Extract the content
    char *content = malloc(contentLength + 1);
    if (content == NULL)
    {
        perror("Failed to allocate memory for content");
        free(destPath);
        return;
    }
    memcpy(content, ss_buffer + offset, contentLength);
    content[contentLength] = '\0';

    createFirstDir(destPath);

    uint8_t response[2];
    response[0] = 0x99;
    response[1] = 0;
    size_t contentSize = sizeof(content) - 1;
    // Open or create the file
    FILE *file = fopen(destPath, "wb"); // 'wb' mode to write in binary format
    if (file == NULL)
    {
        response[1] = -1;
        perror("Invalid Path");
        return;
    }

    // Write the content to the file
    size_t written = fwrite(content, sizeof(char), contentSize, file);
    if (written < contentSize)
    {
        // wont happen as everthing fits in the 1 MB ss_buffer ?
        response[1] = -1;
        fprintf(stderr, "Failed to write the full content to the file\n");
    }
    else
    {
        printf("File content written successfully\n");
    }

    send(socket, response, sizeof(response), 0);

    // Close the file
    fclose(file);
}

// 0x1a
void PasteDir(int socket)
{
    size_t offset = 1;

    // Ensure ss_buffer size is adequate

    // Extract the length of the destination path
    uint32_t destPathLength;
    memcpy(&destPathLength, ss_buffer + offset, sizeof(destPathLength));
    destPathLength = ntohl(destPathLength); // Convert to host byte order
    offset += sizeof(destPathLength);

    char *destPath = malloc(destPathLength + 1);
    if (destPath == NULL)
    {
        perror("Failed to allocate memory for destination path");
        return;
    }
    memcpy(destPath, ss_buffer + offset, destPathLength);
    destPath[destPathLength] = '\0';
    offset += destPathLength + 1; // Move past null byte

    // Extract the content length

    uint32_t contentLength;
    memcpy(&contentLength, ss_buffer + offset, sizeof(contentLength));
    contentLength = ntohl(contentLength); // Convert to host byte order
    offset += sizeof(contentLength);

    // Extract the content
    char *content = malloc(contentLength + 1);
    if (content == NULL)
    {
        perror("Failed to allocate memory for content");
        free(destPath);
        return;
    }
    memcpy(content, ss_buffer + offset, contentLength);
    content[contentLength] = '\0';

    const char *destination_path = destPath;

    // Write TAR file to the temporary path
    int fail_1 = write_tar_to_disk(content, contentLength);

    // Extract TAR file to create the directory and delete the temp file
    int fail_2 = extract_tar_and_delete(destination_path);

    uint8_t response[2];
    response[0] = 0x9a;

    // kinda retarded but works ?

    if (fail_1 == -1 || fail_2 == -1)
    {
        response[1] = -1;
    }
    else
    {
        response[1] = 0;
    }

    send(socket, response, sizeof(response), 0);
}

void handleStorageServerRequest(int socket)
{
    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    uint8_t operationCode;

    while (totalBytesRead < MAX_READ)
    {
        bytesRead = read(socket, ss_buffer + totalBytesRead, MAX_READ - totalBytesRead);
        if (bytesRead <= 0)
        {
            // If read returns 0, the client has closed the connection
            // If read returns -1, an error occurred
            if (bytesRead == 0)
            {
                printf("Client disconnected\n");
            }
            else
            {
                perror("Read failed");
            }
            break;
        }
        totalBytesRead += bytesRead;
    }

    if (totalBytesRead > 0)
    {
        operationCode = ss_buffer[0];
    }

    switch (operationCode)
    {
    case 0x19:
        PasteFile(socket);
        break;
    case 0x1a:
        PasteDir(socket);
        break;
    default:
        printf("Incorrect Protocol Code for Naming Server Request to Storage Server\n");
        break;
    }
}
