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

#define COMM MPI_COMM_WORLD

#define DOVERBOSE         (1<<0)
#define DOPT2PT           (1<<1)
#define DOCOLLECTIVE      (1<<2)
#define DOMINIT           (1<<3)
#define DOSMODER          (1<<4)
#define DOSMODES          (1<<5)
#define DOSMODEB          (1<<6)

unsigned long long flags = 0;

int main(int argc, char **argv) {

  int    	size,rank, me, you, left, right, ndata;
  int           rv,mdata;
  double 	*sbuf, *rbuf, *mbuf, *x;
  MPI_Status    *s;
  MPI_Request   *r;
  time_t	ts;


  while(--argc && argv++) {
   if(!strcmp("-v",*argv)) {
    flags |= DOVERBOSE;
   } else if(!strcmp("-p",*argv)) {
     flags |= DOPT2PT;
   } else if(!strcmp("-c",*argv)) {
     flags |= DOCOLLECTIVE;
   } else if(!strcmp("-i",*argv)) {
     flags |= DOMINIT;
   }
  }

  MPI_Init(&argc,&argv);
  if(flags & DOMINIT) {
   MPI_Init(&argc,&argv);
  }
  MPI_Comm_size(COMM, &size);
  MPI_Comm_rank(COMM, &rank);

  ndata = size;
  mdata = ndata;
  s = (MPI_Status *)malloc((size_t)(sizeof(MPI_Status)*2*size));
  r = (MPI_Request *)malloc((size_t)(sizeof(MPI_Request)*2*size));
  sbuf = (double *)malloc((size_t)(ndata*sizeof(double)));
  rbuf = (double *)malloc((size_t)(ndata*sizeof(double)));
  mbuf = (double *)malloc((size_t)(ndata*sizeof(double)));

  me = rank;
  if(size>1) {
  if(!rank) {left=size-1;} else { left = rank-1;}
  if(rank == size-1) { right=0;} else {right=rank+1;}
  you =  (rank < size/2)?(rank+size/2):(rank-size/2);
  } else  {
   you = left = right = rank;
  }
 

 if(flags & DOVERBOSE) {
  printf("topo: rank=%d you=%d left=%d right=%d\n", rank, you, left, right);
 }

 if(flags & DOCOLLECTIVE) {
  MPI_Barrier(COMM);
  MPI_Bcast(&sbuf,ndata,MPI_DOUBLE,0,COMM);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  COMM);
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, COMM);
  MPI_Alltoall(sbuf,1,MPI_DOUBLE,rbuf,1,MPI_DOUBLE,COMM);
 }

 if(flags & DOPT2PT) {
  MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,right,1,rbuf,ndata,MPI_DOUBLE,left,1,COMM,s);

  if(me < you) {
   MPI_Send(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
  } else {
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
   MPI_Send(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
  }

  if(me < you) {
   MPI_Ssend(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
  } else {
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
   MPI_Ssend(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
  }

  if(me < you) {
   sleep(1); /* ugh */
   MPI_Rsend(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
  } else {
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
   sleep(1); /* ugh */
   MPI_Rsend(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
  }

/*
  MPI_Buffer_attach(mbuf, mdata+MPI_BSEND_OVERHEAD);
  if(me < you) {
   MPI_Bsend(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
  } else {
   MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,COMM, s);
   MPI_Bsend(sbuf,ndata,MPI_DOUBLE,you,0,COMM);
  }
  MPI_Buffer_detach(mbuf,&mdata);
*/

  MPI_Isend(sbuf, ndata, MPI_DOUBLE, you, 0, COMM, r);
  MPI_Irecv(rbuf, ndata, MPI_DOUBLE, you, 0, COMM, r+1);
  MPI_Waitall(2,r,s);

 

 }

  MPI_Finalize();
  return 0;
}

