#include <stdio.h>
#include <unistd.h>
#include <omp.h>

#define N 20

int main() {
	int tid;
	omp_set_num_threads(4);	
	#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();
		printf("Hello %d\n", tid);
		#pragma omp sections
		{
			#pragma omp section
			{
				printf("Seção 1 - thread %d\n", tid);
				sleep(1);
			}
			#pragma omp section
			{
				printf("Seção 2 - thread %d\n", tid);
				sleep(1);
			}
			#pragma omp section
			{
				printf("Seção 3 - thread %d\n", tid);
				sleep(1);
			}
			#pragma omp section
			{
				printf("Seção 4 - thread %d\n", tid);
				sleep(1);
			}
		}
	}
	
	return 0;
}
