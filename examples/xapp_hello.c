#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
 int            i,rank,size;
 int		*bogus;


 bogus = (int *)malloc((size_t)(4*sizeof(int)));

 MPI_Init(&argc, &argv);
 for(i=0;i<3;i++) {
 MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 MPI_Comm_size(MPI_COMM_WORLD, &size);
 printf(" Hello world. I am %d of %d tasks in a C MPI program\n", rank, size);
 fflush(stdout);
 MPI_Bcast(bogus,i,MPI_INT,0,MPI_COMM_WORLD);
 MPI_Barrier(MPI_COMM_WORLD);
 }
 MPI_Finalize();
 return 0;
}

