// Shep Trundle HW1
// dvf5rd
// CSO 2

#include <stdlib.h>
#include <string.h>
#include "split.h"


char **string_split(const char *input, const char *sep, int *num_words) {
    char **words_arr = malloc(20 * sizeof(char*));
    *num_words = 0;

    const char *cur_char = input;
    const char *word_start = input;
    int first_occurence = 1;

    int capacity = 20;

    // Outer loop iterates over whole input
    while (*cur_char != '\0') {
        // Found separator
        if (strchr(sep, *cur_char) != NULL) {
            int length = cur_char - word_start;

            // Disallow two seps in a row
            if (length > 0 || first_occurence) {
                char *word = malloc(length+1);
                strncpy(word, word_start, length);
                word[length] = '\0';

                // Dynamically resize if needed
                if (*num_words >= capacity) {
                    capacity *= 2;
                    words_arr = realloc(words_arr, capacity * sizeof(char*));
                }

                // Assign word to array and update word count
                words_arr[*num_words] = word;
                (*num_words)++;
                first_occurence = 0;   
            }

            // Move on in input
            cur_char++;
            word_start = cur_char;
        } 
        // Non-separator, keep checking
        else {
            cur_char++;
        }
    }

    // Handle last word
    int len = cur_char - word_start;
    if (len > 0 || input[strlen(input) - 1] == *sep) {
        char *word = malloc(len + 1);
        strncpy(word, word_start, len);
        word[len] = '\0';

        // Dynamically resize if needed
        if (*num_words >= capacity) {
            capacity *= 2;
            words_arr = realloc(words_arr, capacity * sizeof(char *));
        }
        words_arr[*num_words] = word;
        (*num_words)++;
    }
    return words_arr;
}
