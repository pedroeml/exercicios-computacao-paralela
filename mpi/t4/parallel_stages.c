/***
 * How to compile: mpicc parallel_stages.c -o parallel-stages -lm
 * How to run: mpirun -np <NUMBER-OF-PROCESSES> parallel_stages
 * The number of processes must be 16 or 32
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define ARRAY_SIZE 40

void print_array(int* array, int len) {
    printf("[");
    int i;
    for (i = 0; i < len-1; i ++)
        printf("%d, ", *(array + i));
    printf("%d]\n", *(array + len - 1));
}

void bs(int n, int * vetor) {
    int c = 0, d, troca, trocou = 1;

    while (c < (n-1) & trocou ) {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++) {
            if (vetor[d] > vetor[d+1]) {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
            }
        }
        c++;
    }
}

void sort(int* array, int len, int my_rank) {
    bs(len, array);

    printf("[%d] Sorted: ");
    print_array(array, len);
}

void initialize_array(int* array, int len) {
    int i;
    for (i = 0; i < len; i++) {
        array[i] = len - i;
    }
}

void parallel_stages(int* array, int len, int my_rank) {
    int pronto = 0;

    while (!pronto) {
        sort(array, len, my_rank); // ordeno vetor local

        // verifico condição de parada

        // se não for np-1, mando o meu maior elemento para a direita

        // se não for 0, recebo o maior elemento da esquerda

        // comparo se o meu menor elemento é maior do que o maior elemento recebido (se sim, estou ordenado em relação ao meu vizinho)

        // compartilho o meu estado com todos os processos

        // MPI_Bcast(estado);

        // se todos estiverem ordenados com seus vizinhos, a ordenação do vetor global está pronta ( pronto = TRUE, break)

        // senão continuo

        // troco valores para convergir

        // se não for o 0, mando os menores valores do meu vetor para a esquerda

        // se não for np-1, recebo os menores valores da direita

            // ordeno estes valores com a parte mais alta do meu vetor local

            // devolvo os valores que recebi para a direita

        // se não for o 0, recebo de volta os maiores valores da esquerda
    }
}

int main(int argc, char** argv) {
    int my_rank;    // Identificador deste processo
    int proc_n;     // Numero de processos disparados pelo usuário na linha de comando (np)

    MPI_Init(&argc, &argv);     // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);     // pega informação do numero de processos (quantidade total)
    
    int len = ARRAY_SIZE/proc_n;
    int array[len];

    if (my_rank == 0) {
        initialize_array(&array, len);
        printf("[%d]");
        print_array(&array, len);
    } else (my_rank < proc_n - 1) {
        initialize_array(&array, len);
        printf("[%d]");
        print_array(&array, len);
    } else {
        initialize_array(&array, len);
        printf("[%d]");
        print_array(&array, len);
    }

    MPI_Finalize();
    return 0;
}
