#include <stdio.h>
#include <omp.h>

#define N 16

int main(int argc, char **argv) {

	int i;
	
	omp_set_num_threads(4);
	// #pragma omp parallel for schedule(static)
	// #pragma omp parallel for schedule(static, 2)
	// #pragma omp parallel for schedule(dynamic)
	// #pragma omp parallel for schedule(dynamic, 2)
	#pragma omp parallel for schedule(guided)
	for (i = 0; i < N; i++) {
		printf("%d: processing element %d\n", omp_get_thread_num(), i);
	}
}
