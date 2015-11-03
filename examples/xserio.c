#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <math.h>
#ifdef MPI
#include <mpi.h>
#endif
#ifdef AIX
#include <gpfs.h>
#endif


#define DATA_T double
#define MPI_T MPI_DOUBLE
#define INDEX_T long  long int
#define OFF_T off_t
#define TRUTH(i) (DATA_T)i
#define BUFLEN 4096
#define FMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#define GPFS_PREALLOC(fd,bytes) {\
	gpfs_prealloc(fd, 0, bytes); \
        }

#ifdef MPI
#define BARRIER MPI_Barrier(MPI_COMM_WORLD);
#else 
#define BARRIER
#endif

#define HARNESS(tag,a) {\
  BARRIER;\
  gettimeofday(&tv,NULL); t0 = tv.tv_sec + (1.0e-6)*tv.tv_usec;\
  a;\
  gettimeofday(&tv,NULL); t1 = tv.tv_sec + (1.0e-6)*tv.tv_usec;\
  summary(rank,tag,ndata,nbyte,t1,t0);\
 }\
  

#define VERIFY(tag) {\
  for(i=0;i<ndata;i++) if(data[i] != TRUTH(i)) \
  	{printf("data integrity:: %s (%lld)\n",tag,nbyte); exit(1); } \
  }

#define KB 1024
#define MB 1048576
#define GB 1073741824
#define TB 1099511627776

extern void fort_write_da_(int *size, int *rank, INDEX_T *ndata, DATA_T *data);
extern void fort_read_da_(int *size, int *rank, INDEX_T *ndata, DATA_T *data);
int summary(int rank, char *tag, INDEX_T ndata, INDEX_T nbyte, double t1, double t0);
int rank_dir(int action ,int rank);
int get_stat(double x, double *xmin, double *xmax, double *xavg, double *xcov);


int main (int argc, char *argv[]) {
 int 		fd,size,rank,idaho=1;
 FILE 		*file;
 INDEX_T	i,ndata,ndata_min,ndata_max;
 size_t		nbyte;
 DATA_T		*data;
 double		t0,t1;
 char		fname[BUFLEN],ffname[BUFLEN];
 struct timeval tv;

#ifdef MPI
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else 
  size = 1;
  rank = 0;
#endif

 ndata_min = 1*MB/sizeof(DATA_T);
 ndata_max = 2*MB/sizeof(DATA_T);

 while(--argc && argv++) {
  if(!strcmp("-n",*argv)) {
    --argc; argv++;
    ndata_min = atoi(*argv);
    ndata_max = ndata_min;
  } else if(!strcmp("-min",*argv)) {
    --argc; argv++;
    ndata_min = atoi(*argv);
  } else if(!strcmp("-max",*argv)) {
    --argc; argv++;
    ndata_max = atoi(*argv);
  }
 }


 for(ndata=ndata_min;ndata<=ndata_max;ndata+=((ndata/4>0)?(ndata/2):(1))) { 

  nbyte = ndata*sizeof(DATA_T);
  data = (DATA_T *)malloc(nbyte);
  for(i=0;i<ndata;i++) data[i] = TRUTH(i);
  sprintf(fname,"tmpdata_%d_%d_%lld",size,rank,ndata);
  sprintf(ffname,"fort.%d",rank+11);

/*******************************************************/
/*******************************************************/
/*******************************************************/

  if(idaho) {
   HARNESS("priv_mdir", 
    rank_dir(1,rank);
   );
  }

/* fwrite */
  HARNESS ("fwrite3", 
   file = fopen(fname,"w");
   fwrite(data,nbyte,1,file);
   fclose(file);
  );


/* fread */
  HARNESS("fread3", 
   file = fopen(fname,"w");
   fread(data,nbyte,1,file);
   fclose(file);
  );
  VERIFY("fread3");
  HARNESS("unlink3", 
   unlink(fname);
  );

/* write */
  HARNESS("write2", 
   fd = open(fname,O_CREAT|O_WRONLY,FMODE);
   write(fd,data,nbyte);
   close(fd);
  );

/* read */
  HARNESS("read2", 
   fd = open(fname,O_RDONLY,FMODE);
   read(fd,data,nbyte);
   close(fd);
  );
  VERIFY("read2");
  HARNESS("unlink2", 
   unlink(fname);
  );


/* fort write - direct access */
  HARNESS("write_uf", 
   fort_write_da_(&size,&rank,&ndata,data);
  );

/* fort read - direct access */
  HARNESS("read_uf", 
   fort_read_da_(&size,&rank,&ndata,data);
  );
  VERIFY("read_uf");
  HARNESS("unlinkf", 
   unlink(ffname);
  );

  if(idaho) {
   HARNESS("priv_rdir", 
    rank_dir(-1,rank);
   );
  }

/*******************************************************/
/*******************************************************/
/*******************************************************/

  if(!rank) {

  }

  free(data);
 }

#ifdef MPI
  MPI_Finalize();
#endif
}

int summary(int rank, char *tag, INDEX_T ndata, INDEX_T nbyte, double t1, double t0) {
 int size;
 double xmin, xmax, xavg, xcov;

  printf("%d %-14s   %lld %lld %.3e %16.6f %16.6f\n",rank, tag,ndata,nbyte,t1-t0,t1,t0);
#ifdef MPI
 get_stat(t1-t0,&xmin,&xmax,&xavg,&xcov);
 if(!rank) {
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  printf("# %d %-14s %lld %lld % .4e % .4e % .4e % .4e\n",size , tag,ndata,nbyte,xmin,xmax,xavg,xcov);
 }
#endif
  fflush(stdout);
  return 0;
}


int rank_dir(int action ,int rank) {
 char buf[BUFLEN];
 
 sprintf(buf,"datadir_%d",rank);
 
 switch(action) {
  case -1:
        chdir("..");
        remove(buf);
        break;
  case  0:
        chdir("..");
        break;
  case  1:
        mkdir(buf,S_IRWXU);
        chdir(buf);
        break;
 
 }
}

#ifdef MPI
int get_stat(double x, double *xmin, double *xmax, double *xavg, double *xcov) {
  int                   size,rank,i;
  static double		xglob[4096],min,max,tot,totc;
  struct rusage         u;
                                                                                
                                                                                
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  getrusage(RUSAGE_SELF, &u); 
 
  MPI_Gather(&x,1,MPI_DOUBLE,xglob,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  if(!rank) {
   max = min = xglob[0];  tot = 0.0;
   for(i=0;i<size;i++)  {
    if(xglob[i] < *xmin) { *xmin = xglob[i];}
    if(xglob[i] > *xmax) { *xmax = xglob[i];}
    tot += xglob[i];
   }

   *xavg = tot/size; totc = 0.0;
   for(i=0;i<size;i++)  {
    totc += (xglob[i]-*xavg)*(xglob[i]-*xavg);
   }

   if(size>=2) {
    *xcov = sqrt(totc/(size-1.0))/(*xavg); 
   } else {
    *xcov = sqrt(totc/(size))/(*xavg); 
   }
  }
 
  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}
#endif

