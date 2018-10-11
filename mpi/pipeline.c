#include <stdio.h>
#include "mpi.h"

#define MESSAGE_LEN 10

void print_message(int message[MESSAGE_LEN]) {
    printf("[");
    int i;
    for (i = 0; i < MESSAGE_LEN-1; i ++)
        printf("%d, ", *(message + i));
    printf("%d]", *(message + MESSAGE_LEN - 1));
}

void main(int argc, char** argv) {
    int my_rank;    // Identificador deste processo
    int proc_n;     // Numero de processos disparados pelo usuário na linha de comando (np) 
    int messages[MESSAGE_LEN][MESSAGE_LEN]; // Matriz de mensagens  
    int message[MESSAGE_LEN];               // Buffer para as mensagens
    int tag = 50;       // Tag para as mensagens
    MPI_Status status;  // Status de retorno

    MPI_Init(&argc, &argv);     // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);     // pega informação do numero de processos (quantidade total)

    // receber da esquerda
    
    int i;
    for (i = 0; i < MESSAGE_LEN; i++) {
        if (my_rank == 0) {   // sou o primeiro?
            int j;
            for (j = 0; j < MESSAGE_LEN; j++) {
                messages[i][j] = i*10+j;        // sim, sou o primeiro, crio a mensagem sem receber
                message[j] = messages[i][j];    // copia a mensagem
            }
                
        } else {
            MPI_Recv(message, MESSAGE_LEN, MPI_INT, my_rank-1, tag, MPI_COMM_WORLD, &status); // não sou o primeiro, recebo da esquerda
            printf("i = %d;\t[%d] Receiving ", i, my_rank);
            print_message(message);
            printf(" from\t[%d]\n", my_rank-1);
        }

        // processo mensagem

        // enviar para a direita

        if (my_rank == proc_n-1) { // sou o último?
            printf("i = %d;\t[%d] Message: ", i, my_rank); // sim sou o último, apenas mostro mensagem na tela
            print_message(message);
            printf("\n");
        } else {
            MPI_Send(message, MESSAGE_LEN, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD); // não sou o último, envio mensagem para a direita
            printf("i = %d;\t[%d] Sending ", i, my_rank);
            print_message(message);
            printf(" to\t\t[%d]\n", my_rank+1);
        }
    }

    MPI_Finalize();
}
