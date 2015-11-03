#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned long long int flags=0;

#define VERBOSE (1<<0)
#define BUFLEN 4096
int debug = 1;

double comm_cost(int *perm);
int n=0,b=16;
double **data,bytes,cost,ocost;

int main(int argc, char *argv[]) {

 int i,j,ii,jj,*perm,*operm,p;
 char *fname,line[BUFLEN];
 FILE *fh;

 fname = NULL;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-b",*argv)) {
    --argc; argv++;
    b = atoi(*argv);
  } else {
   fname = *argv;
  }
  if(fname) break;
 }

 if(!fname) {
  printf("usage: bwr -b n file\n"); exit(1);
 }

 if(debug) {printf("filename = %s\n", fname);}

 fh = fopen(fname,"r");

 if(!fh) {
  printf("file %s not found\n",fname);
  exit(1);
 }

 n = 0;
 while(fgets((char *)line,BUFLEN,fh) != NULL) {
  sscanf(line,"%d %d %lf", &i, &j, &bytes);
  if(i > n) n = i;
  if(j > n) n = j;
 }
 rewind(fh);
 
 n++;

 if(debug) {printf("n = %d\n", n);}

 data = (double **)malloc((size_t)(n*sizeof(double *)));
 data[0] = (double *)malloc((size_t)(n*n*sizeof(double)));
 for(i=1;i<n;i++)  data[i] = data[i-1] + n;
 perm = (int *)malloc((size_t)(n*sizeof(int)));
 operm = (int *)malloc((size_t)(n*sizeof(int)));
 
 while(fgets((char *)line,BUFLEN,fh) != NULL) {
  sscanf(line,"%d %d %lf", &i, &j, &bytes);
  data[i][j] = 1.0*bytes;
 }
 fclose(fh);

 for(i=0;i<n;i++) {
  perm[i] = i;
  operm[i] = perm[i]; 
 }

 cost = ocost = comm_cost(perm);
 while(1) {

  ii = (int)(drand48()*n);
  jj = (int)(drand48()*n);
  p = perm[ii];
  perm[ii] = perm[jj];
  perm[jj] = p;

  cost = comm_cost(perm);
  if(cost < ocost) {
   ocost = cost;
   for(i=0;i<n;i++) {
    operm[i] = perm[i];
   }
   fh = fopen("perm", "w");
   for(i=0;i<n;i++) {
    for(j=0;j<n;j++) {
     if(data[i][j] > 0.0) {
      fprintf(fh,"%d %d %d %d %.12f\n",i, j, perm[i], perm[j], data[i][j]);
     }
    }
   }
   fclose(fh);
   printf("%.12e\n", cost);
  } else {
   for(i=0;i<n;i++) {
    perm[i] = operm[i]; 
   }
  }
 }

 for(i=0;i<n;i++) {
  for(i=0;i<n;i++) {
   if(data[i][j] > 0) {

    if(0) printf("%d %d %12.6f\n", i,j,data[i][j]);
    
   }
  }
 }

 return 0;
}

double comm_cost(int *perm) { 
 int i,j;
 double time=0.0;

 for(i=0;i<n;i++) {
  for(j=0;j<n;j++) {
   if(data[perm[i]][perm[j]] > 0) {
    if(i%b != j%b) {
     time += data[perm[i]][perm[j]];
     printf("tag %d %d %d %d %.12f\n",i, j, perm[i], perm[j], data[perm[i]][perm[j]]);
    }
   }
  }
 }

 return time;
}
