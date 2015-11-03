#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

unsigned long long int flags=0;

#define VERBOSE (1<<0)

int main(int argc, char *argv[]) {
 unsigned long long int i,n,nmin=10,nmax=100,edn=32;
 double *x;
 FILE *fh;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
  } else if(!strcmp("-dn",*argv)) {
    --argc; argv++;
    edn = atol(*argv);
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    nmax = nmin = atol(*argv);
  } else if(!strcmp("-min",*argv)) {
    --argc; argv++;
    nmin = atol(*argv);
  } else if(!strcmp("-max",*argv)) {
    --argc; argv++;
    nmax = atol(*argv);
  }

 }

 fh = fopen("bas","w");

 fprintf(fh,"a");

 printf("#%lld %lld %lld\n", nmin, nmax, edn);
 for(n=nmin;n<=nmax;n+=n/edn+1) {
  x = (double *)malloc((size_t)(n*sizeof(double)));
  for(i=0;i<n;i++) x[i] = i+1;
  if(flags & VERBOSE) {printf("n = %lld bytes = %lld\n", n, (n*sizeof(double)));}
  free((char *)x);
 }

 fclose(fh);
 return 0;
}

