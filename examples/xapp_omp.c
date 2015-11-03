#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <omp.h>

#define INDEX_T long
#define DATA_T double

#define VERBOSE  (1<<0)
#define DOERR    (1<<1)
#define DOSCAN   (1<<2)
#define DOSCANX  (1<<3)

double wtime() {
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + 1.0e-6*tv.tv_usec;
}


int main (int argc, char *argv[])  {
 int 		flags=0,sdone=0;
 INDEX_T	it,nit=128;
 INDEX_T     	i,j,n=1000000,dn=1,nt=-1;
#ifdef DOBIND
 int		dobind=0;
#endif
 DATA_T 	*a;
 double		sum, t0,t1; 
 time_t		ts;				/* DATE */
 char		cbuf[4096];			/* STRBUFFER */


 if(argc==1) { 
  printf("usage: %s [-scan dn | -scanx dn] [-bind]  vec_size num_threads [dobind]\n",argv[0]);
  exit(1);
 }
 i=0; n=0;
 gethostname(cbuf,4096);
 printf("host         %s ",cbuf);
 time(&ts);
 strftime(cbuf, 4096, "%c", localtime(&ts));
 printf("    (%s)\n", cbuf);
 printf("cmd          \"");
 for(i=0;i<argc;i++) printf("%s ",argv[i]);
 i=0;
 printf("\"  ");
 while(--argc && argv++) {
  if(!strcmp("-v",*argv) ) {
   flags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-nit",*argv) ) {
    --argc; argv++;
    nit = atol(*argv);
  } else if(!strcmp("-scan",*argv)) {
    flags |= DOSCAN;
    --argc; argv++;
    dn = atol(*argv);
   printf("SCAN ");
  } else if(!strcmp("-scanx",*argv)) {
    flags |= DOSCANX;
    --argc; argv++;
    dn = atol(*argv);
   printf("SCANX ");
  } else if(n == 0) { 
   n = atol(*argv);
  } else if(n > 0) { 
   nt = atol(*argv);
  }
 }
 printf("\n");


 sdone = 0;
 while(!sdone) {

  if(nt>0) { omp_set_dynamic(0); omp_set_num_threads(nt); }
 
  a = (DATA_T *)malloc((size_t)n*sizeof(DATA_T));

  if(!a) {printf("malloc error %d\n",n); exit(1);}

  for (i=0;i<n;i++) { a[i] = 1.0*i; }

  sum = 0.0;
  t0 = wtime();
#pragma omp parallel shared(a,sum,nt) private(i)
{
#ifdef DOBIND
   if(dobind) {
    bindprocessor(BINDTHREAD, thread_self(),omp_get_thread_num());
    printf("thread %ld on cpu %ld\n",omp_get_thread_num(),mycpu());
   }
#endif
   nt = omp_get_num_threads();
   for(it=0; it < nit; it++) {
#pragma omp master
   sum = 0.0;
#pragma omp barrier 
#pragma omp for schedule(static) reduction(+:sum)
    for (i = 0; i < n; i++) { sum += a[i]*a[i]; }
   }
}
  t1 = wtime();
  printf("%ld %ld %ld %.3e %.3e\n",
	nt,n,nit,sum-(2.0*n*n*n-3*n*n+n)/6.0,(t1-t0)/nit); fflush(stdout);

  if(flags & DOSCAN) { n += dn; }
  else if(flags & DOSCANX) { n += n/dn +1; }
  else { sdone=1; }
 } /* while */
}

