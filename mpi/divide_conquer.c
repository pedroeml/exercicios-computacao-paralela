/***
 * How to compile: mpicc divide_conquer.c -o divide_conquer -lm
 * How to run: mpirun -np <NUMBER-OF-PROCESSES-POWER-OF-2> divide_conquer
 **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define DEBUG 1            // comentar esta linha quando for medir tempo
#define ARRAY_SIZE 80      // trabalho final com o valores 10.000, 100.000, 1.000.000
int delta;
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

int calc_next_target(int my_rank, int tree_level) {
    return round(pow(2.0, tree_level) + my_rank);
}

int calc_father(int my_rank, int tree_level) {
    return round(my_rank - pow(2.0, tree_level));
}

void sort(int* array, int len, int my_rank) {
    bs(len, array);

    printf("[%d] sorted array of len %d\n", my_rank, len);
    print_array(array, len);
}


void recursive_join(int* array, int len, int my_rank, int father, int tree_level) {
    if (tree_level > 0) {
        if (father > -1 && father != my_rank) {
            MPI_Send(&len, 1, MPI_INT, father, 1, MPI_COMM_WORLD);
            MPI_Send(array, len, MPI_INT, father, 1, MPI_COMM_WORLD);
        } else {
            int new_tree_level = tree_level - 1;
            int half_len;
            int son = calc_next_target(my_rank, new_tree_level);
            MPI_Recv(&half_len, 1, MPI_INT, son, 1, MPI_COMM_WORLD, &status);
            
            int half_array[half_len];
            MPI_Recv(&half_array, half_len, MPI_INT, son, 1, MPI_COMM_WORLD, &status);
            
            int new_array[len+half_len];
            int i;
            for (i = 0; i < len; i++) {
                new_array[i] = *(array + i);
            }
            for (i = len; i < len+half_len; i++) {
                new_array[i] = half_array[i - len];
            }

            sort(new_array, len+half_len, my_rank);

            if (new_tree_level > 0) {
                int new_father = calc_father(my_rank, new_tree_level-1);
                recursive_join(&new_array, len+half_len, my_rank, new_father < 0 ? my_rank : new_father, new_tree_level);
            }
        }
    }
}


void divide_if_needed(int* array, int len, int target, int my_rank, int father, int tree_level) {
    if (len > delta) {
        int half_len = len/2;
        int* half_array = &(array[half_len]);   // The second half of array
        
        print_array(half_array, half_len);
        
        int new_tree_level = tree_level + 1;
        MPI_Send(&my_rank, 1, MPI_INT, target, 1, MPI_COMM_WORLD);
        MPI_Send(&new_tree_level, 1, MPI_INT, target, 1, MPI_COMM_WORLD);
        MPI_Send(&half_len, 1, MPI_INT, target, 1, MPI_COMM_WORLD);
        MPI_Send(half_array, half_len, MPI_INT, target, 1, MPI_COMM_WORLD);

        if (half_len > delta) {
            divide_if_needed(array, half_len, calc_next_target(my_rank, new_tree_level), my_rank, my_rank, new_tree_level);
        } else {
            sort(array, half_len, my_rank);
            recursive_join(array, half_len, my_rank, my_rank, new_tree_level);
        }
    } else {
        sort(array, len, my_rank);
        recursive_join(array, len, my_rank, father, tree_level);
    }
}

void divide(int* array, int len, int targetA, int targetB, int my_rank, int father, int tree_level) {
    if (len > delta) {
        int half_len = len/2;
        int* half_array = &(array[half_len]);   // The second half of array
        
        print_array(array, half_len);
        print_array(half_array, half_len);
        
        int new_tree_level = tree_level + 1;
        
        MPI_Send(&my_rank, 1, MPI_INT, targetA, 1, MPI_COMM_WORLD);
        MPI_Send(&new_tree_level, 1, MPI_INT, targetA, 1, MPI_COMM_WORLD);
        MPI_Send(&half_len, 1, MPI_INT, targetA, 1, MPI_COMM_WORLD);
        MPI_Send(array, half_len, MPI_INT, targetA, 1, MPI_COMM_WORLD);
        
        MPI_Send(&my_rank, 1, MPI_INT, targetB, 1, MPI_COMM_WORLD);
        MPI_Send(&new_tree_level, 1, MPI_INT, targetB, 1, MPI_COMM_WORLD);
        MPI_Send(&half_len, 1, MPI_INT, targetB, 1, MPI_COMM_WORLD);
        MPI_Send(half_array, half_len, MPI_INT, targetB, 1, MPI_COMM_WORLD);

        MPI_Recv(array, half_len, MPI_INT, targetA, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(half_array, half_len, MPI_INT, targetB, 1, MPI_COMM_WORLD, &status);

        sort(array, len, my_rank);

        MPI_Send(array, len, MPI_INT, father, 1, MPI_COMM_WORLD);
    } else {
        sort(array, len, my_rank);

        MPI_Send(half_array, half_len, MPI_INT, father, 1, MPI_COMM_WORLD);
    }
}


int main(int argc, char** argv) {
    int my_rank;    // Identificador deste processo
    int proc_n;     // Numero de processos disparados pelo usuário na linha de comando (np)

    MPI_Init(&argc, &argv);     // funcao que inicializa o MPI, todo o código paralelo esta abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    // pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);     // pega informação do numero de processos (quantidade total)
    delta = ARRAY_SIZE/proc_n;
    
    if (my_rank == 0) {
        printf("DELTA = %d\n", delta);
        int vetor[ARRAY_SIZE];
        int i;

        for (i = 0; i < ARRAY_SIZE; i++)              /* init array with worst case for sorting */
            vetor[i] = ARRAY_SIZE-i;
        
        int tree_level = 0;
        
        // TODO: selector mechanism to select which version to use

        divide_if_needed(&vetor, ARRAY_SIZE, calc_next_target(my_rank, tree_level), my_rank, -1, tree_level);
    } else {
        int father;
        MPI_Recv(&father, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        father = status.MPI_SOURCE;
        int tree_level;
        MPI_Recv(&tree_level, 1, MPI_INT, father, 1, MPI_COMM_WORLD, &status);
        int len;
        MPI_Recv(&len, 1, MPI_INT, father, 1, MPI_COMM_WORLD, &status);
        int half_array[len];
        MPI_Recv(&half_array, len, MPI_INT, father, 1, MPI_COMM_WORLD, &status);
        
        // TODO: selector mechanism to select which version to use
        
        divide_if_needed(&half_array, len, calc_next_target(my_rank, tree_level), my_rank, father, tree_level);
    }
    
    MPI_Finalize();
    return 0;
}
