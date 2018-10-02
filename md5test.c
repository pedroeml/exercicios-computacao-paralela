/*** 
 * How to compile: 
 * GCC 4.6.3: gcc -o md5test md5test.c -lcrypto -lssl -std=c99
 * GCC 6.3.0: gcc -o md5test md5test.c -lcrypto -lssl
 **/

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
    printf("\n");
}

unsigned char* str_to_md5(char* str) {
    unsigned char* result = (unsigned char*) malloc(MD5_DIGEST_LENGTH*sizeof(unsigned char));
    
    MD5(str, strlen(str), result);

    return result;
}

char** str_split(char* a_str, const char a_delim, size_t* len) {
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
        size_t idx = 0;
        *len = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            if (strlen(token) > 1)
                (*len)++;
            token = strtok(0, delim);
        }
        *(result + idx) = 0;
    }

    return result;
}

typedef struct Line1 {
    char* str;
    unsigned char* md5;
} Line;

typedef struct Book1 {
    int number;
    size_t lines_len;
    Line* lines;
} Book;

void books_print(Book* books) {
    for (int i = 0; i < NUMBER_OF_BOOKS; i++) {
        for (int j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            printf("%d: %d/%d %s\n", books[i].number, j, books[i].lines_len, line->str);
            md5_print(line->md5);
        }
    }
}

int main() {
    Book books[NUMBER_OF_BOOKS];

    for (int i = 1; i <= NUMBER_OF_BOOKS; i++) {
        books[i-1].number = i;

        char* filepath = (char*) malloc(25*sizeof(char));
        char* filename = (char*) malloc(8*sizeof(char));
        
        sprintf(filepath, "plain_text_books/%d.txt", i);
        sprintf(filename, "%d.txt", i);
        
        char* str = file_to_str(filepath, filename);
        free(filepath);
        free(filename);

        char** tokens;
        size_t len;
        tokens = str_split(str, '\n', &len);
        free(str);

        books[i-1].lines_len = len;
        books[i-1].lines = (Line*) malloc(len*sizeof(Line));

        if (tokens) {
            int count = 0;
            for (int j = 0; *(tokens + j); j++) {
                if (strlen(*(tokens + j)) > 1) {
                    unsigned char* md5 = str_to_md5(*(tokens + j));
                    Line* line = &(books[i-1].lines[count++]);
                    line->str = *(tokens + j);
                    line->md5 = md5;
                }
            }
        }
    }

    books_print(&books);

    return EXIT_SUCCESS;
}
