#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

unsigned long long int flags=0;


#include <ipm.h>

static struct ipm_taskdata task;
static IPM_TICK_TYPE            ipm_trc_tick_init;
static double                   ipm_trc_time_init=0.0;
static int                      ipm_trc_mask_region_iti[MAXSIZE_REGION];
static int                      ipm_trc_mask_region_itf[MAXSIZE_REGION];
static char                     ipm_trc_mask_region[MAXSIZE_REGION][MAXSIZE_LABEL];
static double                   ipm_trc_mask_time_init=-1.0;
static double                   ipm_trc_mask_time_final=-2.0;
static int                      ipm_trc_nregion=0;
static IPM_COUNT_TYPE           ipm_trc_count=0;
static IPM_COUNT_TYPE           ipm_trc_count_max=0;
static double                   region_wtime_init;
static double                   region_utime_init;
static double                   region_stime_init;
static int                      region_current;
static double                   stamp_current;
static int                      eventset_current;
static struct timeval tod_tv;


#include "ipm_memusage.c"

int main(int argc, char *argv[]) {
 long long i,j=1024;
 double *data;

 task.pid = getpid(); /* should be in ipm_init */

 if(argc < 2) {
 } else {
  j = atol(argv[1]);
 }

 data = (double *)malloc((size_t)(j*sizeof(double)));
 for(i=0;i<j;i++) {
  data[i] = i%task.pid;
 }

 if(data[i] < 0.0) {
  exit(1); /* this is here to prevent the loop above from getting ignored */
 }

 ipm_get_procmem(&task.bytes);
 printf("task.bytes = %e ( malloc %e bytes rel err = %e)\n",
	 task.bytes,j*8.0,1.0-(task.bytes/(1.0*j*8)));

 return 0;
}

