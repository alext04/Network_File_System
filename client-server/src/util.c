#include "../inc/util.h"
// Credit : https://stackoverflow.com/a/56045299
/*
 * strsub : substring and replace substring in strings.
 *
 * Function to replace a substring with a replacement string. Returns a
 * buffer of the correct size containing the input string with all instances
 * of the substring replaced by the replacement string.
 *
 * If the substring is empty the replace string is written before each character
 * and at the end of the string.
 *
 * Returns NULL on error after setting the error number.
 *
 */

char * strsub (char *input, char *substring, char *replace)
{
    int     number_of_matches = 0;
    size_t  substring_size = strlen(substring), replace_size = strlen(replace), buffer_size;
    char    *buffer, *bp, *ip;

/*
 * Count the number of non overlapping substring occurences in the input string. This
 * information is used to calculate the correct buffer size.
 */
    if (substring_size)
    {
        ip = strstr(input, substring);
        while (ip != NULL)
        {
            number_of_matches++;
            ip = strstr(ip+substring_size, substring);
        }
    }
    else
        number_of_matches = strlen (input) + 1;

/*
 * Allocate a buffer of the correct size for the output.
 */
    buffer_size = strlen(input) + number_of_matches*(replace_size - substring_size) + 1;

    if ((buffer = ((char *) malloc(buffer_size))) == NULL)
    {
        // errno=ENOMEM;
        return NULL;
    }

/*
 * Rescan the string replacing each occurence of a match with the replacement string.
 * Take care to copy buffer content between matches or in the case of an empty find
 * string one character.
 */
    bp = buffer;
    ip = strstr(input, substring);
    while ((ip != NULL) && (*input != '\0'))
    {
        if (ip == input)
        {
            memcpy (bp, replace, replace_size+1);
            bp += replace_size;
            if (substring_size)
                input += substring_size;
            else
                *(bp++) = *(input++);
            ip = strstr(input, substring);
        }
        else 
            while (input != ip)
                *(bp++) = *(input++);

    }

/*
 * Write any remaining suffix to the buffer, or in the case of an empty find string
 * append the replacement pattern.
 */
    if (substring_size)
        strcpy (bp, input);
    else
        memcpy (bp, replace, replace_size+1);

    return buffer;
}

// From Linux Kernel
// Removes leading and trailing whitespace from the string.
char *strstrip(char *s)
{
        size_t size;
        char *end;

        size = strlen(s);

        if (!size)
                return s;

        end = s + size - 1;
        while (end >= s && isspace(*end))
                end--;
        *(end + 1) = '\0';

        while (*s && isspace(*s))
                s++;

        return s;
}

// CREDIT : https://stackoverflow.com/q/2468421
/**********************************************************************
 * dynamically allocate and append new string to old string and return a pointer to it
 **********************************************************************/
 char * strapp(char * old, char * new)
 {
     // find the size of the string to allocate
     int len = sizeof(char) * (strlen(old) + strlen(new));

     // allocate a pointer to the new string
     char * out = (char*)malloc(len+1);

     // concat both strings and return
     sprintf(out, "%s%s", old, new);

     return out;
 }


// Hashing for strings: MurmurOAAT_32
// Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
uint32_t hash(char* str)
{
    // One-byte-at-a-time hash based on Murmur's mix
    int h = 0x12345678;
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}