/* 
 *   Command line driver for PMAPI via fork
 *
 *   usage: getrusage cmd
 *
 *   build: cc -o xpmapi xpmapi.c
 *
 *   David Skinner	Aug 2004 (dskinner@nersc.gov) 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pmapi.h> 

int show_pmapi_info(int rv, pm_info_t p, pm_groups_info_t pg);

int main (int argc, char *argv[]) {
 int status,i,j,rv;
 pid_t pid;
 struct rusage u;
 char *heap_s, *heap_e;
 pm_prog_t pmapi_prog; 
 pm_info_t pmapi_info;
 pm_groups_info_t pmapi_groups_info;

 rv = pm_init(PM_VERIFIED|PM_GET_GROUPS, &pmapi_info, &pmapi_groups_info);

 show_pmapi_info(rv, pmapi_info, pmapi_groups_info);

 for(i=0;i<pmapi_info.maxpmcs;i++)
            pmapi_prog.events[i]=COUNT_NOTHING;
        pmapi_prog.mode.w          = 0;
        pmapi_prog.mode.b.user     = 1;
        pmapi_prog.mode.b.proctree = 1;


 heap_s = sbrk(0);

 if(argc < 2) {
  printf("usage: %s program [arg1] [arg2] [...]\n",argv[0]);
 }

 
 pid = fork();
 switch(pid) {
  case 0:  /* child */
   execvp(argv[1],argv+1);
   printf("errno = %d\n",errno);
   break;
  case -1: /* error parent, no child */
   if(errno==EAGAIN) { fprintf(stderr,"error: fork() EAGAIN\n"); exit(1); }
   if(errno==ENOMEM) { fprintf(stderr,"error: fork() ENOMEM\n"); exit(1); }
   fprintf(stderr,"error: fork() ?\n"); exit(1);
   break;
  default: /* parent */
   printf("waiting for %d\n",pid);
   fflush(stdout);
   waitpid(pid,0,0);
   getrusage(RUSAGE_CHILDREN, &u);
   heap_e = sbrk(0);
   printf("sbrk_s: %ld\n",heap_s);
   printf("sbrk_e: %ld\n",heap_e);
   printf("ru_utime.: %ld.%6.6ld\n", u.ru_utime.tv_sec, u.ru_utime.tv_usec);
   printf("ru_stime.: %ld.%6.6ld\n", u.ru_stime.tv_sec, u.ru_stime.tv_usec);
   printf("ru_maxrss: %ld\n",u.ru_maxrss);
   printf("ru_ixrss: %ld\n",u.ru_ixrss);
   printf("ru_idrss: %ld\n",u.ru_idrss);
   printf("ru_isrss: %ld\n",u.ru_isrss);
   printf("ru_minflt: %ld\n",u.ru_minflt);
   printf("ru_majflt: %ld\n",u.ru_majflt);
   printf("ru_inblock: %ld\n",u.ru_inblock);
   printf("ru_oublock: %ld\n",u.ru_oublock);
   printf("ru_msgsnd: %ld\n",u.ru_msgsnd);
   printf("ru_msgrcv: %ld\n",u.ru_msgrcv);
   printf("ru_nvcsw: %ld\n",u.ru_nvcsw);
   printf("ru_nivcsw: %ld\n",u.ru_nivcsw);
   break;
 }
 exit(0);
}

int show_pmapi_info(int rv, pm_info_t p, pm_groups_info_t pg) {
 
 int i,j,k;
 pm_events_t pt;

 printf("pmapi initrv %d\n", rv);
 printf("pmapi info maxpmcs %d\n", p.maxpmcs);

 for(i=0;i<p.maxpmcs;i++) {
  printf("pmapi info %d maxevents %d\n", i, p.maxevents[i]);
/*
  for(j=0;j<p.maxevents[i];j++) {
   printf("pmapi info %d event %d %s \"%s\"\n",
	 i,j,p.list_events[i][j].short_name, p.list_events[i][j].description);
  }
*/
 }

 printf("pmapi info thresholdmult %d\n", p.thresholdmult);
 printf("pmapi info proc_name %s\n", p.proc_name);
 printf("pmapi info hthresholdmult %d\n", p.hthresholdmult);

 if(pg==NULL) { 
  printf("pmapi nogroups\n");
 } else {
  printf("pmapi # groups = %d\n", pg.maxgroups);
  for(i=0;i<pg.maxgroups;i++) {
   printf("pmapi group %d \n", pg.event_groups[i].group_id);
  }
 }

}
