
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define SIZE       100
#define DATATYPE   MPI_BYTE
#define REPEAT     1

#define SEND         0
#define ISEND        1
#define BSEND        2
#define RSEND        3
#define SSEND        4
#define IBSEND       5
#define IRSEND       6
#define ISSEND       7
#define RECV         8
#define IRECV        9


void sendandrecv( int stype, int rtype )
{
  int i;
  int myrank, nprocs;
  MPI_Status status;
  MPI_Request request;
  int dsize; 

  char *sbuf=0;
  char *rbuf=0;
  char *msgbuf;

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  if( nprocs%2 )
    {
      fprintf(stderr, "Err: must have even number of processes\n");
      MPI_Finalize();
      exit(1);
    }

  if( stype==BSEND ) {
    msgbuf = (char*) malloc(dsize*SIZE+MPI_BSEND_OVERHEAD);
    MPI_Buffer_attach( msgbuf, dsize*SIZE+MPI_BSEND_OVERHEAD );
  }

  if( myrank%2 )
    {
      rbuf = (char*) malloc( dsize*SIZE );
      
      for( i=0; i<REPEAT; i++ )
	{
	  switch( rtype )
	    {
	    case RECV:
	      if( stype==RSEND || stype==IRSEND )
		MPI_Barrier(MPI_COMM_WORLD); 
	      MPI_Recv( rbuf, SIZE, DATATYPE, myrank-1, 33, MPI_COMM_WORLD, &status );

	      break;
	    case IRECV:
	      if( stype==RSEND || stype==IRSEND )
		MPI_Barrier(MPI_COMM_WORLD); 

	      MPI_Irecv(  rbuf, SIZE, DATATYPE, myrank-1, 33, MPI_COMM_WORLD, &request );
	      sleep(1);
	      MPI_Wait( &request, &status );
	      break;
	    }
	}
    }
  else
    {
      sbuf = (char*) malloc( dsize*SIZE );
      
      for( i=0; i<REPEAT; i++ )
	{
	  switch( stype )
	    {
	    case SEND:
	      MPI_Send( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD );
	      break;

	    case SSEND:
	      MPI_Ssend( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD );
	      break;	      

	    case ISEND:
	      MPI_Isend( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD, &request );
	      sleep(1);
	      MPI_Wait( &request, &status );
	      break;
	      
	    case BSEND:
	      MPI_Bsend( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD );
	      break;

	    case RSEND:
	      // sync with receivers with a barrier
	      MPI_Barrier(MPI_COMM_WORLD); 
	      sleep(1);
	      MPI_Rsend( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD );
	      break;

	    case IBSEND:
	      MPI_Ibsend( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD, &request );
	      sleep(1);
	      MPI_Wait( &request, &status );
	      break;
	      
	    case IRSEND:
	      // sync with receivers with a barrier
	      MPI_Barrier(MPI_COMM_WORLD); 
	      sleep(1);
	      MPI_Irsend( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD, &request );
	      sleep(1);
	      MPI_Wait( &request, &status );	      
	      break;
	      
	    case ISSEND:
	      MPI_Issend( sbuf, SIZE, DATATYPE, myrank+1, 33, MPI_COMM_WORLD, &request );
	      sleep(1);
	      MPI_Wait( &request, &status );	      
	      break;
	    }
	}
    }
}



int main( int argc, char* argv[] )
{
  int send, recv;

  MPI_Init( &argc, &argv );
  
  send=SEND;
  if( argc>1 && !strcmp(argv[1],  "ISEND") ) send =  ISEND;
  if( argc>1 && !strcmp(argv[1],  "BSEND") ) send =  BSEND;
  if( argc>1 && !strcmp(argv[1],  "RSEND") ) send =  RSEND;
  if( argc>1 && !strcmp(argv[1],  "SSEND") ) send =  SSEND;
  if( argc>1 && !strcmp(argv[1], "IBSEND") ) send = IBSEND;
  if( argc>1 && !strcmp(argv[1], "IRSEND") ) send = IRSEND;
  if( argc>1 && !strcmp(argv[1], "ISSEND") ) send = ISSEND;
  
  recv=RECV;
  if( argc>2 && !strcmp(argv[2],  "IRECV") ) recv = IRECV;

  sendandrecv( send, recv);

  MPI_Finalize();
}
