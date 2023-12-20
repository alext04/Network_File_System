#include <stdio.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

void processInput(char *input);

// int main() {
//     char buffer[MAX_BUFFER_SIZE];

//     // Take input from the user
//     printf("Enter a sentence: ");
//     fgets(buffer, sizeof(buffer), stdin);

//     // Remove newline character from the input
//     buffer[strcspn(buffer, "\n")] = '\0';

//     // Process the input and check for the first word
//     processInput(buffer);

//     return 0;

// }

void processInput(char *input) {
    char *token = strtok(input, " ");

    // Check if there is at least one word in the input
    if (token != NULL) {
        // Check if the first word is "buffer"
        if (strcmp(token, "buffer") == 0) {
            printf("The first word is 'buffer'. Returning the sentence: hahahaha\n");
        } else {
            printf("The first word is not 'buffer'.\n");
        }
    } else {
        printf("No words found in the input.\n");
    }
}
