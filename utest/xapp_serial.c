#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "ipm.h"

struct ipm_jobdata job;
struct ipm_taskdata task;
struct ipm_reportdata rep;

int main(int argc, char **argv)
{
  int    i,size,rank, left, right, you,n=127;
  double *sbuf, *rbuf;
  char *env_ptr,*cptr;


  env_ptr = getenv("IPM_PIPE_READ");
  printf("** IPM_PIPE_READ=%s\n", env_ptr);
  env_ptr = getenv("IPM_PIPE_WRITE");
  printf("** IPM_PIPE_WRITE=%s\n", env_ptr);

  ipm_init(0);
  ipm_report("bingo");
  ipm_finalize();


  execv("/usr/bin/env", NULL);
  return 0;   
}

