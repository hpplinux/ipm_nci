#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define IOWA_VIAFORK
#define LIBPC_PATH "/home/dskinner/src/ipm/lib/libpc.so"

char *tcmd,**targv;
struct rusage u;
char *heap_s, *heap_e;

#ifdef IOWA_VIATHREAD
void thread_go();
pthread_t thrd;
#endif

#ifdef IOWA_VIAFORK
pid_t pid;
#endif

#define MAXSIZE_CMDLINE 4096

int iowa_report();

int main (int argc, char *argv[]) {
 int status;
 char envstr[MAXSIZE_CMDLINE];

 heap_s = sbrk(0);

 if(argc < 2) {
  printf("usage: %s program cmd [arg1] [arg2] [...]\n",argv[0]);
 }

 tcmd = argv[1];
 targv = argv+1;

#ifdef IOWA_VIATHREAD
 pthread_create(&thrd, NULL, (void *) &thread_go, NULL);
 getrusage(RUSAGE_SELF, &rusage_self_start);
 fflush(stdout);
 pthread_join(thrd, NULL); 
 if(0) printf("joined\n");
 fflush(stdout);
 getrusage(RUSAGE_SELF, &rusage_self_final);
#endif

#ifdef IOWA_VIAFORK
 pid = fork(); 
  switch(pid) {
  case 0:  /* child */
   sprintf(envstr, "LD_PRELOAD", LIBPC_PATH);
   putenv(envstr);
   execvp(tcmd,targv);
  case -1: /* error parent, no child */
  default: /* parent */
#endif

 heap_e = sbrk(0);
 iowa_report();
 }
 exit(0);
}

void thread_go() {
   char envstr[MAXSIZE_CMDLINE];
   sprintf(envstr, "LD_PRELOAD", LIBPC_PATH);
   putenv(envstr);
   execvp(tcmd,targv);
   printf("errno = %d\n",errno);
#ifdef IOWA_VIATHREAD
   pthread_exit((void *)(&errno));
#endif
}

int iowa_report() {
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
 printf("ru_inblock: %ld\n",u.ru_oublock);
 printf("ru_msgsnd: %ld\n",u.ru_msgsnd);
 printf("ru_msgrcv: %ld\n",u.ru_msgrcv);
 printf("ru_nvcsw: %ld\n",u.ru_nvcsw);
 printf("ru_nivcsw: %ld\n",u.ru_nivcsw);
}
