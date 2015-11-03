
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      100
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

  sbuf=0;
  if( myrank==ROOT )
    {
      sbuf=(char*)malloc(SIZE*nprocs*dsize);
    }
  rbuf=(char*)malloc(SIZE*dsize);

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Scatter( sbuf, SIZE, DATATYPE,
		   rbuf, SIZE, DATATYPE, ROOT, MPI_COMM_WORLD );
    }

  fprintf(stdout, "DONE (rank %d)!\n", myrank);
  
  MPI_Finalize();
}
