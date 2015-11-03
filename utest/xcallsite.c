#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ipm.h>
#include <ipm_callsite.h>
unsigned long long int test_flags=0;

struct ipm_taskdata task;

int sub1(void);
int sub2(void);
int sub3(void);

int main(int argc, char *argv[]) {
 int i,ii,ib,rv;
 IPM_KEY_TYPE key;


 task.flags = 0;
 task.flags &= VERBOSE;
 task.flags |= DEBUG;
 task.mpi_rank=0;

 sub1();

 return 0;
}

int sub1(void) { IPM_ADDR_TYPE cs; IPM_GET_CALLSITE(cs); printf("enter sub1 %p\n", cs); sub2(); printf("leave sub1 %p\n", cs); return 0;}
int sub2(void) { IPM_ADDR_TYPE cs; IPM_GET_CALLSITE(cs); printf("enter sub2 %p\n", cs); sub3(); printf("leave sub2 %p\n", cs); return 0;}
int sub3(void) { IPM_ADDR_TYPE cs; IPM_GET_CALLSITE(cs); printf("enter sub3 %p\n", cs); printf("leave sub3 %p\n", cs); return 0;}

