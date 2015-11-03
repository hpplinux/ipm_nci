#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "ipm.h"
struct ipm_jobdata job;
struct ipm_taskdata task;
struct ipm_reportdata rep;

#include "../src/ipm_hpm.c"
#include "../src/ipm_hpm_test.c"
#include "../src/ipm_init.c"
#include "../src/ipm_finalize.c"
#include "../src/ipm_env.c"
#include "../src/ipm_trace.c"

unsigned long long int iflags; 

#define IPM_UTEST(test,label,cond,report)  {\
 IPM_TIME_GTOD(it1);\
 test;\
 IPM_TIME_GTOD(it2);\
 ntests++;\
 if((cond)) {\
  printf("FAIL %-40.40s %.3e ",label,it2-it1);\
  report;\
  printf("\n");\
 } else {\
  npass++;\
  if(iflags & VERBOSE) {\
   printf("PASS %-40.40s %.3e ",label,it2-it1);\
   report;\
   printf("\n");\
  }\
 }\
}

int main(int argc, char *argv[]) {

 int i,n,ntests=0,npass=0,irv;
 double it1,it2,t1,t2;
 IPM_TICK_TYPE T1,T2;


 task.flags = 1;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   iflags |= VERBOSE;
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    n = atoi(*argv);
  }
 }

 IPM_UTEST(irv=ipm_init(iflags),"ipm_init",irv>0,
	printf("--> rv=%d",irv);
 );

 IPM_UTEST(0,"ipm_init (task.flags!=0)", task.flags==0,
	printf("--> %lld",task.flags);
 );

 IPM_UTEST(0,"ipm_init (IPM_APP_RUNNING)",(task.flags & IPM_APP_RUNNING)==0,
	printf("--> %lld",task.flags&IPM_APP_RUNNING);
 );

 IPM_UTEST(0,"ipm_init (!IPM_APP_COMPLETED)",(task.flags & IPM_APP_COMPLETED)!=0,
	printf("--> %lld",task.flags&IPM_APP_COMPLETED)
 );

 IPM_UTEST(0,"ipm_init (!IPM_APP_INTERRUPTED)",(task.flags & IPM_APP_INTERRUPTED)!=0,
	printf("--> %lld",task.flags&IPM_APP_INTERRUPTED)
 );

 IPM_UTEST(0,"ipm_init (task.switch_bytes_rx)",task.switch_bytes_rx!=-1.0,
	printf("--> %f",task.switch_bytes_rx)
 );

 IPM_UTEST(0,"ipm_init (task.switch_bytes_tx)",task.switch_bytes_tx!=-1.0,
	printf("--> %f",task.switch_bytes_tx);
 );

/* timer */

 IPM_UTEST(sleep(1),"ipm_timer (gtod sleep 1)",((it1>it2) || fabs(1.0-(it2-it1))>0.01),
	printf("--> %.6e",1.0-(it2-it1));
 );

 IPM_TIME_BRACKET(sleep(1));
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);

 IPM_UTEST(0,"ipm_timer (ipm sleep 1)",((t1>t2) || fabs(1.0-(t2-t1))>0.01),
	printf("--> %.6e",1.0-(t2-t1));
 );
 return 0;
}

