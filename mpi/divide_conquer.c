/***
 * How to compile: mpicc divide_conquer.c -o divide_conquer -lm
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define DEBUG 1            // comentar esta linha quando for medir tempo
#define ARRAY_SIZE 40      // trabalho final com o valores 10.000, 100.000, 1.000.000
#define DELTA 10

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

/** 
 * recebe um ponteiro para um vetor que contem as mensagens recebidas dos filhos e
 * intercala estes valores em um terceiro vetor auxiliar. Devolve um ponteiro para este vetor 
 */
int *interleaving(int vetor[], int tam) {
	int *vetor_auxiliar;
	int i1, i2, i_aux;

	vetor_auxiliar = (int*) malloc(tam*sizeof(int));

	i1 = 0;
	i2 = tam/2;

	for (i_aux = 0; i_aux < tam; i_aux++) {
		if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2))) || (i2 == tam)) {
			vetor_auxiliar[i_aux] = vetor[i1++];
        } else {
			vetor_auxiliar[i_aux] = vetor[i2++];
        }
	}

	return vetor_auxiliar;
}


int calc_next_target(int my_rank, int tree_level) {
    int target = round(pow(2.0, tree_level) + my_rank);

    printf("[%d] Next target: %d\n", my_rank, target);
    
    return target;
}

void divide_if_needed(int* array, int len, int target, int my_rank, int tree_level) {
    int half_len = len/2;
    int* half_array = &(array[half_len - 1]);   // The second half of array
    
    #ifdef DEBUG
    printf("[%d] array of len %d\n", my_rank, len);
    print_array(array, len);
    printf("[%d] half_array of len %d\n", my_rank, half_len);
    print_array(half_array, half_len);
    #endif

    MPI_Send(&my_rank, 1, MPI_INT, target, 1, MPI_COMM_WORLD);
    MPI_Send(&tree_level, 1, MPI_INT, target, 1, MPI_COMM_WORLD);
    MPI_Send(&half_len, 1, MPI_INT, target, 1, MPI_COMM_WORLD);
    MPI_Send(half_array, half_len, MPI_INT, target, 1, MPI_COMM_WORLD);

    if (half_len > DELTA) {
        divide_if_needed(half_array, half_len, calc_next_target(my_rank, tree_level+1), my_rank, tree_level+1);
    } else {
        bs(half_len, half_array);

        #ifdef DEBUG
        printf("[%d] sorted half_array of len %d\n", my_rank, half_len);
        print_array(half_array, half_len);
        #endif

        // TODO: send to father node using interleaving function maybe?
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
        
        #ifdef DEBUG
        printf("\nVetor: ");
        print_array(&vetor, ARRAY_SIZE);
        #endif
        int tree_level = 0;
        divide_if_needed(&vetor, ARRAY_SIZE, calc_next_target(my_rank, tree_level), my_rank, tree_level);
    } else {
        int i;
        int father;
        printf("[%d] Waiting for father node...\n", my_rank);
        for (i = 0; i < my_rank; i++) {
            MPI_Recv(&father, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
        printf("[%d] Father is %d\n", my_rank, father);

        int tree_level;
        MPI_Recv(&tree_level, 1, MPI_INT, father, 1, MPI_COMM_WORLD, &status);

        int len;
        MPI_Recv(&len, 1, MPI_INT, father, 1, MPI_COMM_WORLD, &status);
        int half_array[len];
        MPI_Recv(&half_array, len, MPI_INT, father, 1, MPI_COMM_WORLD, &status);
        
        divide_if_needed(&half_array, len, calc_next_target(my_rank, tree_level+1), my_rank, tree_level);
    }
    
    MPI_Finalize();
    return 0;
}
