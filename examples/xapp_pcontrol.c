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
  int    size,rank, left, right, you, nmsg=127;
  long long int i,j;
  unsigned long long int  nflop=0,nmem=1,nsleep=0,nrep=1;
  char *env_ptr;
  double *sbuf, *rbuf,*x;
  MPI_Status         *s;
  MPI_Request        *r;


  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  s = (MPI_Status *)malloc((size_t)(sizeof(MPI_Status)*size));
  r = (MPI_Request *)malloc((size_t)(sizeof(MPI_Request)*size));

  sbuf = (double *)malloc(nmsg*sizeof(double));
  rbuf = (double *)malloc(nmsg*sizeof(double));
  for(i=0;i<nmsg;i++) { sbuf[i] = rbuf[i] = i; }

  for(i=0;i<nrep;i++) {
   MPI_Bcast(sbuf,nmsg,MPI_DOUBLE,0,MPI_COMM_WORLD);
  }

  if(size>1) {
  if(!rank) {left=size-1;} else { left = rank-1;}
  if(rank == size-1) { right=0;} else {right=rank+1;}
  you =  (rank < size/2)?(rank+size/2):(rank-size/2);

  for(i=0;i<nrep;i++) {
   MPI_Sendrecv(sbuf,nmsg,MPI_DOUBLE,right,1,rbuf,nmsg,MPI_DOUBLE,left,1,MPI_COMM_WORLD,s);
   MPI_Sendrecv(sbuf,nmsg,MPI_DOUBLE,left,1,rbuf,nmsg,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
  MPI_Pcontrol(1,"region_a"); 
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Sendrecv(sbuf,nmsg,MPI_DOUBLE,left,1,rbuf,nmsg,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
  MPI_Reduce(sbuf,rbuf,nmsg,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,nmsg,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Pcontrol(-1,"region_a"); 

  MPI_Pcontrol(1,"region_b"); 
  MPI_Allreduce(sbuf,rbuf,nmsg,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,nmsg,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,nmsg,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,nmsg-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,nmsg-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Pcontrol(-1,"region_b"); 
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  }
  return 0;   
}


void doflop(int n, double *x) {
 int i;
 double y=0.1;
 
 y = 1.0/n;
 
 for(i=0;i<n;i++) {
  x[i] += y*x[i];
 }
 printf("%.3e \n", x);
 
}
 

