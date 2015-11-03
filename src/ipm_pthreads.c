#define _GNU_SOURCE /* required to get RTLD_NEXT defined */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>

#include "config.h"
#include "ipm_hpm.h"
#include "ipm.h"

//#include "ipm_pthreads.h"

pthread_key_t    tid_key;
pthread_once_t   key_once = PTHREAD_ONCE_INIT;
pthread_mutex_t  mutex    = PTHREAD_MUTEX_INITIALIZER;
int              next_tid = 0;

extern ipm_pthread_data pthreads[MAXSIZE_NTHREADS];

typedef struct 
{
  void *(*func)(void *);
  void *args;
} pthread_func_t;


void ipm_pthread_begin(int tid);
void ipm_pthread_end(int tid);


/* get the linear TID associated with this thread from 
   thread local storage, get the next free tid and store it
   in TLS if there is currently no such association */
int ipm_pthread_get_tid()
{
  int tid;

  if( !(pthread_getspecific(tid_key)) )
    {
      int *mytid = (int*) malloc(sizeof(int)); 
      pthread_mutex_lock(&mutex);
      (*mytid) = next_tid++;
      pthread_mutex_unlock(&mutex);
      pthread_setspecific(tid_key, mytid);
    }

  tid = *((int*)(pthread_getspecific(tid_key)));
  
  return tid;
}


void* ipm_pthread_trampoline(void *arg)
{
  void *ret;
  int tid;
  pthread_t ptid;
  pthread_func_t *ptfunc = (pthread_func_t*) arg;
 
  ptid = pthread_self();
  tid  = ipm_pthread_get_tid();
  
  ipm_pthread_begin(tid);
  if( ptfunc && ptfunc->func && ptfunc->args )
    ret=(ptfunc->func)(ptfunc->args);
  ipm_pthread_end(tid);

  return ret;
}


void ipm_pthread_once()
{
  int i, j;

  pthread_key_create(&tid_key, 0);

  for( i=0; i<MAXSIZE_NTHREADS; i++ )
    {
      pthreads[i].papi_running=0;
      pthreads[i].active=0;
      for( j=0; j<MAXSIZE_HPMCOUNTERS; j++ )
	{
	  pthreads[i].papi_ctrs[j]=0;
	}
    }

  ipm_pthread_hpm_init();
}



/* overloaded pthread_create function */
#if 0
int pthread_create(pthread_t *ptid, 
                   const pthread_attr_t *attr, 
                   void *(*func)(void *), 
                   void *ptr)
{
  int rv;
  static int loaded=0; 
  static int (*real_pthread_create)(pthread_t *, 
				    const pthread_attr_t *, 
				    void *, void *);
  pthread_func_t *ptfunc;
  
  if(!loaded)
    {
      real_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
      
      if(!dlerror())
        loaded=1;
      else
        {
          fprintf(stderr, "Error locating pthread_create\n");
          return 0;
        }
    }

  /* make sure everything is initialized */
  pthread_once( &key_once, ipm_pthread_once );

  ipm_pthread_get_tid();

  /* set up data structure with original thread function 
     and data ptr and pass this to our trampoline function */
  ptfunc = (pthread_func_t*)malloc(sizeof(pthread_func_t));
  ptfunc->func = func;
  ptfunc->args = ptr;
  
  rv = real_pthread_create(ptid, attr, 
			   ipm_pthread_trampoline, ptfunc);

  return rv;
}
#endif

void ipm_pthread_begin(int tid)
{
  int i;

  if( !(pthreads[tid].papi_running) )
    {
      ipm_pthread_hpm_start(tid);
      pthreads[tid].papi_running=1;
      pthreads[tid].active=1;
    }

  ipm_pthread_hpm_read(tid, pthreads[tid].papi_enter );
}



void ipm_pthread_end(int tid)
{
  int i;
  long long papi_now[MAXSIZE_HPMCOUNTERS];

  if( (pthreads[tid].papi_running) )
    {
      ipm_pthread_hpm_read(tid, papi_now );
      
      for( i=0; i<MAXSIZE_HPMCOUNTERS; i++ )
	{
	  pthreads[tid].papi_ctrs[i]+=
	    (papi_now[i]-pthreads[tid].papi_ctrs[i]);
	}
    }

  /*
  fprintf(stderr, "HW counters for thread %d\n", tid);
  for( i=0; i<MAXSIZE_HPMCOUNTERS; i++ )
    {
      fprintf(stderr, "%lld ", pthreads[tid].papi_ctrs[i]);
    }
  fprintf(stderr, "\n");
  */
}
