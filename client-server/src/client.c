#include "../inc/base.h"
#include "../inc/client.h"

#define PORT 8080

#define MAX_BUFFER_SIZE 1024

char **split(char *token, char **argument)
{
    int bufsize = 64;

    const char *del = " ";

    char *command;

    int position = 0;

    command = strtok(token, del);
    int flag = 0;
    while (command != NULL && position < 3)
    {
        argument[position] = command;
        position++;
        if (flag == 0)
        {
            command = strtok(NULL, del);
            flag = 1;
        }
        else
        {
            command = strtok(NULL, "");
        }
    }

    argument[position] = NULL;

    return argument;
}

void connectToServer(int sock, int ip_address)
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = NM_INIT_PORT;
    serv_addr.sin_addr.s_addr = ip_address;
    // // Convert IPv4 and IPv6 addresses from text to binary form
    // if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0)
    // {
    //     perror("Invalid address/ Address not supported");
    //     exit(EXIT_FAILURE);
    // }

    // // Bind the socket to the Client port
    // if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    // {
    //     perror("[-]Binding to Client Port failed");
    //     exit(1);
    // }
    // printf("[+] Coshed\n\n");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    printf("[+] Connection Established to Naming Server");
}

void connectToSTORAGEServer(int sock, int ip_address, int port)
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = port;
    serv_addr.sin_addr.s_addr = ip_address;

    // Convert IPv4 and IPv6 addresses from text to binary form
    // if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0)
    // {
    //     perror("Invalid address/ Address not supported");
    //     exit(EXIT_FAILURE);
    // }

    // if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    // {
    //     perror("[-]Binding to Client Port failed");
    //     exit(1);
    // }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    printf("[+] Connection Established to Storage Server");
}

char *nts(int num, char *string)
{
    int copy = num;

    int count = 0;

    while (copy > 0)
    {
        copy = copy / 10;
        count++;
    }

    string = (char *)malloc((count + 2) * sizeof(char));

    int copy1 = num;

    int index = 0;

    while (copy1 > 0)
    {
        int prev = copy1;
        copy1 = copy1 / 10;
        int digit = prev - (copy1 * 10);

        string[count - index - 1] = 48 + digit;
        index++;
    }

    string[index] = '/';

    string[index + 1] = '\0';

    return string;
}

char *joiner(char *newpath, char *path, char *cid)
{
    newpath = (char *)malloc((strlen(path) + strlen(cid) + 1) * sizeof(char));

    strcpy(newpath, cid);

    strcat(newpath, path);

    return newpath;
}

void getfileloc(int pathlength, char *path, int sock, uint32_t *clientID, int *DestSS_IP, uint32_t *DestSS_PORT)
{
    int size = 1 + 4 + pathlength + 1;
    unsigned char *sendingdata;
    sendingdata = (unsigned char *)malloc(size);

    sendingdata[0] = 0x09;
    *(uint32_t *)&sendingdata[1] = pathlength;
    snprintf((char *)&sendingdata[1 + 4], pathlength + 1, "%s", path);
    sendingdata[size - 1] = '\0';
    for (int i = 0; i < size; i++)
    {
        printf("%d\t", sendingdata[i]);
    }

    printf("\n__> %d %s <__ \n", pathlength, path);

    send(sock, sendingdata, size, 0);

    unsigned char buffer[MAX_BUFFER_SIZE];

    recv(sock, buffer, MAX_BUFFER_SIZE, 0);

    if (buffer[0] == 0x89)
    {
        if (buffer[1] == 0)
        {
            printf("Get File Info successful \n");

            *clientID = *((uint32_t *)&buffer[2]);

            *DestSS_IP = *((uint32_t *)&buffer[6]);

            *DestSS_PORT = *((uint32_t *)&buffer[10]);
        }
        else
        {
            // enter error
            printf("Error for Get File Info from SS \n");
        }
    }
    else
    {
        printf("Response for Get File Info unsuccessful \n");
    }
}

