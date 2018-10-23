/*** 
 * How to compile: 
 * GCC 4.6.3: gcc -fopenmp -o md5test md5test.c -lcrypto -lssl -std=c99
 * GCC 6.3.0: gcc -fopenmp -o md5test md5test.c -lcrypto -lssl
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <openssl/md5.h>
#include <omp.h>

#define NUMBER_OF_BOOKS 30

typedef struct Line1 {
    char* str;
    unsigned char* md5;
} Line;

typedef struct Book1 {
    int number;
    size_t lines_len;
    Line* lines;
} Book;

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
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x", *(result + i));
    
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
            *(result + idx++) = strdup(token);
            if (strlen(token) > 1)
                (*len)++;
            token = strtok(0, delim);
        }
        *(result + idx) = 0;
    }

    return result;
}

char* load_book_i(int i) {
    char filepath[25];
    char filename[8];
        
    sprintf(filepath, "plain_text_books/%d.txt", i);
    sprintf(filename, "%d.txt", i);
    
    char* whole_text = file_to_str(filepath, filename);
    
    return whole_text;
}

void books_print(Book* books) {
    for (int i = 0; i < NUMBER_OF_BOOKS; i++) {
        printf("Book %d; total lines %lu\n", books[i].number, books[i].lines_len);
        for (int j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            printf("Book %d; line %d of %lu: %s\n", books[i].number, j+1, books[i].lines_len, line->str);
            md5_print(line->md5);
        }
    }
}

bool md5_equals(unsigned char* md5_a, unsigned char* md5_b) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        if (*(md5_a + i) != *(md5_b + i))
            return false;
    
    return true;
}

int find_line_in_books(Book* books, char* line_str) {
    unsigned char* md5 = str_to_md5(line_str);
    int book_number = 0;
    
    #pragma omp parallel shared(md5, book_number)
    #pragma omp for schedule (dynamic)
    for (int i = 0; i < NUMBER_OF_BOOKS; i++) {
        for (int j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            if (book_number != 0) {
                break;
            } else if (md5_equals(md5, line->md5)) {
                #pragma omp atomic
                book_number += books[i].number;   
            }
        }
    }
    
    free(md5);

    return book_number;
}

void find_all_lines_in_books(Book* books) {
    for (int i = 0; i < NUMBER_OF_BOOKS; i++) {
        printf("\rBook %d of %d", i+1, NUMBER_OF_BOOKS);
        fflush(stdout);
        for (int j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            char* line_str = line->str;
            int book_number = find_line_in_books(books, line_str);
        }
    }
    printf("\n");
}

void free_books(Book* books) {
    for (int i = 0; i < NUMBER_OF_BOOKS; i++) {
        for (int j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            char* line_str = line->str;
            unsigned char* md5 = line->md5;
            free(md5);
            free(line_str);
        }
        Line* lines = &(books[i].lines);
        // free(lines); // TODO: Freeing all lines structs not working
    }
}

void update_book(Book* book, char* whole_text) {
    size_t len;
    char** lines = str_split(whole_text, '\n', &len);
    book->lines_len = len;
    book->lines = (Line*) malloc(len*sizeof(Line));
    
    if (lines) {
        int count = 0;
        int j;
        for (j = 0; *(lines + j); j++) {
            if (strlen(*(lines + j)) > 1) {
                Line* line = &(book->lines[count++]);
                line->str = *(lines + j);
                line->md5 = str_to_md5(*(lines + j));
            }
        }
    }
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        printf("Usage %s <number_of_threads>\n", argv[0]);
        exit(0);
    }

	printf("Number of processors: %d\n", omp_get_num_procs());

    const int NUM_THREADS = atoi(argv[1]);
    omp_set_num_threads(NUM_THREADS);

    printf("Number of threads: %d\n", NUM_THREADS);

    Book books[NUMBER_OF_BOOKS];

    for (int i = 1; i <= NUMBER_OF_BOOKS; i++) {
        books[i-1].number = i;

        char* whole_text = load_book_i(i);

        update_book(&(books[i-1]), whole_text);

        free(whole_text);
    }
    
    books_print(&books);

    double starttime, stoptime;
    starttime = omp_get_wtime();

    find_all_lines_in_books(&books);

    stoptime = omp_get_wtime();
    printf("Execution time: %3.2f s\n", stoptime-starttime);

    free_books(&books);

    return EXIT_SUCCESS;
}
