
#ifndef IPM_LOG_USEMULTI
#ifndef IPM_LOG_USELOCKS
#ifndef IPM_LOG_USETMPFS
#ifndef IPM_LOG_USEMPI
#define IPM_LOG_USEMPI
#endif
#endif
#endif
#endif

void ipm_trace(IPM_KEY_TYPE key, double T) {

 if(task.ipm_trc_count >= task.ipm_trc_count_max) {
  task.flags &= ~IPM_TRC_ACTIVE;
  /* FIXME this would be a good place to roll the trace buffer over if 
     it's being used as a ring buffer, or dump it out on a one time basis
     if it's being used as a snapshot buffer */
  return; 
 }

/*
 if(task.flags & DEBUG)  {
  printf("IPM: %d TRACE_ENT %s %d %d %d\n",
	 task.mpi_rank,
	 task.call_label[KEY_CALL(key)],
	 KEY_RANK(key),
	 KEY_BYTE(key),
	 KEY_REGION(key));
 }
*/

 
 task.ipm_trc_buf[task.ipm_trc_count].key = key;
#ifdef IPM_ENABLE_PROFLOW
 task.ipm_trc_buf[task.ipm_trc_count].callsite = 0;
#endif
 ipm_hpm_read(task.ipm_trc_buf[task.ipm_trc_count].hpm); 
 task.ipm_trc_buf[task.ipm_trc_count].t = T;

 task.ipm_trc_count++;
 return;
}


static void ipm_trc_mask_time_tron() {
 task.flags |= IPM_TRC_ACTIVE;
 signal(SIGALRM, ipm_trc_mask_time_troff);
 alarm(task.ipm_trc_mask_time_final);
}

static void ipm_trc_mask_time_troff() {
 task.flags &= ~IPM_TRC_ACTIVE;
}

static void ipm_trc_dump_internal();

static void ipm_trc_dump() {
 int i,j,jj,fd,rv;
 char tmp_fname[MAXSIZE_FILENAME];
 char tmp_cmd[MAXSIZE_FILENAME];
 FILE *fh, *ipm_trc_fh;

 for(i=0;i<task.ipm_trc_count;i++) {
#ifdef IPM_ENABLE_PROFLOW
  printf("IPM: %d TRACE %.6e %p %s %d %d %d ",
#else
  printf("IPM: %d TRACE %.6e - %s %d %d %d ",
#endif
        task.mpi_rank,
        task.ipm_trc_buf[i].t-task.ipm_trc_time_init,
#ifdef IPM_ENABLE_PROFLOW
        task.ipm_trc_buf[i].callsite,
#endif
        task.call_label[KEY_CALL(task.ipm_trc_buf[i].key)],
        KEY_BYTE(task.ipm_trc_buf[i].key),
        KEY_REGION(task.ipm_trc_buf[i].key),
        KEY_RANK(task.ipm_trc_buf[i].key));
  for(j=0;j<MAXSIZE_HPMCOUNTERS;j++) {
   printf("%lld ", task.ipm_trc_buf[i].hpm[j]);
  }
  printf("\n");
 }

 return;

}
