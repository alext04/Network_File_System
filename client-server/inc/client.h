#include "base.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>//for FD macros

enum ClientType {USER, STORAGE};

typedef struct ClientData
{
    uint32_t id;
    int sockfd;
    enum ClientType cType;  
    struct sockaddr_in addr;  
}ClientData;

void connectToServer(int sock, int ip_address);
void sendData(int sock, const char *data) ;
void receiveResponse(int sock, char *buffer);