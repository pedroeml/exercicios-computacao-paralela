#include <stdio.h>
#include "mpi.h"

#define MESSAGE_LEN 10

void main(int argc, char** argv) {
    int my_rank;    // Identificador deste processo
    int proc_n;     // Numero de processos disparados pelo usuário na linha de comando (np) 
    int messages[MESSAGE_LEN];   // Buffer para as mensagens
    int message;
    int tag = 50;   // Tag para as mensagens
    MPI_Status status;  // Status de retorno

    int i;
    for (i = 0; i < MESSAGE_LEN; i ++)
        messages[i] = i;

    MPI_Init(&argc, &argv);     // funcao que inicializa o MPI, todo o código paralelo esta abaixo


    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);     // pega informação do numero de processos (quantidade total)

    // receber da esquerda

    for (i = 0; i < MESSAGE_LEN; i++) {
        if (my_rank == 0) {   // sou o primeiro?
            message = messages[i];    // sim, sou o primeiro, crio a mensagem sem receber
        } else {
            MPI_Recv(&message, 1, MPI_INT, my_rank-1, tag, MPI_COMM_WORLD, &status); // não sou o primeiro, recebo da esquerda
            printf("i = %d;\t[%d] Receiving %d from\t[%d]\n", i, my_rank, message, my_rank-1);
        }

        // processo mensagem

        // enviar para a direita

        if (my_rank == proc_n-1) { // sou o último?
            printf("i = %d;\t[%d] Message: %d\n", i, my_rank, message); // sim sou o último, apenas mostro mensagem na tela
        } else {
            MPI_Send(&message, 1, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD); // não sou o último, envio mensagem para a direita
            printf("i = %d;\t[%d] Sending %d to\t\t[%d]\n", i, my_rank, message, my_rank+1);
        }
    }

    MPI_Finalize();
}
