#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>        
#include <string.h>            
#include <errno.h>

#include <ipm.h>


#define GNU_SOURCE
#include <dlfcn.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *) -1l)
#endif 

#define IPM_LIBPC_INITIALIZED		(0x0000000000000001ULL <<  1)
#define IPM_LIBPC_FINALIZING  		(0x0000000000000001ULL <<  2)
#define IPM_LIBPC_ACTIVE		(0x0000000000000001ULL <<  3)
#define IPM_TRC_MEM			(0x0000000000000001ULL <<  5)
#define IPM_TRC_IO			(0x0000000000000001ULL <<  6)

static unsigned long long int flags=0;
static char *char_error;
static void libpc_init(void);
static void libpc_finalize(void);
static void libpc_finalize_sig(int sig);
static void libpc_clear(void);
static void libpc_report(void);
static int libpc_signal_caught;

static void * (*pmalloc)()=NULL;
static void * (*pfree)()=NULL;
static void * (*open)()=NULL;

int open (__const char *__file, int __oflag, ...) {
}

void libpc_init(void) {
 char *envptr;
 struct sigaction sigact;

 libpc_clear();

 getenv("IPM_DEBUG");
 if(envptr) {
  flags |= DEBUG;
 }
 getenv("IPM_TRACE_MEM");
 if(envptr) {
  flags |= IPM_TRC_MEM;
 }

 getenv("IPM_TRC_IO");
 if(envptr) {
  flags |= IPM_TRC_IO;
 }


 if ( atexit( libpc_finalize ) != 0 )
   fprintf(stderr, "Warning: can't register exit function \"cleanup()\n" );

 sigact.sa_handler = libpc_finalize_sig;
 sigact.sa_flags = 0;

 if ( sigemptyset( &sigact.sa_mask ) == -1 ) {
     fprintf( stderr, "Error: cannot clear interrupt handler: %s\n",
              strerror( errno ));
     exit( -1 );
 }

 if ( sigaction( SIGINT, &sigact, NULL ) == -1 ) {
     fprintf( stderr, "Warning: cannot install interrupt handler: %s\n",
               strerror( errno ));
 }

 if ( sigaction( SIGTERM, &sigact, NULL ) == -1 ) {
     fprintf( stderr, "Warning: cannot install timeout handler: %s\n",
               strerror( errno ));
 }


#define GET_PFUNC(f,pf) { \
 if(!pf) {\
  *(void **) (&pf) = dlsym(RTLD_NEXT, "malloc");\
  if ((char_error = dlerror()) != NULL)  {\
   fprintf (stderr, "malloc %12.12s\n", char_error);\
   exit(1);\
  }\
 }\

}

 if(!pmalloc) {
  *(void **) (&pmalloc) = dlsym(RTLD_NEXT, "malloc");
  if ((char_error = dlerror()) != NULL)  {
   fprintf (stderr, "malloc %12.12s\n", char_error);
   exit(1);
  }
 }

 if(!pfree) {
  *(void **) (&pfree) = dlsym(RTLD_NEXT, "free");
  if ((char_error = dlerror()) != NULL)  {
   fprintf (stderr, "free %12.12s\n", char_error);
   exit(1);
  }
 }

 flags |= IPM_LIBPC_ACTIVE;
 flags |= IPM_LIBPC_INITIALIZED;
}

void libpc_finalize_sig(int sig) {
 flags &= ~IPM_LIBPC_ACTIVE;
 libpc_signal_caught = sig;
 if(flags & DEBUG) {
  printf("IPM: libpc_finalize_sig enter\n"); fflush(stdout);
 }
 libpc_finalize();
 if(flags & DEBUG) {
  printf("IPM: libpc_finalize_sig exit\n"); fflush(stdout);
 }
}

