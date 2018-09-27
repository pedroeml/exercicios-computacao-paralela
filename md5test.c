// How to compile: gcc -o md5test md5test.c -lcrypto -lssl -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/md5.h>

#define NUMBER_OF_BOOKS 30

char* file_to_str(char* filepath, char* filename) {
    // https://stackoverflow.com/questions/3747086/reading-the-whole-text-file-into-a-char-array-in-c
    FILE* fp = fopen(filepath, "rb");
    long lSize;
    char* buffer;

    if (!fp) {
        perror(filename);
        exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = calloc(1, lSize + 1);
    if (!buffer) {
        fclose(fp);
        fputs("memory alloc fails", stderr);
        exit(1);
    }

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp)) {
        fclose(fp);
        free(buffer);
        fputs("entire read fails", stderr);
        exit(1);
    }

    fclose(fp);
    
    return buffer;
}

void md5_print(unsigned char* result) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", *(result + i));
    }
}

char* md5_result_to_str(unsigned char* result) {
    char* result_str = (char*) malloc(2*MD5_DIGEST_LENGTH*sizeof(char));
    char* ptr = result_str;
    
    printf("STR: ");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(ptr, "%02x", *(result + i));
        ptr += 2;
    }

    return result_str;
}

unsigned char* str_to_md5(char* str) {
    unsigned char* result = (unsigned char*) malloc(MD5_DIGEST_LENGTH*sizeof(unsigned char));
    
    MD5(str, strlen(str), result);

    printf("MD5: ");
    md5_print(result);
    printf("\n");

    char* result_str = md5_result_to_str(result);
    printf("%s\n\n", result_str);

    return result;
}

char** str_split(char* a_str, const char a_delim) {
    // https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int main() {
    for (int i = 1; i <= NUMBER_OF_BOOKS; i++) {
        char* filepath = (char*) malloc(25*sizeof(char));
        char* filename = (char*) malloc(8*sizeof(char));
        
        sprintf(filepath, "plain_text_books/%d.txt", i);
        sprintf(filename, "%d.txt", i);
        
        char* str = file_to_str(filepath, filename);
        free(filepath);
        free(filename);

        char** tokens;
        tokens = str_split(str, '\n');
        free(str);

        if (tokens) {
            for (int j = 0; *(tokens + j); j++) {
                if (strlen(*(tokens + j)) > 1) {
                    printf("Line %d: %s\n", j, *(tokens + j));
                    // unsigned char* md5_result = str_to_md5(*(tokens + j));
                    // TODO: md5_result needs to be free later
                    free(*(tokens + j));
                }
            }
            printf("\n");
            // free(tokens);
        }
    }

    return EXIT_SUCCESS;
}
