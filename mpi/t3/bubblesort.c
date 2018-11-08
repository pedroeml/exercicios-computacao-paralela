/***
 * How to compile: mpicc bubblesort.c -o bubblesort
 * How to run: mpirun -np 1 bubblesort
 **/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ARRAY_SIZE 500000

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

int main(int argc, char** argv) {
    int my_rank;    // Identificador deste processo
    int proc_n;     // Numero de processos disparados pelo usuário na linha de comando (np)
    MPI_Status status;  // Status de retorno

    MPI_Init(&argc, &argv);     // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);     // pega informação do numero de processos (quantidade total)
    
    if (my_rank == 0) {
        int vetor[ARRAY_SIZE];
        int i;

        for (i = 0; i < ARRAY_SIZE; i++)              /* init array with worst case for sorting */
            vetor[i] = ARRAY_SIZE-i;
        
        double starttime = MPI_Wtime();
        
        bs(ARRAY_SIZE, &vetor);

        double stoptime = MPI_Wtime();
        double executiontime = stoptime - starttime;
        printf("\nExecution time: %.2f s\n", executiontime);
    }
    
    MPI_Finalize();
    return 0;
}
