
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define COUNT      100
#define DATATYPE   MPI_BYTE
#define REPEAT     1
#define TAG        33


int main( int argc, char* argv[] )
{  
  int i, j;
  int myrank, nprocs;
  char *buf;
  int dsize;
  MPI_Status status;

  int sto; int rfrom;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  sto   = (myrank+1)%nprocs;
  rfrom = (myrank-1+nprocs)%nprocs;

  buf=(char*)malloc(COUNT*dsize );

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Sendrecv_replace( buf, COUNT, DATATYPE, sto, TAG,
			    rfrom, TAG, MPI_COMM_WORLD, &status );
    }

  MPI_Finalize();
}
