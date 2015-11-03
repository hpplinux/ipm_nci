#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ipm.h>
unsigned long long int test_flags=0;

struct ipm_taskdata task;
struct ipm_jobdata job;
#include "../src/ipm_env.c"


int main(int argc, char *argv[]) {
 int i,ii,ib,rv;
 IPM_KEY_TYPE key;
 int ireg,icall,irank,ibyte;



 task.flags = 0;
 task.flags &= VERBOSE;
 task.flags |= DEBUG;
 task.mpi_rank=0;

 ipm_get_env();

 task.call_mask[0] = CALL_MASK_BPREC;
 printf("init ");
 IPM_SHOWBITS(task.call_mask[0]);
 printf("\n");

 for(i=0;i<5;i++) {
  key = 0;
  ireg = i;
  irank = i;
  icall = 0;
  ibyte = i;
  IPM_MPI_KEY(key,ireg,icall,irank,ibyte);
  printf("key%d ",i);
  IPM_SHOWBITS(key);
  printf("\n");
 }

 CALL_BUF_PRECISION_DOWN(0);
 printf("down ");
 IPM_SHOWBITS(task.call_mask[0]);
 printf("\n");

 for(i=0;i<5;i++) {
  key = 0;
  ireg = i;
  irank = i;
  icall = 0;
  ibyte = i;
  IPM_MPI_KEY(key,ireg,icall,irank,ibyte);
  printf("key%d ",i);
  IPM_SHOWBITS(key);
  printf("\n");
 }

 CALL_BUF_PRECISION_UP(0);
 printf("up   ");
 IPM_SHOWBITS(task.call_mask[0]);
 printf("\n");

 return 0;
}

