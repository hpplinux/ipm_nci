
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define COUNT     1
#define DATATYPE  MPI_BYTE
#define REPEAT    1

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int *rcnt;
  int dsize;
  int dcnt;

  MPI_Init( &argc, &argv );
  
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  rcnt = (int*) malloc( sizeof(int)*nprocs );
  for( i=0; i<nprocs; i++ )
    {
      rcnt[i]=(i+1)*COUNT;
    }

  dcnt = 0; for( i=0; i<nprocs; i++ ) { dcnt+=rcnt[i]; }

  sbuf = (char*) malloc( dsize*dcnt );
  rbuf = (char*) malloc( dsize*rcnt[i] );

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Reduce_scatter( sbuf, rbuf, rcnt, DATATYPE,
			      MPI_BXOR, MPI_COMM_WORLD );
    } 
    
  fprintf(stdout, "DONE (rank %d)!\n", myrank);
  
  MPI_Finalize();
}
