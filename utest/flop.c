#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned long long int flags=0;

#define VERBOSE (1<<0)

int main(int argc, char *argv[]) {

 int i,n=1000000;
 double x=0.1;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    n = atoi(*argv);
  }
 }

 for(i=0;i<n;i++) {
  x+= 0.111111111111112;
 }
 
 if((int)(x)%32571 == 511) printf("%d\n", n);
 return 0;
}

