#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

unsigned long long int flags=0;

#include <mpi.h>
#include <ipm.h>
#include <ipm_fproto.h>

struct ipm_taskdata task;

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

#include "../src/ipm_api.c"
#include "../src/ipm_hpm.c"
#include "../src/ipm_mpi_pcontrol.c"
#include "../src/ipm_memusage.c"

