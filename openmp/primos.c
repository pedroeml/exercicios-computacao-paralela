#include<stdlib.h>
#include<stdio.h>
#include<omp.h>

int main (int argc, const char** argv) {
	if (argc < 2) {
        printf("Usage ./%s <number_of_threads>\n", argv[0]);
        exit(0);
    }

	printf("Number of processors: %d\n", omp_get_num_procs());

    const int NUM_TH = atoi(argv[1]);
	
	const int intervalo = 5000;
	double starttime, stoptime;

	int i,j,k;
	int prime;
	int total;
	
	starttime = omp_get_wtime(); 

    omp_set_num_threads(NUM_TH);
	
	#pragma omp parallel private ( i, j, k, prime )
	#pragma omp for schedule (dynamic)
	for (k = 1 ; k <= intervalo ; k++) { 
		total = 0;
		for ( i = 2; i <= k ; i++ ) {
		    prime = 1;
		        for ( j = 2; j < i; j++ ) {
			        if ( i % j == 0 ) {
				      prime = 0;
				      break;
					}
				}
		    total = total + prime;
		}
	     
	    printf("O número de primos do intervalo [1-%d] é %d\n", k, total);
	}
	
	stoptime = omp_get_wtime();
	printf("Tempo de execução: %3.2f segundos\n", stoptime-starttime);
	return(0);
}
