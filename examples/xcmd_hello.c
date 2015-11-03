#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>

unsigned long long int flags=0;

#define VERBOSE (1<<0)
#define FMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH


int main(int argc, char *argv[]) {

 int i,n=3,fd;
 double *x; 
 mode_t amode;
 FILE *fh;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    n = atoi(*argv);
  }
 }

 x = (double *)malloc((size_t)(n*sizeof(double)));
 printf("hello n=%d\n", n);
 free((char *)x);

 fh=fopen("tmp_file","w");
 fprintf(fh,"1");
 fclose(fh);

 unlink("tmp_file");

 fd = open("tmp_file",O_CREAT|O_WRONLY,FMODE);
 write(fd,x,n*sizeof(double));
 close(fd);

 unlink("tmp_file");

 fd = open("tmp_file",O_CREAT|O_WRONLY);
 write(fd,x,n*sizeof(double));
 close(fd);

 unlink("tmp_file");

 return 0;
}

