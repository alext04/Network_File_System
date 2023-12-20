#include "base.h"


#define OP_CREATE_FILE 0x03
#define OP_CREATE_DIR  0x04
#define OP_DELETE_FILE 0x05
#define OP_DELETE_DIR  0x06
#define OP_COPY_FILE   0x07
#define OP_COPY_DIR    0x08
#define MAX_READ 1048576 


// #define NM_PORT 5555 // Port for listening to Naming Server
#define NM_INITIALISING_PORT 4444 // Port which Naming Server listens to

extern int NM_PORT;
typedef struct {
    char* srcPath;
    char* destPath;
    uint32_t destIP;
    uint32_t destPort;
} CopyOperationData;




// char* readPathFromSocket(int socket) ;

// CopyOperationData* processCopyOperationInput(int socket) ;


// 0x03
// void handleCreateFile(int socket);
 
// // 0x04
// void handleCreateDir(int socket);


// // 0x05
// void handleDeleteFile(int socket);

// // 0x06
// void handleDeleteDir(int socket);

// // 0x07
// void handleCopyFile(int socket);
// // 0x08
// void handleCopyDir(int socket);

int isPathValid(const char *path);
int createFirstDir(const char *path);
void handleNamingServerRequest(int socket);