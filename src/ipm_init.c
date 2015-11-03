/*
clear task data
ipm_get_env
test hpm
test locking
test hashing
populate call labels
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

static double region_wtime_init;


int ipm_init_task(unsigned long long int iflags);

int ipm_init(unsigned long long int iflags) {

 int i,j,k,rv=0,ii,ib;
 IPM_KEY_TYPE key;
 unsigned long long int test_flags;


/* zero out the taskdata struct */

 ipm_init_task(iflags);

 task.flags  |= IPM_APP_RUNNING;

 /* signal(SIGINT,ipm_mpi_interrupt);    */
 /* signal(SIGXCPU,ipm_mpi_interrupt);    */

 IPM_TIME_INIT;
 IPM_TIME_GTOD(task.stamp_init);
 time(&task.start_ts);
 IPM_TIME_GET(task.ipm_trc_tick_init);
 task.ipm_trc_time_init = IPM_TIME_SEC(task.ipm_trc_tick_init);
 region_wtime_init = task.ipm_trc_time_init;
 IPM_GETRUSAGE_SELF(task.ru_SELF_init);
 IPM_GETRUSAGE_CHILD(task.ru_CHILD_init);
 task.flags |= IPM_APP_RUNNING;

/* default verbosity */
 task.flags |= REPORT_FULL;
 task.flags |= LOG_FULL;
 task.flags |= PARALLEL_IO_LOG;

/* Parse environment variables                                          */
/* set defaults and then loop over environ and pick out IPM_ vars       */
/* report IPM_X if X is unrecognized as being a likely mispelling       */
/*                                                                      */



 task.hpm_eventset=0;
 task.flags |= IPM_HPM_ACTIVE; /* unless canceled or failed use HPM */
#ifndef IPM_DISABLE_LOG
 strncpy(job.log_dir,IPM_LOG_DIR, MAXSIZE_FILENAME);
#endif
 task.flags &= ~IPM_TRC_ACTIVE;
 for(i=1;i<=MAXSIZE_CALLS;i++) {
  task.call_mask[i] &= ~CALL_OPT_TRACE;
 }

 ipm_get_env();

/* runtime startup tests */
#include "ipm_bit_test.c"
#include "ipm_call_test.c"
#include "ipm_hash_test.c"
#include "ipm_size_test.c"

#ifndef IPM_MONITOR_PTHREADS
  if(!(task.flags & IPM_HPM_CANCELED)) {
   rv = ipm_hpm_test(0);
   if(rv == 0) {
    if(task.mpi_rank == 0) {
    printf("IPM: 0 WARNING HPM already in use or not available, continuing without HPM\n");
    }
/*
  printf("a "); IPM_SHOWBITS(task.flags); printf("\n");
  printf("b "); IPM_SHOWBITS(IPM_HPM_CANCELED); printf("\n");
*/
  task.flags |= IPM_HPM_CANCELED;
/*
  printf("c "); IPM_SHOWBITS(task.flags); printf("\n");
  printf("d %lld %lld %lld rv = %d\n",
        IPM_HPM_CANCELED,
        task.flags,
        (task.flags & IPM_HPM_CANCELED),
        rv);
*/
    task.flags &= ~IPM_HPM_ACTIVE;
   }
  }
#endif

#ifndef IPM_DISABLE_LOG

 if(task.flags & DEBUG) {
  printf("IPM: %d TRACE prep enter\n", task.mpi_rank);
  fflush(stdout);
 }
 fflush(stdout);
 /* if IPM_TRC_ACTIVE then the above was triggered and we will need
    the trace buffer now or at some point later so let's set it up */

 if(task.flags & IPM_TRC_ACTIVE) {
  task.ipm_trc_count = 0;
  if(!task.ipm_trc_count_max) {
   task.ipm_trc_count_max = 1024*1024*MAXSIZE_TRACEMEM/sizeof(struct ipm_trc_ent);
  }
#ifndef IPM_DISABLE_LOG
  task.ipm_trc_buf = (struct ipm_trc_ent *)malloc((size_t)task.ipm_trc_count_max*sizeof(struct ipm_trc_ent));
  if(!task.ipm_trc_buf) {
   printf("IPM: %d ERROR trace memory=%f MB (%lld) malloc failed\n",
         task.mpi_rank,
         OOMB*task.ipm_trc_count_max*sizeof(struct ipm_trc_ent),
         task.ipm_trc_count_max);
  }

  for(i=0;i<task.ipm_trc_count_max;i++) {
   IPM_CLEAR_TRCENT(task.ipm_trc_buf[i]);
  }

#endif

  if(task.flags & DEBUG) {
   printf("IPM: %d TRACE memory=%f MB (%lld) \n",
         task.mpi_rank,
         OOMB*task.ipm_trc_count_max*sizeof(struct ipm_trc_ent),
         task.ipm_trc_count_max);
  }
 }


 /* now turn off IPM_TRC_ACTIVE temporarily if time slicing */
 /* violates semantics of the above code but avoids another bit var */
 if(task.ipm_trc_mask_time_init >= 0.0) {
  task.flags &= ~IPM_TRC_ACTIVE;
  signal(SIGALRM, ipm_trc_mask_time_tron);
  alarm(task.ipm_trc_mask_time_init);
 }

#endif

 if(task.flags & DEBUG) {
  printf("IPM: %d TRACE prep exit\n", task.mpi_rank);
  fflush(stdout);
 }
 fflush(stdout);


 return rv;
}

int ipm_init_task(unsigned long long int iflags) {
 int i,j,k;

 task.flags=0;
 task.ipm_size = task.mpi_size = -1;
 task.ipm_rank = task.mpi_rank = -1;
 task.wtime=task.utime=task.stime=task.mtime=task.mtime_lost=0.0;
 task.region_nregion=0;
 task.region_current=0;
 task.ipm_trc_time_init=0.0;
 task.ipm_trc_nregion=0;
 task.ipm_trc_mask_time_init=-1.0;
 task.ipm_trc_mask_time_final=-2.0;
 task.ipm_trc_count=0;
 task.ipm_trc_count_max=0;
 task.ipm_trc_buf=NULL;
 task.bytes = task.flops=0.0;
 task.stamp_init=task.stamp_current=task.stamp_final=task.stamp_init_global;
 task.hash_nlog = task.hash_nkey = 0;
 task.switch_bytes_tx = task.switch_bytes_rx = -1.0;
 for(i=0;i<=MAXSIZE_CALLS;i++) {
  task.call_mask[i] = CALL_MASK_BPREC;
 }
 for(j=0;j<MAXSIZE_REGION;j++) {
  for(i=0;i<=MAXSIZE_CALLS;i++) {
  task.call_mtime[j][i] = 0.0; 
  task.call_mcount[j][i] = 0; 
  }
 }
 for(i=0;i<MAXSIZE_REGION;i++) {
  task.region_count[i] = 0;
  task.region_wtime[i] = 0.0;
  task.region_utime[i] = 0.0;
  task.region_stime[i] = 0.0;
  task.region_mtime[i] = 0.0;
  for(j=0;j<MAXSIZE_NEVENTSETS;j++) {
   for(k=0;k<MAXSIZE_HPMCOUNTERS;k++) {
    task.hpm_count[i][j][k] = 0;
   }
  }
 }
 IPM_LABEL_WRITE
 return 0;
}

