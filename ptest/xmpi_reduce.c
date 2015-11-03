
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define COUNT     100
#define DATATYPE  MPI_BYTE
#define ROOT      0
#define REPEAT    1


int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  char *svbuf, *rvbuf;
  int *scnt, *sdpl;
  int dsize;

  MPI_Init( &argc, &argv );
  
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  rbuf=0;
  if( myrank==ROOT )
    {
      rbuf=(char*)malloc(COUNT*dsize);
    }
  sbuf=(char*)malloc(COUNT*dsize);

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Reduce( sbuf, rbuf, COUNT, DATATYPE,
		  MPI_BXOR, ROOT, MPI_COMM_WORLD );
    }

  fprintf(stdout, "DONE (rank %d)!\n", myrank);
  
  MPI_Finalize();
}
