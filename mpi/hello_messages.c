#include <stdio.h>
#include "mpi.h"
 
void main(int argc, char** argv) {
    int my_rank;  // Identificador do processo
    int proc_n;   // Número de processos
    int source;   // Identificador do proc.origem
    int dest;     // Identificador do proc. destino
    int tag = 50; // Tag para as mensagens
    
    char message[100]; // Buffer para as mensagens
    MPI_Status status; // Status de retorno
    
    MPI_Init (&argc , & argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    
    if (my_rank != 0) {
        sprintf(message, "Greetings from process %d!", my_rank);
        dest = 0;
        MPI_Send (message, strlen(message)+1, MPI_CHAR,dest, tag, MPI_COMM_WORLD);
    } else {
        for (source = 1; source < proc_n; source++) {
            MPI_Recv (message, 100, MPI_CHAR , source, tag, MPI_COMM_WORLD, &status);
            printf("%s\n", message);
        }
    }
    
    MPI_Finalize();
}
