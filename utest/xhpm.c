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

#define do_flops(n) {for (i = 0; i < n; i++) {c += b;   } if(c<0.1 && c>0.2) printf("trickster\n");}
#define do_fmas(n) {for (i = 0; i < n; i++) {c += a * b;   } if(c<0.1 && c>0.2) printf("trickster\n");}

int main(int argc, char *argv[]) {
 int i,j,k,ii,ireg,three_reg=0;
 double gflops;
 volatile double a = 0.5, b = 2.2;
 double c = 0.11;
 long int nflop=128*1024*1024;
 long int nfma = 0;

 task.flags = 0;
 ipm_init(task.flags);
/*
 IPM_CLEAR_TASKDATA(task);
 task.mpi_rank = 0;
 task.mpi_size = 1;
 task.hpm_eventset = 0;
*/
 task.flags |= IPM_HPM_ACTIVE;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
    task.flags |= VERBOSE;
  } else if(!strcmp("-x",*argv)) {
    task.hpm_eventset = -1;
    task.flags &= ~IPM_HPM_ACTIVE;
  } else if(!strcmp("-r",*argv)) {
    three_reg = 1;
  } else if(!strcmp("-s",*argv)) {
    --argc; argv++;
    task.hpm_eventset = (int)atol(*argv);
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    nflop = atol(*argv);
/*    printf("nflop = 2 * %ld\n", nflop); */
  } else if(!strcmp("-fma",*argv)) {
    --argc; argv++;
    nfma = atol(*argv);
  }
 }

 ipm_hpm_start();

 if(0) {
  printf("IPM: %d hpm_test eventset=%d events :", task.mpi_rank, task.hpm_eventset);
  for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
    ii = ipm_hpm_eorder[task.hpm_eventset][i];
    printf("%s ", ipm_hpm_ename[task.hpm_eventset][ii]);
  }
  printf("\n");

  for(ireg=0;ireg<MAXSIZE_REGION;ireg++) {
    printf("IPM: %d hpm_test init region(%d) %s count",
        task.mpi_rank,ireg, task.region_label[ireg]);
   for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
    ii = ipm_hpm_eorder[task.hpm_eventset][i];
	printf(" %lld", task.hpm_count[ireg][task.hpm_eventset][ii]);
   }
   printf("\n");
  }
 }

 if(!three_reg) {
  ipm_region(1,"ipm_noregion");
  ipm_region(-1,"ipm_noregion");

  do_flops(nflop);
  do_fmas(nfma);

  ipm_region(-1,"ipm_noregion");
 } else {

  ipm_region(1,"ipm_noregion");
  do_flops(nflop);
  ipm_region(-1,"ipm_noregion");

  ipm_region(1,"a");
  do_flops(nflop*2);
  ipm_region(-1,"a");

  ipm_region(1,"b");
  do_flops(nflop*3);
  ipm_region(-1,"b");

 }

 ipm_hpm_stop();

 IPM_PRINT_TASKDATA(task);

 return 0;
}

