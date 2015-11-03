#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <mpi.h>


double wtime(void);
#define DATA_T double
#define MPI_T MPI_DOUBLE
#define INDEX_T long 
#define TRUTH(rank,i,ndata) (DATA_T)(rank*1.0*ndata+i)
#define KB 1024
#define MB 1048576
#define GB 1073741824
#define TB 1099511627776
#define BUFLEN 4096

#include "../include/ipm.h"
                                                                                
INDEX_T n,nmin,nmax;
INDEX_T heap_start,heap_mpi_init;


int main(int argc, char **argv)
{
  INDEX_T	     i;
  int                size,rank,left,right;
  int 		     *snd_count, *rcv_count, *snd_disp, *rcv_disp;
  int 		     nbar=1,it=0,itmax=256;
  double             t0,t1,t2,t3;
  double	     sum,*sbuf,*rbuf;
  MPI_Status 	     *s;
  MPI_Request        *r;


  heap_start = (long int)sbrk(0);
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  heap_mpi_init = (long int)sbrk(0);

   
#ifdef MPI64
  nmin = size;
  nmax =  16*MB/(2*sizeof(DATA_T));
#else
  nmin = size;
  nmax =  16*MB/(2*sizeof(DATA_T));
#endif
  
  if(size%2) {
   printf("must use an even # of MPI tasks\n");
   MPI_Finalize();
   exit(1);
  }

  s = (MPI_Status *)malloc((size_t)(sizeof(MPI_Status)*size));
  r = (MPI_Request *)malloc((size_t)(sizeof(MPI_Request)*size));

/*
  if(!rank) {
   printf("#n %ld %ld #mpi_size %d #t %.3e \n", nmin, nmax,size,MPI_Wtick());
  }
*/

  if(!rank) {left=size-1;} else { left = rank-1;}
  if(rank == size-1) { right=0;} else {right=rank+1;}
/*
  printf("#t %d <--> %d <--> %d\n",left,rank,right);
*/

  snd_count = (int *)malloc(size*sizeof(int));
  rcv_count = (int *)malloc(size*sizeof(int));
  snd_disp = (int *)malloc(size*sizeof(int));
  rcv_disp = (int *)malloc(size*sizeof(int));

  for(n=size; n<=nmax; n+=size) {

   it ++; 
   if(it > itmax) break;

   sbuf = (double *)malloc(n*sizeof(DATA_T));
   rbuf = (double *)malloc(n*sizeof(DATA_T));
   if(!rbuf) {
    printf("%d %d malloc_failed %ld\n",size,rank,n);
    exit(1);
   }

   for(i=0;i<n;i++) sbuf[i] = 1.0;
   for(i=0;i<n;i++) rbuf[i] = 0.0;

   for(i=0;i<size;i++) snd_count[i] = n/size;
   for(i=0;i<size;i++) rcv_count[i] = n/size;
   for(i=0;i<size;i++) snd_disp[i] = i*n/size;
   for(i=0;i<size;i++) rcv_disp[i] = i*n/size;

   

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   MPI_Barrier(MPI_COMM_WORLD);
   PMPI_Barrier(MPI_COMM_WORLD);
   t1 = wtime();
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   printf("Barrier %d %lld %.3e %.3e\n",rank, (long long int )(n),t1-t0,t2-t1);

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   MPI_Irecv(rbuf, n, MPI_T, left, 0, MPI_COMM_WORLD, r);
   MPI_Isend(sbuf, n, MPI_T, right, 0, MPI_COMM_WORLD, r+1);
   MPI_Wait(r,s);
   t1 = wtime();
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   PMPI_Irecv(rbuf, n, MPI_T, left, 0, MPI_COMM_WORLD, r);
   PMPI_Isend(sbuf, n, MPI_T, right, 0, MPI_COMM_WORLD, r+1);
   MPI_Wait(r,s);
   t3 = wtime();
   printf("Iexch %d %lld %.3e %.3e\n",rank, (long long int)(n),t1-t0,t3-t2);

/*
   printf("RFSI %d %d %d\n", rank, s[0].MPI_SOURCE, s[1].MPI_SOURCE);
*/

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   if(rank%2) {
    MPI_Recv(rbuf, n, MPI_T, left, 1, MPI_COMM_WORLD,s+0);
    MPI_Send(sbuf, n, MPI_T, right, 2, MPI_COMM_WORLD);
   } else {
    MPI_Send(sbuf, n, MPI_T, right, 1, MPI_COMM_WORLD);
    MPI_Recv(rbuf, n, MPI_T, left, 2, MPI_COMM_WORLD,s+1);
   }
   t1 = wtime();
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   if(rank%2) {
    PMPI_Recv(rbuf, n, MPI_T, left, 1, MPI_COMM_WORLD,s+0);
    PMPI_Send(sbuf, n, MPI_T, right, 2, MPI_COMM_WORLD);
   } else {
    PMPI_Send(sbuf, n, MPI_T, right, 1, MPI_COMM_WORLD);
    PMPI_Recv(rbuf, n, MPI_T, left, 2, MPI_COMM_WORLD,s+1);
   }
   t3 = wtime();
   printf("Bexch %d %lld %.3e %.3e\n",rank, (long long int)(n),t1-t0,t3-t2);

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   MPI_Bcast(sbuf,n,MPI_T,0,MPI_COMM_WORLD);
   t1 = wtime();
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   PMPI_Bcast(sbuf,n,MPI_T,0,MPI_COMM_WORLD);
   t3 = wtime();
   printf("Bcast %d %lld %.3e %.3e\n",rank, (long long int)(n),t1-t0,t3-t2);

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   MPI_Alltoall(sbuf,n/size,MPI_T,rbuf,n/size, MPI_DOUBLE, MPI_COMM_WORLD);
   t1 = wtime(); 
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   PMPI_Alltoall(sbuf,n/size,MPI_T,rbuf,n/size, MPI_DOUBLE, MPI_COMM_WORLD);
   t3 = wtime();
   printf("Alltoall %d %lld %.3e %.3e\n",rank, (long long int)(n), t1-t0,t3-t2);

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   MPI_Alltoallv(sbuf,snd_count,snd_disp,MPI_T,rbuf,rcv_count,rcv_disp,MPI_DOUBLE, MPI_COMM_WORLD);
   t1 = wtime(); 
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   PMPI_Alltoallv(sbuf,snd_count,snd_disp,MPI_T,rbuf,rcv_count,rcv_disp,MPI_DOUBLE, MPI_COMM_WORLD);
   t3 = wtime();
   printf("Alltoallv %d %lld %.3e %.3e\n",rank, (long long int)(n), t1-t0,t3-t2);

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   MPI_Allreduce(sbuf,rbuf,n,MPI_T, MPI_SUM, MPI_COMM_WORLD);
   t1 = wtime(); 
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   PMPI_Allreduce(sbuf,rbuf,n,MPI_T, MPI_SUM, MPI_COMM_WORLD);
   t3 = wtime();
   printf("Allreduce %d %lld %.3e %.3e\n",rank, (long long int)(n),t1-t0,t3-t2);

   PMPI_Barrier(MPI_COMM_WORLD);
   t0 = wtime();
   MPI_Reduce(sbuf,rbuf,n,MPI_T, MPI_SUM,0, MPI_COMM_WORLD);
   t1 = wtime();
   PMPI_Barrier(MPI_COMM_WORLD);
   t2 = wtime();
   PMPI_Reduce(sbuf,rbuf,n,MPI_T, MPI_SUM,0, MPI_COMM_WORLD);
   t3 = wtime();
   printf("Reduce %d %lld %.3e %.3e\n",rank, (long long int)(n),t1-t0,t3-t2);

   free(rbuf);
   free(sbuf);

   MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;   
}

double wtime(void) {
 IPM_TICK_TYPE t;

 IPM_TIME_GET(t);
 return IPM_TIME_SEC(t); 
}

