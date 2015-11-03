#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

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
static long long int            ipm_trc_hpm_global_init[MAXSIZE_HPMCOUNTERS];
static long long int            ipm_trc_hpm_region_init[MAXSIZE_HPMCOUNTERS];
static long long int            ipm_trc_hpm_region_final[MAXSIZE_HPMCOUNTERS];
static double                   region_wtime_init;
static double                   region_utime_init;
static double                   region_stime_init;
static int                      region_current;
static double                   stamp_current;
static int                      eventset_current;
static struct timeval tod_tv;

#include "ipm_hpm.c"
#include "ipm_memusage.c"
#include "ipm_api.c"

int main(int argc, char *argv[]) {
 int i,j,k,ii,ireg,three_reg=0;
 double gflops;
 volatile double a = 0.5, b = 2.2;
 double c = 0.11;
 long int nflop=128*1024*1024;
 long int nfma = 0;

 IPM_CLEAR_TASKDATA(task);
 
 for(i=0;i<=MAXSIZE_CALLS;i++) {
  printf("%d \"%s\"\n", i, task.call_label[i]);
 }
 

 return 0;
}