void libpc_finalize(void) {
 flags &= ~IPM_LIBPC_ACTIVE;
 flags |= IPM_LIBPC_FINALIZING;

 if(flags & DEBUG) {
  printf("IPM: libpc_finalize enter\n"); fflush(stdout);
 }

 libpc_report();

 if(flags & DEBUG) {
  printf("IPM: libpc_finalize exit\n"); fflush(stdout);
 }
 exit(libpc_signal_caught);
}

void libpc_report(void) {
 int i;
 unsigned long long int buff_size=1;

 flags &= ~IPM_LIBPC_ACTIVE;

 if(flags & DEBUG) {
  printf("IPM: libpc_report enter\n"); fflush(stdout);
 }

 if(flags & DEBUG) {
  printf("IPM: libpc_report exit\n"); fflush(stdout);
 }

 flags |= IPM_LIBPC_ACTIVE;
}

static void libpc_clear(void) {
 int i;

}

/*
 *
 * Profiled functions follow
 *
 */

void *malloc(size_t size) {
  int bin,i;
  void *handle,*rv;
  double wall;
  IPM_TICK_TYPE T1,T2;


/*
  LIBPC_COND_DIVERT(pmalloc);
*/

  if(!(flags & IPM_LIBPC_INITIALIZED))  libpc_init();

  if(flags & IPM_LIBPC_ACTIVE) {
   flags &= ~IPM_LIBPC_ACTIVE;
  } else {
   rv = pmalloc(size);
  } 

/*
*/

  HARNESS_TRACE_RV(pmalloc(size),malloc(size));

#define HARNESS_TRACE_RV(pfunc,func) {
  IPM_TIME_TICK(T1);\
  rv = pfunc;\
  IPM_TIME_TICK(T2);\
  if(flags & IPM_TRC_MEM) {\
   printf("malloc size = %d %12.6f %12.6f\n",\
	 size,\
	 IPM_TIME_SEC(T1),\
	 IPM_TIME_SEC(T2));\
  }\
  wall = IPM_TIME_SEC(T2) -  IPM_TIME_SEC(T2);\
  flags |= IPM_LIBPC_ACTIVE;\
  return rv;\
}

  IPM_TIME_TICK(T1);
  rv = pmalloc(size);
  IPM_TIME_TICK(T2);
  if(flags & IPM_TRC_MEM) {
   printf("malloc size = %d %12.6f %12.6f\n",
	 size,
	 IPM_TIME_SEC(T1),
	 IPM_TIME_SEC(T2));
  }
  wall = IPM_TIME_SEC(T2) -  IPM_TIME_SEC(T2);
  flags |= IPM_LIBPC_ACTIVE;
  return rv;
}

void *free(void *ptr) {
  int bin,i;
  void *handle,*rv;
  double wall;
  IPM_TICK_TYPE T1,T2;


  if(!(flags & IPM_LIBPC_INITIALIZED))  libpc_init();

  if(flags & IPM_LIBPC_ACTIVE) {
   flags &= ~IPM_LIBPC_ACTIVE;
  } else {
   pfree(ptr);
  } 

  IPM_TIME_TICK(T1);
  pfree(ptr);
  IPM_TIME_TICK(T2);
  if(flags & IPM_TRC_MEM) {
   IPM_LIBPC_PAUSE;
   printf("free    size = ? %12.6f %12.6f\n",
	 IPM_TIME_SEC(T1),
	 IPM_TIME_SEC(T2));
   IPM_LIBPC_RESUME;
  }
  wall = IPM_TIME_SEC(T2) -  IPM_TIME_SEC(T2);
  GETBIN(size,bin); 
  mhist.fcount[bin]++;
  mhist.fttot[bin] += wall;
  if(wall > mhist.ftmax[bin])  mhist.ftmax[bin] = wall;
  if(wall < mhist.ftmin[bin])  mhist.ftmin[bin] = wall;

  flags |= IPM_LIBPC_ACTIVE;

}

int open (const char *file, int oflag, ...) {

}



