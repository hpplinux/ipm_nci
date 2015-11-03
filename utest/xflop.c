#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef MPI
#include <mpi.h>
#endif

unsigned long long int flags=0;

#define VERBOSE  (1<<0)
#define CORE     (1<<1)

void doflop(int m, int n,double *x);
int main(int argc, char *argv[]) {

 int rank=0, size=1;
 long long int i,n=1,m=1;
 double *x;

 if(argc < 2) {
  printf("usage ./xflop [-n #] [-m #]\n");
  printf("\t m*n flops will be done in a vector of length m.\n");
 }


 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    n = atol(*argv);
  } else if(!strcmp("-m",*argv)) {
    --argc; argv++;
    m = atol(*argv);
  } else if(!strcmp("-c",*argv)) {
    flags |= CORE;
  }
 }

#ifdef MPI
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif 

/*
 if(m) { 
 x = (double *)malloc((size_t)(m*sizeof(double)));
 for(i=0;i<m;i++) {
  x[i] = i;
 }
 for(i=0;i<m;i++) {
  x[i] = i*x[i];
 }
 if(x[m-1]*x[m-1] < 0) {
  printf("trickster\n");
 }
 free((char *)x);
}
*/

 if(n) {
  x = (double *)malloc((size_t)(m*sizeof(double)));
  for(i=0;i<m;i++) {
   x[i] = i;
  }
  doflop(m,n,x);
  free((char *)x);
 }

 if(m<n) m=n;

 printf("FLOPS = %lld BYTES = %lld\n", n*m, m*sizeof(double));

 fflush(stdout);

 if(flags & CORE) {
  for(i=0;;i++) {
   x[i] = x[i-1000];
  }
 }


#ifdef MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
#endif

 return 0;
}

void doflop(int m, int n, double *x) {
 int i,j;
 double y=0.1;

 y = 1.1;

 for(j=0;j<n;j++) {
  for(i=0;i<m;i++) {
   x[i] += y*x[i];
  }
 }

 if(x[i] == 1.1234) {
  printf("%.3e \n", x);
 }

}

