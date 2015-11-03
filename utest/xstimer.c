#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ipm.h>

static struct ipm_taskdata task;

#if defined(IPM_REQUIRE_SECONDS_PER_CYCLE) 
#if !defined (IPM_SECONDS_PER_CYCLE)
#define IPM_DETERMINE_SPC
#define IPM_SECONDS_PER_CYCLE 1
#endif
#endif


int main(int argc, char *argv[]) {
  int i,j,k,nsec=0,usec=0;
  IPM_TICK_TYPE T1,T2;
  double t1,t2;

  IPM_TIME_INIT
  IPM_TIME_BRACKET( sleep(1);)

#if defined(IPM_DETERMINE_SPC) 
  printf("#define IPM_SECONDS_PER_CYCLE %.12e\n", 1.0/(T2-T1*1.0));
#else 
  t1 = IPM_TIME_SEC(T1);
  t2 = IPM_TIME_SEC(T2);
  printf("IPM_TIME_BRACKET(1 second) is %.12e seconds , err = %.12e\n",t2-t1,1-(t2-t1));
  IPM_TIME_BRACKET(0;)
  t1 = IPM_TIME_SEC(T1);
  t2 = IPM_TIME_SEC(T2);
  printf("IPM_TIME_BRACKET(null) is %.12e seconds\n",t2-t1);
#endif 



 return 0;
}

