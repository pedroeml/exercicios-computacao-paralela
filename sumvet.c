#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define N 100000000
#define NUM_TH 4

void print_array(float* v, int length) {
    printf("[");
    for (int i = 0; i < length-1; i++) {
        printf("%.2f, ", *(v + i));
    }
    printf("%.2f]\n", *(v + length - 1));
}

int main (int argc, char *argv[]) {
    float* a = (float*) malloc(N*sizeof(float));
    float* b = (float*) malloc(N*sizeof(float));
    float sum; 

    omp_set_num_threads(NUM_TH);

    double starttime, stoptime;

    starttime = omp_get_wtime();

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
        const float val = i * 1.0;
        *(a + i) = val;
        *(b + i) = val;
    }

    stoptime = omp_get_wtime();
    printf("Tempo de execução: %3.6f segundos\n", stoptime-starttime);

    if (N < 15) {
        print_array(a, N);
        print_array(b, N);
    }

    sum = 0.0;
    
    starttime = omp_get_wtime();

    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += *(a + i) * (*(b + i));
    }

    stoptime = omp_get_wtime();
    printf("Tempo de execução: %3.6f segundos\n", stoptime-starttime);
    
    free(a);
    free(b);

    printf("   Sum = %f\n",sum);

    return 0;
}
