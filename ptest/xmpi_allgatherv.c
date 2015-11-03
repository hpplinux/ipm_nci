
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define COUNT     10
#define DATATYPE  MPI_BYTE
#define REPEAT    1

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int *rcnt, *rdpl, rsize;
  int dsize;

  MPI_Init( &argc, &argv );
  
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Type_size(DATATYPE, &dsize);

  rcnt = (int*) malloc(nprocs*(sizeof(int)));
  rdpl = (int*) malloc(nprocs*(sizeof(int)));

  for( i=0; i<nprocs; i++ )
    {
      rcnt[i]=COUNT*(i+1);
      rdpl[i]=COUNT*i*(i+1)/2;
    }

  rsize=0; for( i=0; i<nprocs; i++ ) rsize+=rcnt[i];

  sbuf=(char*)malloc(COUNT*dsize*(myrank+1));
  rbuf=(char*)malloc(COUNT*dsize*rsize);

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Allgatherv( sbuf, COUNT*(myrank+1), DATATYPE,
		      rbuf, rcnt, rdpl, DATATYPE,
		      MPI_COMM_WORLD );
    }

  fprintf(stdout, "DONE (rank %d)!\n", myrank);
  
  MPI_Finalize();
}
