
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      1
#define DATATYPE  MPI_DOUBLE
#define ROOT      1
#define REPEAT    1

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int *scnt, *sdpl;
  int dsize;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  sbuf=0; 
  scnt=0; sdpl=0;
  if( myrank==ROOT )
    {
      sbuf=(char*)malloc(SIZE*dsize * ( (nprocs*(nprocs+1))/2+nprocs) );
      scnt=(int*) malloc(sizeof(int)*nprocs);
      sdpl=(int*) malloc(sizeof(int)*nprocs);

      for( i=0; i<nprocs; i++ )
	{
	  scnt[i] = SIZE*(i+1);
	  sdpl[i] = SIZE*(i*((i+1)/2)+i+1);
	}


    }
  rbuf=(char*)malloc(SIZE*dsize * (myrank+1) );

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Scatterv( sbuf, scnt, sdpl, DATATYPE,
		    rbuf, SIZE*(myrank+1), DATATYPE, ROOT, MPI_COMM_WORLD );
      
    }

  fprintf(stdout, "DONE (rank %d)!\n", myrank);
  
  MPI_Finalize();
}
