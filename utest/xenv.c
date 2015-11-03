#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ipm.h>

unsigned long long int test_flags=0;
struct ipm_taskdata task;
struct ipm_jobdata job;

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
static char                     mpi_label[MAXSIZE_CALLS+1][MAXSIZE_LABEL];
static char                     mpi_lang[MAXSIZE_LABEL];



#include "ipm_env.c"

int main(int argc, char *argv[]) {
 int i,j,k,ii,ib,rv;
 IPM_KEY_TYPE key;

 IPM_CLEAR_TASKDATA(task); /* this resets task.flags */
 task.mpi_rank=0;	   /* this emulates the 1 task parallel case */

 ipm_get_env();

 IPM_SHOWBITS(task.flags);
 printf("\n");

 return 0;
}

