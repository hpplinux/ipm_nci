#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <mpi.h>


#define DATA_T double
#define MPI_T MPI_DOUBLE
#define INDEX_T long 
#define TRUTH(rank,i,ndata) (DATA_T)(rank*1.0*ndata+i)
#define KB 1024
#define MB 1048576
#define GB 1073741824
#define TB 1099511627776
#define BUFLEN 4096

INDEX_T n,nmin,nmax,nexp;
INDEX_T heap_start,heap_mpi_init;

int getsize_int(char*tag, double x);
int getsize_ext(char*tag, double x);
double wtime(void);

int main(int argc, char **argv)
{
  INDEX_T	     i;
  int                size,rank,left,right;
  int 		     nbar=1;
  double             t0,t1;                
  double	     sum,*sbuf,*rbuf;
  MPI_Status 	     *s;
  MPI_Request        *r;


  heap_start = (long int)sbrk(0);
  t1 = wtime();
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  t1 = wtime();
  heap_mpi_init = (long int)sbrk(0);
  getsize_int("mpi_init",t1-t0);
  getsize_int("heapi",1.0*(heap_mpi_init-heap_start));

   
#ifdef MPI64
  nmin = size;
  nmax =  1*MB/(2*sizeof(DATA_T));
  nexp =  1;
#else
  nmin = size;
  nmax =  1*MB/(2*sizeof(DATA_T));
  nexp =  1;
#endif
  
  if(size%2) {
   printf("must use an even # of MPI tasks\n");
   MPI_Finalize();
   exit(1);
  }

  t0 = MPI_Wtime();
  t1 = MPI_Wtime();
  getsize_int("mpi_wtime",t1-t0);

  s = (MPI_Status *)malloc((size_t)(sizeof(MPI_Status)*size));
  r = (MPI_Request *)malloc((size_t)(sizeof(MPI_Request)*size));

  if(!rank) {
   printf("#n %ld %ld #mpi_size %d #t %.3e \n", nmin, nmax,size,MPI_Wtick());
  }

  if(!rank) {left=size-1;} else { left = rank-1;}
  if(rank == size-1) { right=0;} else {right=rank+1;}
  printf("#t %d <--> %d <--> %d\n",left,rank,right);

  for(n=size; n<=nmax; n+=n/nexp+1) {
   t0 = MPI_Wtime();
   getsize_int("start",t0);

   t0 = MPI_Wtime();
   sbuf = (double *)malloc(n*sizeof(DATA_T));
   rbuf = (double *)malloc(n*sizeof(DATA_T));
   if(!rbuf) {
    printf("%d %d malloc_failed %ld\n",size,rank,n);
    exit(1);
   }
   t1 = MPI_Wtime(); getsize_int("malloc",t1-t0); t0 = MPI_Wtime();

   for(i=0;i<n;i++) sbuf[i] = 1.0;
   for(i=0;i<n;i++) rbuf[i] = 0.0;
   t1 = MPI_Wtime(); getsize_int("2xfill",t1-t0); t0 = MPI_Wtime();

   MPI_Barrier(MPI_COMM_WORLD);
   t1 = MPI_Wtime(); getsize_int("Barrier",t1-t0); t0 = MPI_Wtime();

   MPI_Irecv(rbuf, n, MPI_T, left, 0, MPI_COMM_WORLD, r);
   MPI_Isend(sbuf, n, MPI_T, right, 0, MPI_COMM_WORLD, r+1);
   t1 = MPI_Wtime(); getsize_int("Iexch",t1-t0); t0 = MPI_Wtime();
   printf("RFSI %d %d %d\n", rank, s[0].MPI_SOURCE, s[1].MPI_SOURCE);

   if(rank%2) {
    MPI_Recv(rbuf, n, MPI_T, left, 1, MPI_COMM_WORLD,s+0);
    MPI_Send(sbuf, n, MPI_T, right, 2, MPI_COMM_WORLD);
   } else {
    MPI_Send(sbuf, n, MPI_T, right, 1, MPI_COMM_WORLD);
    MPI_Recv(rbuf, n, MPI_T, left, 2, MPI_COMM_WORLD,s+1);
   }
   t1 = MPI_Wtime(); getsize_int("Bexch",t1-t0); t0 = MPI_Wtime();

   MPI_Bcast(sbuf,n,MPI_T,0,MPI_COMM_WORLD);
   t1 = MPI_Wtime(); getsize_int("Bcast",t1-t0); t0 = MPI_Wtime();

   MPI_Alltoall(sbuf,n/size,MPI_T,rbuf,n/size, MPI_DOUBLE, MPI_COMM_WORLD);
   t1 = MPI_Wtime(); getsize_int("Alltoall",t1-t0); t0 = MPI_Wtime();

   MPI_Allreduce(sbuf,rbuf,n,MPI_T, MPI_SUM, MPI_COMM_WORLD);
   t1 = MPI_Wtime(); getsize_int("Allreduce",t1-t0); t0 = MPI_Wtime();

   MPI_Reduce(sbuf,rbuf,n,MPI_T, MPI_SUM,0, MPI_COMM_WORLD);
   t1 = MPI_Wtime(); getsize_int("Reduce",t1-t0); t0 = MPI_Wtime();

   for(i=0,sum=0.0;i<n;i++) sum+=rbuf[i];
   t1 = MPI_Wtime(); getsize_int("result",sum/(n*size)); t0 = MPI_Wtime();

   free(rbuf);
   free(sbuf);
   t1 = MPI_Wtime(); getsize_int("free",t1-t0); t0 = MPI_Wtime();

   MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;   
}

int getsize_int(char*tag, double x) {
  int                	size,rank;
  INDEX_T		pssz_i, psrss_i;
  DATA_T		rss,rss_tot,sbrksz,sbrksz_tot;
  DATA_T		pssz,pssz_tot,psrss, psrss_tot;
  double	 	xmin=0.0,xmax=0.0,xavg=0.0;
#ifdef AIX
  struct rusage64      	u;
#else
  struct rusage      	u;
#endif
  
  pid_t			mypid;
  FILE			*fp;
  char 			cmd[4096];
  

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifdef AIX
  getrusage64(RUSAGE_SELF, &u); rss=u.ru_maxrss/(1024.0);
#else 
  getrusage(RUSAGE_SELF, &u); rss=u.ru_maxrss/(1024.0);
#endif


/*
   if(rss < 0.0) {
   printf("\n rss<0 \n%ld % .3e %ld\n",rank,rss,u.ru_maxrss);
   exit(1);
  }
*/
  sbrksz = ((long int)sbrk(0) - heap_start)/(1024.0*1024.0);

/* 
 ?  vmstat -a | tail -1 | awk '{print "vmstat", $0}'
*/
  mypid = getpid();
  sprintf(cmd,"ps u %d | tail -1 | awk '{print 1.0*$5, 1.0*$6}'",mypid);
  fp = popen(cmd,"r");
  if(fscanf(fp,"%ld %ld", &pssz_i, &psrss_i) != 2) exit(1); 
  pclose(fp);
  pssz = 1.0*pssz_i/1024.0;
  psrss = 1.0*psrss_i/1024.0;

  MPI_Reduce(&x,&xmin,1,MPI_T,MPI_MIN,0,MPI_COMM_WORLD);
  MPI_Reduce(&x,&xmax,1,MPI_T,MPI_MAX,0,MPI_COMM_WORLD);
  MPI_Reduce(&x,&xavg,1,MPI_T,MPI_SUM,0,MPI_COMM_WORLD); xavg/=size;

  MPI_Reduce(&rss,&rss_tot,1,MPI_T,MPI_SUM,0,MPI_COMM_WORLD); 
  MPI_Reduce(&sbrksz,&sbrksz_tot,1,MPI_T,MPI_SUM,0,MPI_COMM_WORLD); 
  MPI_Reduce(&psrss,&psrss_tot,1,MPI_T,MPI_SUM,0,MPI_COMM_WORLD); 
  MPI_Reduce(&pssz,&pssz_tot,1,MPI_T,MPI_SUM,0,MPI_COMM_WORLD); 

  if(!rank) {
   printf("%d %ld %6.6s %.2e %.2e %.2e  %.2e %.2e %.2e %.2e %.2e\n",
	size,n,tag,
	2.0*size*1.0*sizeof(DATA_T)*1.0*n/(1024.0*1024.0),
        rss_tot, sbrksz_tot, pssz_tot, psrss_tot,
	xmin,xmax,xavg);
   fflush(stdout);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}

#ifdef AIX
double wtime(void) {
   static timebasestruct_t t1={0,0,0};
   static int secs, n_secs;
    
   read_real_time(&t1, TIMEBASE_SZ);
   time_base_to_time(&t1, TIMEBASE_SZ);
   return t1.tb_high + (1.0e-9)*t1.tb_low;
}
#else
double wtime(void) {
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + 1.0e-6*tv.tv_usec;
}
#endif

