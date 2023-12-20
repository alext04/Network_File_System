#include "base.h"



#define OP_READ_FILE       0x0b
#define OP_WRITE_FILE      0x0c
#define OP_GET_FILE_INFO   0x0d



// #define CLIENT_PORT 54321 // Port for Client Connection
#define MAX_CLIENTS 10 // Maximum number of clients
extern int CLIENT_PORT; 
// char* parsePathInput(int socket);


// // 0x0B
// void handleReadFile(int socket);
// // 0x0C
// void handleWriteFile(int socket);

// // 0x0D
// void handleGetFileInfo(int socket);

void handleClientRequest(int socket) ;