// Function to send data and format it acorrding to protocols
void sendData(int sock, const char *data)
{
    char *copy = (char *)malloc(strlen(data) + 1);

    strcpy(copy, data);

    char **argument = malloc(64 * sizeof(char *));

    for (int i = 0; i < 64; i++)
    {
        argument[i] = NULL;
    }

    argument = split(copy, argument);

    int size = 0;
    unsigned char *sendingdata;

    if (argument[0] == NULL)
    {
        return;
    }

    if (strcmp(argument[0], "readfile") == 0)
    {
        /// readfile INPUT FORMAT = readfile path

        if (argument[1] == NULL)
        {
            printf("Enter all requirements for readfile \n");
            return;
        }

        int pathlength = strlen(argument[1]);

        char *path = (char *)malloc((pathlength + 1) * (sizeof(char)));
        strcpy(path, argument[1]);
        path[pathlength] = '\0';

        uint32_t clientID;
        int DestSS_IP;
        uint32_t DestSS_PORT;

        getfileloc(pathlength, path, sock, &clientID, &DestSS_IP, &DestSS_PORT);
        // printf("Get file Loc: %d %d %d \n", clientID, DestSS_IP, DestSS_PORT);
        // small error in getfileloc

        // clientID=527857071;
        // DestSS_IP="127.0.0.1";
        // DestSS_PORT=5432;
        char *cid;
        cid = nts(clientID, cid);

        char *newpath;
        newpath = joiner(newpath, path, cid);

        int sock_storageserver;
        if ((sock_storageserver = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        connectToSTORAGEServer(sock_storageserver, DestSS_IP, DestSS_PORT);

        size = 1 + 4 + strlen(newpath) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x0b;
        *(uint32_t *)&sendingdata[1] = strlen(newpath);
        snprintf((char *)&sendingdata[1 + 4], strlen(newpath) + 1, "%s", newpath);
        sendingdata[size - 1] = 0;

        // printf("__> %s <__ \n", sendingdata);

        send(sock_storageserver, sendingdata, size, 0);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock_storageserver, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x8b)
        {
            if (buffer[1] == 0)
            {
                printf("Read File successful \n");
                uint32_t value = *((uint32_t *)&buffer[2]);
                char *content = malloc(value + 1); // +1 for null-terminator
                if (content == NULL)
                {
                    perror("Failed to allocate memory for content");
                }
                memcpy(content, buffer + 6, value);
                content[value] = '\0'; // Null-terminate the string
                printf("\n%s\n\n", content);
            }
            else
            {
                // enter error
                printf("Error for Read File from SS \n");
            }
        }
        else
        {
            printf("Response for Read File unsuccessful from SS\n");
        }

        close(sock_storageserver);
    }

    else if (strcmp(argument[0], "writefile") == 0)
    {
        /// writefile INPUT FORMAT = writefile path content
        if (argument[1] == NULL || argument[2] == NULL)
        {
            printf("Enter all requirements for writefile \n");
            return;
        }

        int pathlength = strlen(argument[1]);

        char *path = (char *)malloc((pathlength + 1) * (sizeof(char)));
        strcpy(path, argument[1]);
        path[pathlength] = '\0';

        int contentlength = strlen(argument[2]);

        char *content = (char *)malloc((contentlength + 1) * (sizeof(char)));
        strcpy(content, argument[2]);
        content[contentlength] = '\0';

        uint32_t clientID;
        int DestSS_IP;
        uint32_t DestSS_PORT;

        getfileloc(pathlength, path, sock, &clientID, &DestSS_IP, &DestSS_PORT);

        char *cid;
        cid = nts(clientID, cid);

        char *newpath;
        newpath = joiner(newpath, path, cid);

        int sock_storageserver;
        if ((sock_storageserver = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        connectToSTORAGEServer(sock_storageserver, DestSS_IP, DestSS_PORT);

        size = 1 + 4 + strlen(newpath) + 1 + 4 + strlen(content) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x0c;
        *(uint32_t *)&sendingdata[1] = strlen(newpath);
        snprintf((char *)&sendingdata[1 + 4], strlen(newpath) + 1, "%s", newpath);
        sendingdata[4 + strlen(newpath) + 1] = 0;

        *(uint32_t *)&sendingdata[4 + strlen(newpath) + 1 + 1] = strlen(content);
        snprintf((char *)&sendingdata[4 + strlen(newpath) + 1 + 1 + 4], strlen(content) + 1, "%s", content);
        sendingdata[size - 1] = 0;

        // printf("__> %s <__ \n", sendingdata);
        send(sock_storageserver, sendingdata, size, 0);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock_storageserver, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x8c)
        {
            if (buffer[1] == 0)
            {
                printf("Write File successful \n");
            }
            else
            {
                // enter error
                printf("Error for Write File from SS \n");
            }
        }
        else
        {
            printf("Response for Write File unsuccessful from SS\n");
        }

        close(sock_storageserver);
    }

    else if (strcmp(argument[0], "fileinfo") == 0)
    {
        // size = 1 + 4 + atoi(argument[1]) + 1;
        // sendingdata = (unsigned char *)malloc(size);

        // sendingdata[0] = 0x0d;
        // *(uint32_t *)&sendingdata[1] = atoi(argument[1]);
        // snprintf((char *)&sendingdata[1 + 4], atoi(argument[1]), "%s", argument[2]);
        // sendingdata[size - 1] = '\0';

        /// fileinfo INPUT FORMAT = fileinfo path

        if (argument[1] == NULL)
        {
            printf("Enter all requirements for fileinfo \n");
            return;
        }

        int pathlength = strlen(argument[1]);

        char *path = (char *)malloc((pathlength + 1) * (sizeof(char)));
        strcpy(path, argument[1]);
        path[pathlength] = '\0';

        uint32_t clientID;
        int DestSS_IP;
        uint32_t DestSS_PORT;

        getfileloc(pathlength, path, sock, &clientID, &DestSS_IP, &DestSS_PORT);

        char *cid;
        cid = nts(clientID, cid);

        char *newpath;
        newpath = joiner(newpath, path, cid);

        int sock_storageserver;
        if ((sock_storageserver = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        connectToSTORAGEServer(sock_storageserver, DestSS_IP, DestSS_PORT);

        size = 1 + 4 + strlen(newpath) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x0d;
        *(uint32_t *)&sendingdata[1] = strlen(newpath);
        snprintf((char *)&sendingdata[1 + 4], strlen(newpath) + 1, "%s", newpath);
        sendingdata[size - 1] = 0;

        send(sock_storageserver, sendingdata, size, 0);
        // printf("__> %s <__ \n", sendingdata);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock_storageserver, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x8d)
        {
            if (buffer[1] == 0)
            {
                printf("Get File Info successful \n");
                uint32_t value = *((uint32_t *)&buffer[2]);
                printf("\nFile Size : %d\n\n", value);
            }
            else
            {
                // enter error
                printf("Error for Get File Info from SS \n");
            }
        }
        else
        {
            printf("Response for Get File Info unsuccessful from SS\n");
        }

        close(sock_storageserver);
    }
    else if (strcmp(argument[0], "createfile") == 0)
    {
        // createfile format = createfile path
        if (argument[1] == NULL)
        {
            printf("Enter all requirements for createfile \n");
            return;
        }

        // argument[1] = "5";
        // argument[2] = "hello";

        // for (int i = 0; i < 2; i++)
        // {
        //     printf("%s \n", argument[i]);
        // }

        size = 1 + 4 + strlen(argument[1]) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x13;
        *(uint32_t *)&sendingdata[1] = strlen(argument[1]);
        snprintf((char *)&sendingdata[1 + 4], strlen(argument[1]) + 1, "%s", argument[1]);
        sendingdata[size - 1] = 0;

        send(sock, sendingdata, size, 0);
        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock, buffer, MAX_BUFFER_SIZE, 0);
        if (buffer[0] == 0x93)
        {
            if (buffer[1] == 0)
            {
                printf("Create File successful \n");
            }
            else
            {
                // enter error
                printf("Error for Create File from SS \n");
            }
        }
        else
        {
            printf("Response for Create File unsuccessful from SS\n");
        }
    }
    else if (strcmp(argument[0], "createdir") == 0)
    {
        // createdir format = createdir path
        if (argument[1] == NULL)
        {
            printf("Enter all requirements for createdir \n");
            return;
        }
        // argument[1] = "5";
        // argument[2] = "hello";

        size = 1 + 4 + strlen(argument[1]) + 1;
        sendingdata = (unsigned char *)malloc(size);
        bzero(sendingdata, size);
        sendingdata[0] = 0x14;
        *(uint32_t *)&sendingdata[1] = strlen(argument[1]);
        snprintf((char *)&sendingdata[1 + 4], strlen(argument[1]) + 1, "%s", argument[1]);
        sendingdata[size - 1] = 0;

        send(sock, sendingdata, size, 0);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x94)
        {
            if (buffer[1] == 0)
            {
                printf("Create Dir successful \n");
            }
            else
            {
                // enter error
                printf("Error for Create Dir from SS \n");
            }
        }
        else
        {
            printf("Response for Create Dir unsuccessful from SS\n");
        }
    }
    else if (strcmp(argument[0], "deletefile") == 0)
    {
        // deletefile format = deletefile path
        if (argument[1] == NULL)
        {
            printf("Enter all requirements for deletefile \n");
            return;
        }

        size = 1 + 4 + strlen(argument[1]) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x15;
        *(uint32_t *)&sendingdata[1] = strlen(argument[1]);
        snprintf((char *)&sendingdata[1 + 4], strlen(argument[1]) + 1, "%s", argument[1]);
        sendingdata[size - 1] = 0;

        send(sock, sendingdata, size, 0);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x95)
        {
            if (buffer[1] == 0)
            {
                printf("Delete File successful \n");
            }
            else
            {
                // enter error
                printf("Error for Delete File from SS \n");
            }
        }
        else
        {
            printf("Response for Delete File unsuccessful from SS\n");
        }
    }
    else if (strcmp(argument[0], "deletedir") == 0)
    {
        // deletedir format = deletedir path
        if (argument[1] == NULL)
        {
            printf("Enter all requirements for deletedir \n");
            return;
        }

        size = 1 + 4 + strlen(argument[1]) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x16;
        *(uint32_t *)&sendingdata[1] = strlen(argument[1]);
        snprintf((char *)&sendingdata[1 + 4], strlen(argument[1]) + 1, "%s", argument[1]);
        sendingdata[size - 1] = 0;

        send(sock, sendingdata, size, 0);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x96)
        {
            if (buffer[1] == 0)
            {
                printf("Delete dir successful \n");
            }
            else
            {
                // enter error
                printf("Error for Delete Dir from SS \n");
            }
        }
        else
        {
            printf("Response for Delete Dir unsuccessful from SS\n");
        }
    }
    else if (strcmp(argument[0], "copyfile") == 0)
    {
        // copyfile format = copyfile path
        if (argument[1] == NULL)
        {
            printf("Enter all requirements for copyfile \n");
            return;
        }

        size = 1 + 4 + strlen(argument[1]) + 1 + 4 + strlen(argument[2]) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x17;
        *(uint32_t *)&sendingdata[1] = strlen(argument[1]);
        snprintf((char *)&sendingdata[1 + 4], strlen(argument[1]) + 1, "%s", argument[1]);
        sendingdata[4 + strlen(argument[1]) + 1] = 0;

        *(uint32_t *)&sendingdata[4 + strlen(argument[1]) + 1 + 1] = strlen(argument[2]);
        snprintf((char *)&sendingdata[4 + strlen(argument[1]) + 1 + 1 + 4], strlen(argument[2]) + 1, "%s", argument[2]);
        sendingdata[size - 1] = 0;

        // for (int i = 0; i < size; i++)
        // {
        //     printf("%d\t", sendingdata[i]);
        // }
        // printf("\n");
        // printf("__> %s <__ \n", sendingdata);

        send(sock, sendingdata, size, 0);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x97)
        {
            if (buffer[1] == 0)
            {
                printf("Copy File successful \n");
            }
            else
            {
                // enter error
                printf("Error for Copy File from SS \n");
            }
        }
        else
        {
            printf("Response for Copy File unsuccessful from SS\n");
        }
    }
    else if (strcmp(argument[0], "copydir") == 0)
    {
        // copydir format = copydir path
        if (argument[1] == NULL)
        {
            printf("Enter all requirements for copydir \n");
            return;
        }

        size = 1 + 4 + strlen(argument[1]) + 1 + 4 + strlen(argument[2]) + 1;
        sendingdata = (unsigned char *)malloc(size);

        sendingdata[0] = 0x18;
        *(uint32_t *)&sendingdata[1] = strlen(argument[1]);
        snprintf((char *)&sendingdata[1 + 4], strlen(argument[1]) + 1, "%s", argument[1]);
        sendingdata[4 + strlen(argument[1]) + 1] = 0;

        *(uint32_t *)&sendingdata[4 + strlen(argument[1]) + 1 + 1] = strlen(argument[2]);
        snprintf((char *)&sendingdata[4 + strlen(argument[1]) + 1 + 1 + 4], strlen(argument[2]) + 1, "%s", argument[2]);
        sendingdata[size - 1] = 0;

        // printf("__> %s <__ \n", sendingdata);

        send(sock, sendingdata, size, 0);

        unsigned char buffer[MAX_BUFFER_SIZE];

        recv(sock, buffer, MAX_BUFFER_SIZE, 0);

        if (buffer[0] == 0x98)
        {
            if (buffer[1] == 0)
            {
                printf("Copy Dir successful \n");
            }
            else
            {
                // enter error
                printf("Error for Copy Dir from SS \n");
            }
        }
        else
        {
            printf("Response for Copy Dir unsuccessful from SS\n");
        }
    }
    else
    {
        printf("invalid command\n");
    }

    return;
}
