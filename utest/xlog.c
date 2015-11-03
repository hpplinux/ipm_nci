#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

unsigned long long int flags=0;

#include "../include/ipm.h"
static struct ipm_taskdata task;
static struct ipm_jobdata job;
static double              report_delta=-1;
static double              stamp_current;
static int                 eventset_current;
static char 		   txt[MAXSIZE_TXTLINE];
static double                   region_wtime_init;
static double                   region_utime_init;
static double                   region_stime_init;
static int                      region_current=-1;


#include "../src/ipm_util.c"
#include "../src/ipm_init.c"
#include "../src/ipm_env.c"
#include "../src/ipm_execinfo.c"
#include "../src/ipm_api.c"
#include "../src/ipm_log_write.c"
#include "../src/ipm_memusage.c"
#include "../src/ipm_hpm.c"



int main(int argc, char *argv[]) {
 int i,j,k,ib;
 int max_keys = 124096,nwrites=1;
 int region,call,rank,byte;
 int region_err,call_err,rank_err,byte_err;
 IPM_TICK_TYPE T1,T2;
 IPM_KEY_TYPE key,key_err,hkey;
 double t1,t2;
 FILE *fh;

 
 max_keys = 4096;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    max_keys = atoi(*argv);
  } else if(!strcmp("-np",*argv)) {
    --argc; argv++;
    nwrites = atoi(*argv);
  }
 }


 ipm_init(flags);
 ipm_get_env();

 ipm_get_cmdline(task.mpi_cmdline, task.mpi_realpath);
 memset((void *)txt,0,(size_t)(MAXSIZE_TXTLINE*sizeof(char)));

 task.pid = getpid(); /* should be in ipm_init */

 IPM_TIME_BRACKET( IPM_HASH_CLEAR(task.hash));

 if(task.flags & DEBUG) {
  printf("IPM: %d hash_mem = %lld KB \\n", task.mpi_rank,
         (long long int)(size_t)((MAXSIZE_HASH+2)*sizeof(struct ipm_hash_ent))/1024);
 }

 for(i=0;i<=max_keys;i++) {

  rank = 1;
  region = 1;
  call = i%5+1;
  byte = i;
  if(0 && i<=10) {
   rank = 65525+i;
   byte = 2147483637+i; /* 2^31 - 1*/
   call = 245+i;
   region = 245+i;
  }

  if(region > 255) { printf("range error in test region = %d\n", region); }
  if(call > 255) { printf("range error in test call = %d\n", call); }
  if(rank > 65535) { printf("range error in test rank = %d\n", rank); }
  if(byte > INT_MAX) { printf("range error in test byte = %d\n", byte); }

  IPM_LOG_MPIEVENT(region,call,rank,byte,0.1);

  }

 ipm_sync();
 fh = fopen("log.xml","w");
 IPM_TIME_BRACKET( for(i=0;i<nwrites;i++) { ipm_log_write_binary(&job,&task,txt,fh);});
 fclose(fh);
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);

 printf("log_time = %f\n", t2-t1);

 return 0;
}

