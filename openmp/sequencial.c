// Autor    Roland Teodorowitsch
// Data:    ago. 2018
//
// Adaptado Cesar De Rose - set. 2018

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define SIZE 2500

void InicializaMatriz(void);
void VerificaMatriz(void);

// ESTRUTURA DE DADOS COMPARTILHADA

int m1[SIZE][SIZE], m2[SIZE][SIZE], mres[SIZE][SIZE];
int l1, c1, l2, c2;

int main() {
    int i, j, k, lres, cres;
    double starttime, stoptime;

    // INICIALIZA OS ARRAYS A SEREM MULTIPLICADOS

    l1 = c1 = SIZE;
    l2 = c2 = SIZE;

    lres = l1;
    cres = c2;

    InicializaMatriz();

    omp_set_num_threads(8);

    printf("\n  Multiplicando matrizes de tamanho %d com %d threads.\n", SIZE, omp_get_max_threads() );

    starttime = omp_get_wtime();

    // REALIZA A MULTIPLICACAO

    #pragma omp parallel for private(j, k)
    for (i = 0; i < lres; i++) {
        for (j = 0; j < cres; j++) {
            mres[i][j] = 0;
            for (k = 0; k < c1; k++) {
                mres[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }

    stoptime = omp_get_wtime();

    // VERIFICA SE O RESULTADO DA MULTIPLICACAO ESTA CORRETO

    VerificaMatriz();

    printf("  Tempo de execucao: %3.2f segundos\n\n", stoptime - starttime);
    return 0;
}

void InicializaMatriz(void) {
    int i, j, k;

    k=1;

    for (i = 0; i < SIZE; i++) {
        for (j=0; j < SIZE; j++) {
	        if (k%2 == 0)
                m1[i][j] = -k;
	        else
                m1[i][j] = k;
        }
        k++;
    }

    k=1;
    for (j = 0; j < SIZE; j++) {
        for (i = 0 ; i < SIZE; i++) {
	        if (k%2 == 0)
                m2[i][j] = -k;
	        else
                m2[i][j] = k;
        }
        k++;
    }
}

void VerificaMatriz(void) {
    int i, j, k;

    for (i = 0; i < SIZE; i++) {
        k = SIZE*(i+1);
        for (j = 0; j < SIZE; j++) {
            int k_col = k*(j+1);
            if (i % 2 == 0) {
                if (j % 2 == 0) {
                    if (mres[i][j]!=k_col)
                        printf("Verificacao Falhou!\n");
                } else {
                    if (mres[i][j]!=-k_col)
                        printf("Verificacao Falhou!\n");
                }
            } else {
                if (j % 2 == 0) {
                    if (mres[i][j]!=-k_col)
                    printf("Verificacao Falhou!\n");
                } else {
                    if (mres[i][j]!=k_col)
                        printf("Verificacao Falhou!\n");
                }
            }
        }
    }
}
