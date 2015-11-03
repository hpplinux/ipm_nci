#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ipm.h>
unsigned long long int test_flags=0;

struct ipm_taskdata task;

int main(int argc, char *argv[]) {
 int i,ii,ib,rv;
 IPM_KEY_TYPE key;


 task.flags = 0;
 task.flags &= VERBOSE;
 task.flags |= DEBUG;
 task.mpi_rank=0;

#include "ipm_bit_test.c"

 return 0;
}

