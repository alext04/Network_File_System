#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024


// int isPathValid(const char *path)
// {
//     // struct stat buffer;
//     // printf("1\n");
//     // // if (stat(path, &buffer) != 0)
//     // // {
//     // //     return 0; // Path does not exist
//     // // }
//     // printf("2\n");
//     // char realPath[BUFFER_SIZE];
//     // char currentDir[BUFFER_SIZE];
//     // char currentDirWithSep[BUFFER_SIZE + 2];
//     // printf("3\n");
//     // // Get the absolute path of the given path
//     // realpath(path, realPath);
//     // printf("%s\n",path);    
//     // printf("4\n");
//     // if (getcwd(currentDir, sizeof(currentDir)) == NULL)
//     // {
//     //     return 0; // Error in getting the absolute path of the current directory
//     // }
//     // printf("5\n");

//     // // printf("+++++++++  %s %s \n",realPath,currentDir);
//     // snprintf(currentDirWithSep, sizeof(currentDirWithSep), "%s/", currentDir);

//     // // Check if the given path starts with the current directory
//     // if (strncmp(realPath, currentDirWithSep, strlen(currentDirWithSep)) == 0)
//     // {
//     //     return 1;
//     // }

//     // return 0;
// }


int main(){

    // printf("%d\n",isPathValid("abc"));
    FILE *file = fopen("abc", "w");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return 1;
    }
    else{
        return 0;
    }
    return 0;

}