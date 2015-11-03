static void ipm_mpi_interrupt(int signo) {
 task.flags |= IPM_INTERRUPTED;
 task.flags &= ~IPM_APP_RUNNING;
 task.flags |= IPM_APP_INTERRUPTED;
 task.signal = signo;

/* set stamp_final */
 IPM_TIME_GTOD(task.stamp_final_mpi);

 if(!task.mpi_rank) {
  printf("IPM: %d ERROR job interrupted (ipm_mpi_interrupt, signal %d)\n",task.mpi_rank,signo);
 }
 ipm_region(1,"ipm_noregion");
#ifndef IPM_MONITOR_PTHREADS
 if(task.flags & IPM_HPM_ACTIVE) {
  ipm_hpm_stop();
 }
#endif
 ipm_sync();
 ipm_report("IPM_INTERRUPTED");
#ifndef IPM_DISABLE_LOG
 ipm_log();
#endif
}

