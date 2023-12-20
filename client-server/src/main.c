#include "../inc/base.h"
#include "../inc/client.h"

#include <arpa/inet.h>

#define PORT 8087

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    const char *server_ip = "127.0.0.1";

    unsigned char buffer[MAX_BUFFER_SIZE];
    // printf("\n\n++++++++++++++\n\n");
    connectToServer(sock, inet_addr(server_ip));
    int client_id = hash(argv[1]);
    printf("Starting Client, Username : %s, %d \n", argv[1], client_id);

    int size = 1 + 1 + 4 + 1;
    unsigned char *sendingdata = (unsigned char *)malloc(size);

    bzero(sendingdata, size);
    sendingdata[0] = 0x01;
    sendingdata[1] = 0;
    *((int *)(sendingdata + 2)) = client_id;

    send(sock, sendingdata, size, 0);

    recv(sock, buffer, MAX_BUFFER_SIZE, 0);

    if (buffer[0] == 0x81)
    {
        if (buffer[1] == 0)
        {
            printf("Initialize connection successful \n");
        }
        else
        {
            // enter error
            printf("Initialize connection unsuccessful \n");
        }
    }
    else
    {

        printf("Initialize buffer code unsuccessful \n");
    }

    memset(buffer, 0, sizeof(buffer));

    while (1)
    {
        printf("NFS>> ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        fflush(stdin);
        // scanf("%s\n",buffer);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character from input

        if (strcmp(buffer, "quit") == 0)
        {
            break; // Exit the loop and end the client
        }

        sendData(sock, buffer);
    }

    close(sock);
}