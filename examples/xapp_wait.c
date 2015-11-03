#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char **argv) {

  int    	size=-1,rank=-1, left=-1, right=-1, you=-1;
  int           ndata=127,ndata_max=127,seed;
  int           rv, nsec=0, count, cmpl;
  long long int i,j,k;
  unsigned long long int  nflop=0,nmem=1,nsleep=0,nrep=1, myflops;
  char 		*env_ptr, cbuf[4096];
  double 	*sbuf, *rbuf,*x;
  MPI_Status    *s;
  MPI_Request   *r;
  time_t	ts;


   seed = time(&ts);

  MPI_Init(&argc,&argv);

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
 x = (double *)malloc((size_t)(nmem*sizeof(double)));
 for(i=0;i<nmem;i++) {
  x[i] = i;
 }
 s = (MPI_Status *)malloc((size_t)(sizeof(MPI_Status)*2*size));
 r = (MPI_Request *)malloc((size_t)(sizeof(MPI_Request)*2*size));
 sbuf = (double *)malloc((size_t)(ndata_max*sizeof(double)));
 rbuf = (double *)malloc((size_t)(ndata_max*sizeof(double)));

  if(size>1) {
  if(!rank) {left=size-1;} else { left = rank-1;}
  if(rank == size-1) { right=0;} else {right=rank+1;}
  you =  (rank < size/2)?(rank+size/2):(rank-size/2);
  } else  {
   you = left = right = rank;
  }

  MPI_Irecv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,r);
  MPI_Send(sbuf,ndata,MPI_DOUBLE,you,0,MPI_COMM_WORLD);
  MPI_Wait(r,s);

  MPI_Irecv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,r);
  MPI_Send(sbuf,ndata,MPI_DOUBLE,you,0,MPI_COMM_WORLD);
  MPI_Wait(r,NULL);


  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();

  free((char *)rbuf);
  free((char *)sbuf);
  free((char *)r);
  free((char *)s);

  return 0;   
}


