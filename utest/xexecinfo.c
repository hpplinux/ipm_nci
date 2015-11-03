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


#include "ipm_execinfo.c"

int main(int argc, char *argv[]) {
 int i;

 task.pid = getpid(); /* should be in ipm_init */

 printf("argc = %d\n", argc);
 for(i=0;i<=argc-1;i++) {
  printf("argv[%d] = %s\n", i, argv[i]);
 }

 ipm_get_cmdline(task.mpi_cmdline, task.mpi_realpath);

 printf("task.mpi_cmdline  = %s\n", task.mpi_cmdline);
 printf("task.mpi_realpath = %s\n", task.mpi_realpath);

 return 0;
}

