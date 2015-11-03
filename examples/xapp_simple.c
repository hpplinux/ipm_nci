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

unsigned long long int flags=0;
char msg[80];

 
#define CORE    	(1<<1)
#define REGION  	(1<<2)
#define SLEEP   	(1<<3)
#define DOMPI   	(1<<4)
#define DOVERBOSE 	(1<<5)
#define DOSPRAY   	(1<<6)
#define STAIR_RANK 	(1<<10)
#define STAIR_REGION 	(1<<11)
 
void doflop(int n,double *x);


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

   flags |= DOMPI;
   while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
    flags |= DOVERBOSE;
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    nflop = atol(*argv);
  } else if(!strcmp("-N",*argv)) {
    --argc; argv++;
    nrep = atol(*argv);
  } else if(!strcmp("-d",*argv)) {
    --argc; argv++;
    ndata_max = ndata = atol(*argv);
  } else if(!strcmp("-m",*argv)) {
    --argc; argv++;
    nmem = atol(*argv);
  } else if(!strcmp("-w",*argv)) {
    --argc; argv++;
    nsec = atoi(*argv);
  } else if(!strcmp("-s",*argv)) {
    --argc; argv++;
    nsleep = atol(*argv);
  } else if(!strcmp("-spray",*argv)) {
    flags |= DOSPRAY;
  } else if(!strcmp("-c",*argv)) {
    flags |= CORE;
  } else if(!strcmp("-r",*argv)) {
    flags |= REGION;
  } else if(!strcmp("-stair",*argv)) {
    flags |= STAIR_RANK;
  } else if(!strcmp("-stair_region",*argv)) {
    flags |= STAIR_REGION;
  } else if(!strcmp("-nompi",*argv)) {
    flags &= ~DOMPI;
  }
 }
 
 if(flags & DOMPI) {
  MPI_Init(&argc,&argv);

/* test double init 
  MPI_Init(&argc,&argv);
*/
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 }
 
 if(nsec > 0) {
  sleep(nsec);
 }

 if(nmem) {
 nmem = (nmem*1024*1024/sizeof(double));
 x = (double *)malloc((size_t)(nmem*sizeof(double)));
 for(j=0;j<nrep;j++) {
 for(i=0;i<nmem;i++) {
  x[i] = i;
 }
 for(i=0;i<nmem;i++) {
  x[i] = i*x[i];
 }
 if(x[nmem-1]*x[nmem-1] < 0) {
  printf("assumption about flop-test or optimization failed\n");
 }
 }
 if(0) free((char *)x);
}
 
/*
#define LONG_REGNAME rshouldbethelastchar
*/
#define LONG_REGNAME abcdefghijklmnopqrst

 if(flags & REGION) {
  MPI_Pcontrol(0,"enter_region(abcdefghijklmnopqrst)");
  sprintf(cbuf,"");
  MPI_Pcontrol(0,"get_region()",cbuf);
  if(strcmp(cbuf,"abcdefghijklmnopqrst")) {
   printf("%d in region = \"%s\" not \"%s\"\n",
	 rank,cbuf,"abcdefghijklmnopqrst");
   fflush(stdout);
  }
  MPI_Pcontrol(0,"exit_region(abcdefghijklmnopqrst)");
  MPI_Pcontrol(0,"get_region()",cbuf);
  if(strcmp(cbuf,"ipm_noregion")) {
   printf("%d out region = \"%s\" not \"%s\"\n",
	 rank,cbuf,"ipm_noregion");
   fflush(stdout);
  }
 }
  
  if(flags & REGION && rank > -1 ) MPI_Pcontrol(1,"region_zzzzzzzzzzzZz"); 
 if(nflop) {
  x = (double *)malloc((size_t)(10*sizeof(double)));
  j = k = 0;
  for(i=0;i<10;i++) {
   x[i] = 1.0;
  }
if(flags & STAIR_RANK) { 
 myflops = (rank*nflop)/size;
} else {
 myflops = nflop;
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
  if(flags & REGION && rank > -1 ) MPI_Pcontrol(-1,"region_zzzzzzzzzzzZz"); 
 
 if(nmem<nflop) nmem=nflop;
 
 if(nflop>1) printf("FLOPS = %lld BYTES = %lld\n", nflop, nmem);
 
 fflush(stdout);
 
 if(flags & CORE) {
  for(i=0;;i++) {
   x[i] = x[i*i-1000];
  }
 }

 if(flags & DOMPI) {
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
   if(flags & DOSPRAY) {
    ndata = (long int)(drand48()*ndata_max)+1;
   }
   MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,right,1,rbuf,ndata,MPI_DOUBLE,left,1,MPI_COMM_WORLD,s);
   MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,left,1,rbuf,ndata,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
  if(flags & REGION) MPI_Pcontrol(1,"region_a"); 
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,left,1,rbuf,ndata,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
  MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,left,1,rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,s);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);

  MPI_Isend(sbuf,ndata/2,MPI_DOUBLE,you,0,MPI_COMM_WORLD, r);
  MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &cmpl, s);
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, s);
  MPI_Get_count(s,MPI_DOUBLE,&count);
  MPI_Recv(rbuf,ndata,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD, s);
  if(count != ndata/2) {
  printf("error: MPI_Get_count(s,MPI_DOUBLE,&count) --> count = %d\n",count);
  }
  MPI_Wait(r,s);
/* FIXME - the following case may need to be addressed
  MPI_Test(r,&cmpl,s);
  printf("spam1 %d %d\n", s->MPI_SOURCE, cmpl);
  if(r != MPI_REQUEST_NULL) {
   MPI_Wait(r,s);
   printf("spam2 %d\n", s->MPI_SOURCE);
  }
*/

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

  if(flags & REGION) MPI_Pcontrol(-1,"region_a"); 

  if(flags & REGION) MPI_Pcontrol(1,"region_b"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_b"); 

 if(1) {
  if(flags & REGION) MPI_Pcontrol(1,"region_c"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_c"); 
  if(flags & REGION) MPI_Pcontrol(1,"region_d"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_d"); 
  if(flags & REGION) MPI_Pcontrol(1,"region_e"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_e"); 
  if(flags & REGION) MPI_Pcontrol(1,"region_f"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_f"); 
  if(flags & REGION) MPI_Pcontrol(1,"region_g"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_g"); 
  if(flags & REGION) MPI_Pcontrol(1,"region_h"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_h"); 
  if(flags & REGION) MPI_Pcontrol(1,"region_i"); 
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  if(flags & REGION) MPI_Pcontrol(-1,"region_i"); 
 }


  }


  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  }

  free((char *)rbuf);
  free((char *)sbuf);
  free((char *)r);
  free((char *)s);

  free((char *)x);

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
 

