#include <stdio.h>
#include <mpi.h>

int MPI_Barrier(MPI_Comm comm) {
  int rv;
  printf("bingo\n"); fflush(stdout);
  rv = PMPI_Barrier(comm);
  return rv;
 }

