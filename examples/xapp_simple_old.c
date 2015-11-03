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
#ifdef IPM
#include <ipm.h>
#endif
#ifdef HPM
#include <papi.h>

long long int values[2];
char errstring[PAPI_MAX_STR_LEN];
int events[2] = {PAPI_TOT_INS, PAPI_TOT_CYC};
int num_hwcntrs=0;
#endif

unsigned long long int flags=0;

 
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

  int    	size,rank, left, right, you, ndata=127,ndata_max=127,seed;
  int           rv;
  long long int i,j,k;
  unsigned long long int  nflop=0,nmem=1,nsleep=0,nrep=1, myflops;
  char 		*env_ptr;
  double 	*sbuf, *rbuf,*x;
  MPI_Status    *s;
  MPI_Request   *r;
  time_t	ts;


#ifdef HPM

   if((rv = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
   {
      fprintf(stderr, "Error: %d %s\n",rv, errstring);
      exit(1);
   }

   if ((num_hwcntrs = PAPI_num_counters()) < PAPI_OK)
   {
      printf("There are no counters available. \n");
      exit(1);
   }

  if ( (rv = PAPI_start_counters(events, 2)) != PAPI_OK) {
    fprintf(stdout, "ERROR PAPI_start_counters rv=%d\n", rv);
    exit(rv);
   }

#endif
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

/*
  MPI_Init(&argc,&argv);
*/
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
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
  printf("trickster\n");
 }
 }
 if(0) free((char *)x);
}
 
#ifdef IPM
  if(flags & REGION && rank > -1 ) MPI_Pcontrol(1,"region_zzzzzzzzzzzZz"); 
#endif
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
#ifdef IPM
  if(flags & REGION && rank > -1 ) MPI_Pcontrol(-1,"region_zzzzzzzzzzzZz"); 
#endif
 
 if(nmem<nflop) nmem=nflop;
 
 if(nflop>1) printf("FLOPS = %lld BYTES = %lld\n", nflop, nmem);
 
 fflush(stdout);
 
 if(flags & CORE) {
  for(i=0;;i++) {
   x[i] = x[i*i-1000];
  }
 }



  env_ptr = getenv("IPM_SOCKET");
  if(env_ptr) {
   printf("IPM: %d IPM_SOCKET in app %s\n", rank, env_ptr);
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


  for(i=0;i<nrep;i++) {
   if(flags & DOSPRAY) {
    ndata = (long int)(drand48()*ndata_max)+1;
   }
   MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,right,1,rbuf,ndata,MPI_DOUBLE,left,1,MPI_COMM_WORLD,s);
   MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,left,1,rbuf,ndata,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_a"); 
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Sendrecv(sbuf,ndata,MPI_DOUBLE,left,1,rbuf,ndata,MPI_DOUBLE,right,1,MPI_COMM_WORLD,s);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);

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

#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_a"); 
#endif

#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_b"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_b"); 
#endif

 if(1) {
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_c"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_c"); 
#endif
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_d"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_d"); 
#endif
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_e"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_e"); 
#endif
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_f"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_f"); 
#endif
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_g"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_g"); 
#endif
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_h"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_h"); 
#endif
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(1,"region_i"); 
#endif
  MPI_Allreduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 0,  MPI_COMM_WORLD);
  MPI_Reduce(sbuf,rbuf,ndata-1,MPI_DOUBLE, MPI_SUM, 1,  MPI_COMM_WORLD);
#ifdef IPM
  if(flags & REGION) MPI_Pcontrol(-1,"region_i"); 
#endif
 }

  }
  }


  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  }

#ifdef HPM
   if ((rv=PAPI_stop_counters(values, 2)) != PAPI_OK) {
    fprintf(stdout, "ERROR PAPI_stop_counters rv=%d\n", rv);
    exit(rv);
   }
   printf("PAPI: total instruction/cycles  %lld/%lld %.3e \n", values[0], values[1], values[0]/(values[1]*1.0) );
#endif 

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
 

