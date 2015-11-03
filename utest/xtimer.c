#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ipm.h>

static struct ipm_taskdata task;

int main(int argc, char *argv[]) {
  int i,j,k,nsec=0,usec=0;
  IPM_TICK_TYPE T1,T2;
  double t1,t2;

 task.flags = 0;
 IPM_CLEAR_TASKDATA(task);
 task.mpi_rank = 0;
 task.mpi_size = 1;
 task.hpm_eventset = 0;
 task.flags |= IPM_HPM_ACTIVE;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
    task.flags |= VERBOSE;
  } else if(!strcmp("-x",*argv)) {
    --argc; argv++;
    nsec = (int)atoi(*argv);
  }
 }

 IPM_TIME_INIT

 while(1) {

 if(!nsec) usec = (int)(1000*drand48());
 IPM_TIME_BRACKET( usleep(usec);)

 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);

 if(!nsec || task.flags & VERBOSE) {
  printf("%.8e seconds lasts %.8e seconds\n", usec/1000000.0, t2-t1);
 }

 if(t2<t1) {
  printf("ERROR %.8e seconds lasts %.8e - %.8e seconds\n", usec/1000000.0, t2, t1);
 }

  if(nsec) break;
 }

 return 0;
}

/*

cc -o x_rtc x_rtc.c -lfi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <intrinsics.h>


extern long long int rtc_rate_();

int main(int argc, char *argv[]) {

 int it=0;
 long long int tick1, tick2, irate;


 tick1 = _rtc();
 sleep(10);
 tick2 = _rtc();
 printf("10 seconds is %lld ticks\n", tick2-tick1);

 irate = irtc_rate_();

 printf("rtc_rate = %lld\n",irate);
 return 1;
}

*/
