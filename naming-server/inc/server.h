#include "base.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>//for FD macros

#include "client.h"
#include "hashmap.h"

#define SERVER_ADDR "127.0.0.1"
#define PORT 4444
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define REDUNDANCY_CUTOFF 3

typedef struct ServerData{
    int sockfd;//temporary variables to store socket descriptor
	struct sockaddr_in serverAddr;	
    ClientData* clientData[MAX_CLIENTS];
    socklen_t addr_size;
    HashTable* file_ht;
    HashTable* client_ht;
    ClientData* ss_List[MAX_CLIENTS];
    int numServers;
}ServerData;


ServerData* CreateServer();
void RunServer(ServerData* sData);
void CloseServer(ServerData* sData);

void HandleMessage(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);

void MH_InitConn(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_ListDir(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_GetFileLoc(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_CreateFile(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_CreateDir(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_DeleteFile(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_DeleteDir(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_CopyFile(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);
void MH_CopyDir(ServerData* sData, ClientData* cData, uint8_t * buffer, int buflen);



