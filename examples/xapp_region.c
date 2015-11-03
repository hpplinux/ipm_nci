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

void doflop(int n,double *x);


int main(int argc, char **argv) {

  int    	size,rank, left, right, you, ndata=127,ndata_max=127,seed;
  int           rv, nsec=0, ireg, nreg=255;
  long long int i,j,k;
  unsigned long long int  nflop=0,nmem=1,nsleep=0,nrep=1;
  char 		*env_ptr;
  double 	*sbuf, *rbuf,*x;
  MPI_Status    *s;
  MPI_Request   *r;
  time_t	ts;
  char		reg_name[21], pcmd[80];


  while(--argc && argv++) {
   if(!strcmp("-r",*argv)) {
    --argc; argv++;
    nreg = atol(*argv);
   }
 }
 
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
 for(ireg = 0; ireg < nreg ; ireg++) {

  nrep = ireg;
  sprintf(pcmd,"enter_region(abcdefghijklmnop%4.4d)", ireg);
  MPI_Pcontrol(0,pcmd);

 if(nsec > 0) {
  sleep(nsec);
 }

 if(nflop) {
  x = (double *)malloc((size_t)(10*sizeof(double)));
  j = k = 0;
  for(i=0;i<10;i++) {
   x[i] = 1.0;
  }

  for(i=0;i<nflop;i++) {
   x[j] = x[j]*x[k];
   j = ((i%9)?(j+1):(0));
   k = ((i%8)?(k+1):(0));
  }
  free((char *)x);
 }

 if(nsleep) {
  sleep(nsleep);
 }
 
 if(nmem<nflop) nmem=nflop;
 
 if(nflop>1) printf("FLOPS = %lld BYTES = %lld\n", nflop, nmem);
 
 fflush(stdout);
 
  s = (MPI_Status *)malloc((size_t)(sizeof(MPI_Status)*2*size));
  r = (MPI_Request *)malloc((size_t)(sizeof(MPI_Request)*2*size));


  sbuf = (double *)malloc((size_t)(ndata_max*sizeof(double)));
  rbuf = (double *)malloc((size_t)(ndata_max*sizeof(double)));
  for(i=0;i<ndata_max;i++) { sbuf[i] = rbuf[i] = i; }

  MPI_Bcast(&seed,1,MPI_INT,0,MPI_COMM_WORLD);
  srand48(seed);

  for(i=0;i<nrep;i++) {
   MPI_Bcast(sbuf,ndata_max,MPI_DOUBLE,0,MPI_COMM_WORLD);
  }

  if(size>1) {
  if(!rank) {left=size-1;} else { left = rank-1;}
  if(rank == size-1) { right=0;} else {right=rank+1;}
  you =  (rank < size/2)?(rank+size/2):(rank-size/2);
  } else  {
   you = left = right = rank;
  }
 


  for(i=0;i<nrep;i++) {
   ndata = (long int)(drand48()*ndata_max)+1;
   MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,right,1,rbuf,ndata,MPI_DOUBLE,left,1,MPI_COMM_WORLD,s);
   MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,left,1,rbuf,ndata,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,left,1,rbuf,ndata,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);

  MPI_Isend(sbuf,ndata,MPI_DOUBLE,you,0,MPI_COMM_WORLD, r);
  MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD, s);
  MPI_Wait(r,s);

  MPI_Irecv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,r);
  MPI_Send(sbuf,ndata,MPI_DOUBLE,you,0,MPI_COMM_WORLD);
  MPI_Wait(r,s);

  
  for(j=0;j<size;j++) {
   MPI_Isend(sbuf+j%ndata_max,1,MPI_DOUBLE,j,4,MPI_COMM_WORLD, r+j);
   MPI_Irecv(rbuf+j%ndata_max,1,MPI_DOUBLE,j,4,MPI_COMM_WORLD,r+size+j);
  }
  MPI_Waitall(2*size,r,s);
/*
  for(j=0;j<size;j++) {
   printf("rep %d stat %d %d %d\n",i, j, s[j].MPI_SOURCE, s[j+size].MPI_SOURCE);
  }
*/

  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
 }



  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Pcontrol(0,"get_region()",reg_name);
  printf("%d in  region = %s\n", rank,reg_name);
  sprintf(pcmd,"exit_region(abcdefghijklmnop%4.4d)", ireg);
  MPI_Pcontrol(0,pcmd);
 }


  MPI_Finalize();
  return 0;   
}


void doflop(int n, double *x) {
 int i;
 double y=0.1;
 
 y = 1.0/n;
 
 for(i=0;i<n;i++) {
  x[i] += y*x[i];
 }
 printf("%.3e \n", *x);
 
}
 

