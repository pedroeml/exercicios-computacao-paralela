// How to compile: gcc -o md5test md5test.c -lcrypto -lssl -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#define NUMBER_OF_BOOKS 30

char* file_to_str(char* filepath, char* filename) {
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

int main() {
    for (int i = 1; i <= NUMBER_OF_BOOKS; i++) {
        char* filepath = (char*) malloc(25*sizeof(char));
        char* filename = (char*) malloc(8*sizeof(char));
        
        sprintf(filepath, "plain_text_books/%d.txt", i);
        sprintf(filename, "%d.txt", i);
        
        char* str = file_to_str(filepath, filename);
        
        unsigned char* md5_result = str_to_md5(str);
    }

    return EXIT_SUCCESS;
}
