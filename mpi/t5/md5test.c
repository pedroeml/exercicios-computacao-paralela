/*** 
 * How to compile: 
 * GCC 4.6.3: mpicc md5test.c -o md5test -fopenmp -lcrypto -lssl -std=c99
 * GCC 6.3.0: mpicc md5test.c -o md5test -fopenmp -lcrypto -lssl
 * 
 * 5 nodos no máximo
 * - 1 mestre
 *      - O mestre só envia a mensagem para os escravos dizendo encontre essa linha e me digam se encontrar ou não.
 *      - Portanto o escravo irá ficar subutilizando a máquina em que estiver. 
 * - 5 escravos (6 livros por escravo) 
 *      - Não pode ser 4 escravos por que são 30 livros.
 *      - Isso resultaria em 7.5 livros por escravo, o que não é possível fazer. 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <openssl/md5.h>
#include <mpi.h>
#include <omp.h>

#define NUMBER_OF_BOOKS 30
#define NUMBER_OF_SLAVES 5
#define NUMBER_OF_BOOKS_BY_SLAVE 6
#define NUMBER_OF_THREADS 8
#define STOP_WHEN_FOUND 1

MPI_Status status; // estrutura que guarda o estado de retorno

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
    int i;
    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
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
    int i;
    for (i = 0; i < NUMBER_OF_BOOKS; i++) {
        printf("Book %d; total lines %lu\n", books[i].number, books[i].lines_len);
        int j;
        for (j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            printf("Book %d; line %d of %lu: %s\n", books[i].number, j+1, books[i].lines_len, line->str);
            md5_print(line->md5);
        }
    }
}

bool md5_equals(unsigned char* md5_a, unsigned char* md5_b) {
    int i;
    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
        if (*(md5_a + i) != *(md5_b + i))
            return false;
    
    return true;
}

int find_line_in_book(Book* books, unsigned char* md5) {
    int i;
    int book_number = -1;

    #pragma omp parallel shared(md5, book_number)
    #pragma omp for schedule (dynamic)
    for (i = 0; i < NUMBER_OF_BOOKS_BY_SLAVE; i++) {
        int j;
        for (j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            if (book_number != -1) {
                if (STOP_WHEN_FOUND == 1) {
                    break;
                } else {
                    continue;
                }
            } else if (md5_equals(md5, line->md5)) {
                #pragma omp atomic
                book_number += books[i].number;
            }
        }
    }
    
    return book_number;
}

int find_line_in_books(Book* books, char* line_str) {
    unsigned char* md5 = str_to_md5(line_str);
    int book_number = 0;
    
    int i;
    for (i = 1; i <= NUMBER_OF_SLAVES; i++) {
        MPI_Send(md5, MD5_DIGEST_LENGTH, MPI_UNSIGNED_CHAR, i, 1, MPI_COMM_WORLD);
    }
    
    free(md5);

    for (i = 1; i <= NUMBER_OF_SLAVES; i++) {
        int found = 0;
        MPI_Recv(&found, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
        book_number = found > 0 ? found : 0;
        if (book_number > 0) {
            //printf("\nEscravo[%d]: encontrou no livro %d", i, found);
        } else {
            //printf("\nEscravo[%d]: não encontrou", i);
        }
    }

    return book_number;
}

void find_all_lines_in_books(Book* books) {
    int i;
    for (i = 0; i < NUMBER_OF_BOOKS; i++) {
        printf("\rBook %d of %d", i+1, NUMBER_OF_BOOKS);
        fflush(stdout);
        int j;
        for (j = 0; j < books[i].lines_len; j++) {
            Line* line = &(books[i].lines[j]);
            char* line_str = line->str;
            int book_number = find_line_in_books(books, line_str);
        }
    }
    printf("\n");
}

size_t number_of_lines_in_books(Book* books) {
    size_t all_lines_len = 0;
    int i;
    for (i = 0; i < NUMBER_OF_BOOKS; i++) {
        all_lines_len += books[i].lines_len;
    }

    return all_lines_len;
}

void free_books(Book* books, int len) {
    int i;
    for (i = 0; i < len; i++) {
        int j;
        for (j = 0; j < books[i].lines_len; j++) {
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

int main(int argc, char** argv) {
    int my_rank;       // Identificador deste processo
    int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)
    
    MPI_Init(&argc , &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

    if (proc_n != NUMBER_OF_SLAVES + 1) {
        printf("The number of processes (%d) must be equal to the number of slaves (%d) + 1 (the master)!\n", proc_n, NUMBER_OF_SLAVES);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (my_rank == 0) {     // papel do mestre
        Book books[NUMBER_OF_BOOKS];
        
        int slave;
        int i = 1;
        for (slave = 1; slave <= NUMBER_OF_SLAVES; slave++) {
            int cont;
            for (cont = 1; cont <= NUMBER_OF_BOOKS_BY_SLAVE; cont++) {
                books[i - 1].number = i;

                char* whole_text = load_book_i(i);
                size_t len = strlen(whole_text);
                
                MPI_Send(&i, 1, MPI_INT, slave, 1, MPI_COMM_WORLD);
                MPI_Send(&len, 1, MPI_UNSIGNED_LONG, slave, 1, MPI_COMM_WORLD);
                MPI_Send(whole_text, len, MPI_CHAR, slave, 1, MPI_COMM_WORLD);

                update_book(&(books[i-1]), whole_text);

                i++;

                free(whole_text);
            }
        }

        size_t all_lines_len = number_of_lines_in_books(books);
        for (slave = 1; slave <= NUMBER_OF_SLAVES; slave++) {
            MPI_Send(&all_lines_len, 1, MPI_UNSIGNED_LONG, slave, 1, MPI_COMM_WORLD);
        }

        // books_print(&books);

        double starttime = MPI_Wtime();
        find_all_lines_in_books(&books);
        double stoptime = MPI_Wtime();
        double executiontime = stoptime - starttime;
        printf("Execution time: %.2f s\n", executiontime);

        free_books(&books, NUMBER_OF_BOOKS);
    } else {    // papel do escravo
        omp_set_num_threads(NUMBER_OF_THREADS);
        
        Book books[NUMBER_OF_BOOKS_BY_SLAVE];
        
        int i;
        for (i = 1; i <= NUMBER_OF_BOOKS_BY_SLAVE; i++) {
            int book_number;
            MPI_Recv(&book_number, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

            size_t len;
            MPI_Recv(&len, 1, MPI_UNSIGNED_LONG, 0, 1, MPI_COMM_WORLD, &status);

            char* whole_text = (char*) malloc((1+len)*sizeof(char));
            *(whole_text + len) = '\0';
            MPI_Recv(whole_text, len, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);

            books[i-1].number = book_number;
            update_book(&(books[i-1]), whole_text);

            free(whole_text);
        }

        size_t all_lines_len;
        MPI_Recv(&all_lines_len, 1, MPI_UNSIGNED_LONG, 0, 1, MPI_COMM_WORLD, &status);

        size_t request_number;
        for (request_number = 0; request_number < all_lines_len; request_number++) {
            unsigned char md5[MD5_DIGEST_LENGTH];
            MPI_Recv(&md5, MD5_DIGEST_LENGTH, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD, &status);

            int found = find_line_in_book(&books, &md5);
            MPI_Send(&found, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }

        free_books(&books, NUMBER_OF_BOOKS_BY_SLAVE);
    }
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}
