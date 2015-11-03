#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ipm.h"
struct ipm_jobdata job;
struct ipm_taskdata task;
struct ipm_reportdata rep;
#include "../src/ipm_init.c"
#include "../src/ipm_env.c"

unsigned long long int iflags; 

int main(int argc, char *argv[]) {

 int i,n;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   iflags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    n = atoi(*argv);
  }
 }

 ipm_init(iflags);

 printf("hello n=%d\n", n);
 return 0;
}

