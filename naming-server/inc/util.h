
#define _OPEN_SYS_ITOA_EXT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

char * strsub (char *input, char *substring, char *replace);
char *strstrip(char *s);

char * strapp(char * old, char * new);
uint32_t hash(char* str);
