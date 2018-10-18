#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main (int argc, char** argv) {
    if (argc < 2) {
        printf("Usage ./%s <number_of_threads>\n", argv[0]);
        exit(0);
    }

    printf("Number of processors: %d\n", omp_get_num_procs());

    const int NUM_TH = atoi(argv[1]);

    int th_id, nthreads;

    omp_set_num_threads(NUM_TH); // disparar 4 threads pois se trata de uma máquina Quad-Core
    
    #pragma omp parallel private(th_id, nthreads) num_threads(NUM_TH)
    {
        th_id = omp_get_thread_num();
        nthreads = omp_get_num_threads();

        printf("Hello World from thread %d of %d threads.\n", th_id, nthreads);
    }

    getchar();
    return EXIT_SUCCESS;
}
