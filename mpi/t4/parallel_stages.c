/***
 * How to compile: mpicc parallel_stages.c -o parallel-stages -lm
 * How to run: mpirun -np <NUMBER-OF-PROCESSES> parallel_stages
 * The number of processes must be 16 or 32
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <math.h>
#include <mpi.h>

#define ARRAY_SIZE 100000
#define DELTA ((int)(ARRAY_SIZE*0.025))
MPI_Status status;  // Status de retorno

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
}

void initialize_array(int* array, int len, int my_rank, int proc_n) {
    int i;
    for (i = 0; i < len; i++) {
        array[i] = (proc_n - my_rank)*len - i;
    }
}

bool check_states(int* states, int len) {
    bool all_sorted = true;
    int i;
    for (i = 0; i < len; i++) {
        if (states[i] == 0) {
            all_sorted = false;
            break;
        }
    }
    return all_sorted;
}

void parallel_stages(int len, int my_rank, int proc_n) {
    int array[len + DELTA];
    initialize_array(&array, len, my_rank, proc_n);
    int states[proc_n];  // Initialize with zeros
    states[0] = 1;
    int i;
    for (i = 1; i < proc_n; i++) {
        states[i] = 0;
    }

    bool done = false;
    while (!done) {
        sort(&array, len, my_rank); // ordeno vetor local

        if (my_rank != proc_n - 1) {    // se não for np-1, mando o meu maior elemento para a direita
            MPI_Send(&array[len - 1], 1, MPI_INT, my_rank + 1, 1, MPI_COMM_WORLD);
        }

        int left_greatest;
        if (my_rank != 0) {     // se não for 0, recebo o maior elemento da esquerda
            MPI_Recv(&left_greatest, 1, MPI_INT, my_rank - 1, 1, MPI_COMM_WORLD, &status);
            
            if (array[0] > left_greatest) {    // comparo se o meu menor elemento é maior do que o maior elemento recebido (se sim, estou ordenado em relação ao meu vizinho)
                states[my_rank] = 1;
            }
        }
        
        for (i = 0; i < proc_n; i++) {
            MPI_Bcast(&states[i], 1, MPI_INT, i, MPI_COMM_WORLD);    // compartilho o meu estado com todos os processos
        }

        done = check_states(states, proc_n);    // verifico condição de parada
        if (done)   // se todos estiverem ordenados com seus vizinhos, a ordenação do vetor global está pronta ( pronto = TRUE, break)
            break;

        if (my_rank != 0) {     // se não for o 0, mando os menores valores do meu vetor para a esquerda
            MPI_Send(&array[0], DELTA, MPI_INT, my_rank - 1, 1, MPI_COMM_WORLD);
        }

        if (my_rank != proc_n - 1) {    // se não for np-1, recebo os menores valores da direita
            MPI_Recv(&array[len], DELTA, MPI_INT, my_rank + 1, 1, MPI_COMM_WORLD, &status);
            sort(&array[len - DELTA], 2*DELTA, my_rank); // ordeno estes valores com a parte mais alta do meu vetor local
            MPI_Send(&array[len], DELTA, MPI_INT, my_rank + 1, 1, MPI_COMM_WORLD);    // devolvo os valores que recebi para a direita
        }
            
        if (my_rank != 0) {     // se não for o 0, recebo de volta os maiores valores da esquerda
            MPI_Recv(&array[0], DELTA, MPI_INT, my_rank - 1, 1, MPI_COMM_WORLD, &status);
        }
    }
}

int main(int argc, char** argv) {
    int my_rank;    // Identificador deste processo
    int proc_n;     // Numero de processos disparados pelo usuário na linha de comando (np)

    MPI_Init(&argc, &argv);     // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);     // pega informação do numero de processos (quantidade total)
    
    int len = ARRAY_SIZE/proc_n;
    double starttime;
    
    if (my_rank == 0) {
        starttime = MPI_Wtime();
    }

    parallel_stages(len, my_rank, proc_n);

    if (my_rank == 0) {
        double stoptime = MPI_Wtime();
        double executiontime = stoptime - starttime;
        printf("\nExecution time: %.2f s\n", executiontime);
    }

    MPI_Finalize();
    return 0;
}
