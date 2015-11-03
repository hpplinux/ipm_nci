
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define COUNT      1
#define DATATYPE   MPI_DOUBLE
#define REPEAT     1
#define OPERATION  MPI_SUM

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int dsize;

  MPI_Init( &argc, &argv );
  
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Type_size(DATATYPE, &dsize);

  sbuf=(char*)malloc(COUNT*dsize);
  rbuf=(char*)malloc(COUNT*dsize);

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Scan( sbuf, rbuf, COUNT, DATATYPE,
		MPI_SUM, MPI_COMM_WORLD );
    }

  fprintf(stdout, "DONE (rank %d)!\n", myrank);
  
  MPI_Finalize();
}
