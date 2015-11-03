#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <mpi.h>


int main(int argc, char **argv)
{
  int    i,size,rank, left, right, you,n=127;
  double *sbuf, *rbuf;
  MPI_Status         *s;
  MPI_Request        *r;



  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  
  s = (MPI_Status *)malloc((size_t)(sizeof(MPI_Status)*size));
  r = (MPI_Request *)malloc((size_t)(sizeof(MPI_Request)*size));


  sbuf = (double *)malloc(n*sizeof(double));
  rbuf = (double *)malloc(n*sizeof(double));
  for(i=0;i<n;i++) { sbuf[i] = rbuf[i] = i; }

 
  if(!rank) {left=size-1;} else { left = rank-1;}
  if(rank == size-1) { right=0;} else {right=rank+1;}
  you =  (rank < size/2)?(rank+size/2):(rank-size/2);

  printf("app :: %d %d\n", rank, size);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Sendrecv(sbuf,n,MPI_DOUBLE,left,1,rbuf,n,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);

  MPI_Allreduce(sbuf,rbuf,n,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);


  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  return 0;   
}

