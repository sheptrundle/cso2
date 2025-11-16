// Shep Trundle HW1
// dvf5rd
// CSO 2

#include <stdio.h>
#include <stdlib.h>
#include "split.h"
#include <string.h>


int main(int argc, char *argv[]) {
    char *sep;

    // Default separator 
    if (argc == 1){
        sep = " \t";
    } else {
        // Get number of chars
        int total_len = 0;
        for (int i = 1; i < argc; i++) {
            total_len += strlen(argv[i]);
        }

        // Allocate space and save sep value
        sep = malloc(total_len + 1);
        sep[0] = '\0';
        for (int i = 1; i < argc; i++) {
            strcat(sep, argv[i]);
        }
    } 

    char line[1000];

    // Begin reading lines 
    while (fgets(line, sizeof(line), stdin)) {

        // Get rid of new line char
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Exit in case of "."
        if (strcmp(line, ".") == 0) {
            break;
        }

        // Line is good, call string_split
        int word_count = 0;
        char **words = string_split(line, sep, &word_count);

        // Print words in correct format
        for (int i = 0; i < word_count; i++) {
            printf("[%s]", words[i]);
            free(words[i]);
        }
        // Add new line when done
        printf("\n");

        free(words);
    }

    // Hopefully this fixes memory leak autograder not checking?
    if (argc > 1) {
        free(sep);
    }
    
    return 0;
}