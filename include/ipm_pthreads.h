
#ifndef IPM_PTHREADS_H_INCLDUED
#define IPM_PTHREADS_H_INCLDUED

#ifdef IPM_MONITOR_PTHREADS 

#include "ipm_sizes.h"
#include "ipm_hpm.h"

typedef struct
{
  long long papi_ctrs[MAXSIZE_HPMCOUNTERS];
  long long papi_enter[MAXSIZE_HPMCOUNTERS];
  char papi_running;
  char active;
} ipm_ptstats_t;


extern ipm_ptstats_t ipm_ptstats[MAXSIZE_NTHREADS];

#endif 

#endif /* IPM_PTHREADS_H_INCLDUED */
