#include "mpi.h"
#include "synchronizer.h"
#include "hirschberg_sinclair_standard.h"
#include "hirschberg_sinclair_optimized.h"
#include "peterson.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Function to swap values at two pointers */
void swap(int *x, int *y) {
	int temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

void printArray(int *array, int length) {
	int i;
	for (i = 0; i < length - 1; ++i) {
		printf("%d, ", array[i]);
	}
	printf("%d \n", array[length - 1]);
}

void permute(int *array, int i, int length, int **permutations, int *position) {
	if (length == i) {
		printArray(array, length);
		printf("%d \n", *position);
		int a = 0;
		for (; a < length; ++a) {
			permutations[*position][a] = array[a];
		}
		//permutations[*position] = array;
		printArray(permutations[*position], length);
		*position = *position + 1;
		return;
	}
	int j = i;
	for (j = i; j < length; j++) {
		swap(array + i, array + j);
		permute(array, i + 1, length, permutations, position);
		swap(array + i, array + j);
	}
	return;
}

int main(int argc, char *argv[]) {

	/** permutation **/
	int n = 6, perm = 720;
	int **permutations = (int**) malloc(sizeof(int*) * perm);
	int i;
	for (i = 0; i < perm; ++i) {
		permutations[i] = (int*) malloc(sizeof(int) * n);
	}
	int ids[] = { 7, 10, 6, 4, 2, 8 };

	int *position = (int*) malloc(sizeof(int));
	*position = 0;
	permute(ids, 0, n, permutations, position);
//	printf("printing the matrix \n");
//	i = 0;
//	for (i = 0; i < perm; ++i) {
//		printArray(permutations[i], n);
//	}

	/** MPI CODE **/

	int my_rank; /* rank of process */
	int processes_number; /* number of processes */
	int tag = 0; /* tag for messages */

	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &processes_number);

	i = 0;
	for (; i < perm; ++i) {

		if (my_rank == (processes_number - 1)) {
			Synchronizer_Master(my_rank, i, processes_number - 1, 3);
		} else {
			HS_Standard(my_rank, processes_number - 1, i, permutations[i]);
			//		HS_Optimized(my_rank, processes_number - 1, tag, ids);
			//		Peterson(my_rank, processes_number - 1, tag, ids);
		}
		//usleep(1000000 * 10);
		printf("%d %d \n", i, my_rank);

	}


	/* shut down MPI */
	MPI_Finalize();

	return 0;
}
