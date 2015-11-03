/***************************************/
/* IPM library HPM API                 */
/***************************************/

#include "config.h"

#ifdef IPM_MONITOR_PTHREADS
#include <pthread.h>
#endif

#ifdef IPM_MONITOR_PTHREADS
void ipm_hpm_start() {}
void ipm_hpm_read(long long int *count) { }
void ipm_hpm_stop() {}
#endif

#ifdef CPU_PPC450D
extern int node_rank;
#endif

#ifndef IPM_MONITOR_PTHREADS

void ipm_hpm_start() {
    int i,j,k,rv;


#ifndef HPM_DISABLED

#ifdef HPM_PAPI
    char errstring[PAPI_MAX_STR_LEN];
    char event_name[PAPI_MAX_STR_LEN];
#endif

    if(task.hpm_eventset < 0) {
	return;
    }

    for(i=0;i<MAXSIZE_REGION;i++) {
	for(j=0;j<MAXSIZE_NEVENTSETS;j++) {
	    for(k=0;k<MAXSIZE_HPMCOUNTERS;k++) { 
		task.hpm_count[i][j][k] = 0;
	    }
	}
    }

#ifdef HPM_PAPI
/* Initialize the low level PAPI library */

    rv = PAPI_library_init(PAPI_VER_CURRENT);
    if(rv != PAPI_VER_CURRENT) {
	printf("IPM: %d papi_error library_init in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	perror("PAPI_library_init");
    } 
    if(rv == PAPI_OK) {
	if(task.flags & DEBUG && task.mpi_rank==0) {
	    printf("IPM: %d PAPI_library_init in hpm_start rv=%d \"%s\"\n",
		   task.mpi_rank,
		   rv,
		   PAPI_strerror(rv));
	}
    }

#ifdef CPU_PPC450D
/* then we are on a bluegene P*/
    if (node_rank != 0 ) return;
#endif

    rv = PAPI_num_counters();
    if(rv < 0) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error num_counters in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
    }

    if ((hwinfo = PAPI_get_hardware_info()) == NULL) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error PAPI_get_hardware_info in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
    } else {
	/* do something clever wrt. formal machine description */
    }

    rv = PAPI_create_eventset(&papi_eventset[0]);
    if(rv != PAPI_OK) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error create_eventset in hpmstart rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
    }

    if(0) {
	printf("PAPI: JIE_DEBUG:: rank %d task.hpm_eventset is %d.\n",
	       task.mpi_rank, task.hpm_eventset);
	fflush(stdout);
    }

    for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
       
	if (papi_event[task.hpm_eventset][i] != -1) {
	    rv = PAPI_query_event(papi_event[task.hpm_eventset][i]);

	    if (rv != PAPI_OK) {
		PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
		PAPI_event_code_to_name(papi_event[task.hpm_eventset][i],event_name);
		printf("IPM: %d papi_error query_event %s %d \"%s\"\n",
		       task.mpi_rank, 
		       event_name,
		       rv,
		       PAPI_strerror(rv));
	    }
	   
	    rv = PAPI_add_event(papi_eventset[0], papi_event[task.hpm_eventset][i]);
	    if(rv != PAPI_OK) {
		PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
		PAPI_event_code_to_name(papi_event[task.hpm_eventset][i],event_name);
		printf("IPM: %d papi_error add_event %s %d \"%s\"\n",
		       task.mpi_rank, 
		       event_name,
		       rv,
		       PAPI_strerror(rv));
	    }

	    if(0) {
		PAPI_event_code_to_name(papi_event[task.hpm_eventset][i],event_name);
		printf("PAPI: JIE_DEBUG:: rank %d add event %s.\n",
		       task.mpi_rank, event_name);
		fflush(stdout);
	    }
	}
    }
    rv = PAPI_start(papi_eventset[0]);
    if (rv != PAPI_OK) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error: start in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
	task.flags &= ~IPM_HPM_ACTIVE;
    }

#endif

#ifdef HPM_PMAPI
#ifdef AIX51
    rv = pm_init(PM_VERIFIED|PM_UNVERIFIED|PM_CAVEAT|PM_GET_GROUPS, &pmapi_info, &pmapi_groups_info);
#else 
    rv = pm_initialize(PM_VERIFIED|PM_UNVERIFIED|PM_CAVEAT|PM_GET_GROUPS, &pmapi_info, &pmapi_groups_info,PM_CURRENT);
#endif
 
    if(rv) {
	printf("IPM: %d pmapi_error: pm_initialize \n",
	       task.mpi_rank);
	pm_error("IPM: pmapi : pm_initialize",rv);
    }

    for(i=0;i<pmapi_info.maxpmcs;i++)
	pmapi_prog.events[i]=COUNT_NOTHING;
    pmapi_prog.mode.w          = 0;
    pmapi_prog.mode.b.user     = 1;
    pmapi_prog.mode.b.proctree = 1;

#ifndef POWER3

#ifdef CPU_POWER4
    pmapi_prog.mode.b.is_group  = 1;
    if(task.hpm_eventset == 0) { pmapi_prog.events[0]       = 60; }
    if(task.hpm_eventset == 1) { pmapi_prog.events[0]       = 59; }
    if(task.hpm_eventset == 2) { pmapi_prog.events[0]       =  5; }
    if(task.hpm_eventset == 3) { pmapi_prog.events[0]       = 58; }
    if(task.hpm_eventset == 4) { pmapi_prog.events[0]       = 53; }
#endif

#ifdef CPU_POWER5
    pmapi_prog.mode.b.is_group  = 1;
    if(task.hpm_eventset == 0) { pmapi_prog.events[0]       = 137; }
#endif

#ifdef CPU_POWER6
    pmapi_prog.mode.b.is_group  = 1;
/* use all the pm_hpm* groups 186 - 195 */
    pmapi_prog.events[0] = 186 + task.hpm_eventset;
#endif

#else 

    for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
	pmapi_prog.events[i] = pmapi_event[task.hpm_eventset][i];
    }
#endif

    rv = pm_set_program_mythread(&pmapi_prog);
    if(rv) {
	printf("IPM: %d pmapi_error: pm_set_program_mythread\n",
	       task.mpi_rank);
	pm_error("IPM: pmapi : pm_set_program_mythread",rv);
    }
    rv = pm_start_mythread();
    if(rv) {
	printf("IPM: %d pmapi_error: pm_start_mythread\n",
	       task.mpi_rank);
	pm_error("IPM: pmapi : pm_start_mythread",rv);
	task.flags &= ~IPM_HPM_ACTIVE;
    }
    rv = pm_reset_data_mythread();
    if(rv) {
	printf("IPM: %d pmapi_error: pm_reset_data_mythread\n",
	       task.mpi_rank);
    }
#endif


#endif
    return;
}

void ipm_hpm_read(long long int *count) { /* read the counters */
    /* assumes hpm is IPM_HPM_ACTIVE     */
 
    int i;

#ifndef HPM_DISABLED
#ifdef HPM_PAPI

#ifdef CPU_PPC450D
/* then we are on a bluegene P*/
    if (node_rank != 0 ) return;
#endif

    PAPI_read(papi_eventset[0], count);
#endif
#ifdef HPM_PMAPI
    pm_get_data_mythread(&pmapi_data);
    for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
	count[i] = pmapi_data.accu[i];
    }
#endif
#endif
    return;
}

void ipm_hpm_stop() { 
#ifndef HPM_DISABLED
    int 		i,rv;
#ifdef HPM_PAPI
    char 		errstring[PAPI_MAX_STR_LEN];
    char 		event_name[PAPI_MAX_STR_LEN];
    long long int  papi_hpm[MAXSIZE_HPMCOUNTERS];
#endif

    if(task.flags & IPM_HPM_CANCELED) {
	return;
    }

    if(task.hpm_eventset < 0) {
	return;
    }


#ifdef HPM_PAPI

#ifdef CPU_PPC450D
/* then we are on a bluegene P*/
    if (node_rank != 0 ) return;
#endif 

    rv = PAPI_stop(papi_eventset[0], papi_hpm);
    if(rv != PAPI_OK) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error hpm_stop %d %s\n",
	       task.mpi_rank,
	       rv,
	       errstring);
    }
#endif
#ifdef HPM_PMAPI
    pm_stop_mythread();
#endif

    /* TAG_EXIT: exit from global is different than exit from non global */
    if(task.flags & IPM_HPM_ACTIVE) {
	if(task.region_current > 0) {
	    printf("IPM: %d exit from region=%d > 0 \n",
		   task.mpi_rank, task.region_current);
	}
    }


    return;
#endif
}

#endif /* IPM_MONITOR_PTHREADS */

#ifdef IPM_MONITOR_PTHREADS

void ipm_pthread_hpm_init() 
{
    int i,j,k,rv;
  
#ifdef HPM_PAPI
    char errstring[PAPI_MAX_STR_LEN];
    char event_name[PAPI_MAX_STR_LEN];
#endif
  
    //task.flags |= DEBUG; 

    if(task.hpm_eventset < 0) {
	return;
    }
  
    for(i=0;i<MAXSIZE_REGION;i++) {
	for(j=0;j<MAXSIZE_NEVENTSETS;j++) {
	    for(k=0;k<MAXSIZE_HPMCOUNTERS;k++) { 
		task.hpm_count[i][j][k] = 0;
	    }
	}
    }
  
#ifdef HPM_PAPI
    for( i=0; i<MAXSIZE_NTHREADS; i++ )
    {
	papi_eventset[i]=PAPI_NULL;
    }

  
    /* Initialize the low level PAPI library */
  
    rv = PAPI_library_init(PAPI_VER_CURRENT);
    if(rv != PAPI_VER_CURRENT) {
	printf("IPM: %d papi_error library_init in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	perror("PAPI_library_init");
    } 
  
  
    rv = PAPI_thread_init(pthread_self);
    if( rv != PAPI_OK )
	if(task.flags & DEBUG && task.mpi_rank==0) {
	    printf("IPM: %d PAPI_thread_init in hpm_start rv=%d \"%s\"\n",
		   task.mpi_rank,
		   rv,
		   PAPI_strerror(rv));
	    PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	    perror("PAPI_thread_init");
	}


    rv = PAPI_num_counters();

    if(rv < 0) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error num_counters in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
    }

    if ((hwinfo = PAPI_get_hardware_info()) == NULL) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error PAPI_get_hardware_info in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
    } else {
	/* do something clever wrt. formal machine description */
    }
  
    return;
}
#endif


void ipm_pthread_hpm_start(int tid) 
{
#ifdef HPM_PAPI
    int rv, i; 

    char errstring[PAPI_MAX_STR_LEN];
    char event_name[PAPI_MAX_STR_LEN];
#endif


    rv = PAPI_create_eventset(&papi_eventset[tid]);
    if(rv != PAPI_OK) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error create_eventset in hpmstart rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
    }

    for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) 
    {
	if (papi_event[task.hpm_eventset][i] != -1) {
	    rv = PAPI_query_event(papi_event[task.hpm_eventset][i]);
	    if (rv != PAPI_OK) {
		PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
		PAPI_event_code_to_name(papi_event[task.hpm_eventset][i],event_name);
		printf("IPM: %d papi_error query_event %s %d \"%s\"\n",
		       task.mpi_rank, 
		       event_name,
		       rv,
		       PAPI_strerror(rv));
	    }
	
	    rv = PAPI_add_event(papi_eventset[tid], papi_event[task.hpm_eventset][i]);
	    if(rv != PAPI_OK) {
		PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
		PAPI_event_code_to_name(papi_event[task.hpm_eventset][i],event_name);
		printf("IPM: %d papi_error add_event %s %d \"%s\"\n",
		       task.mpi_rank, 
		       event_name,
		       rv,
		       PAPI_strerror(rv));
	    }
	

	    // PAPI_event_code_to_name(papi_event[task.hpm_eventset][i],event_name);
	    //	fprintf(stderr, "tid=%d added event %d '%s'\n", 
	    //tid, papi_event[task.hpm_eventset][i], event_name);

	}
    }

    rv = PAPI_start(papi_eventset[tid]);
    if (rv != PAPI_OK) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error: start in hpm_start rv=%d \"%s\"\n",
	       task.mpi_rank,
	       rv,
	       PAPI_strerror(rv));
	//    task.flags &= ~IPM_HPM_ACTIVE;
    }	
}


void ipm_pthread_hpm_read(int tid, long long int *count)
{
#ifdef HPM_PAPI
    int i;
  
    PAPI_read(papi_eventset[tid], count);
#endif

    return;
}


void ipm_pthread_hpm_stop(int tid)
{
#ifdef HPM_PAPI
    int rv;
    char 		 errstring[PAPI_MAX_STR_LEN];
    long long int  papi_hpm[MAXSIZE_HPMCOUNTERS];

    rv = PAPI_stop(papi_eventset[tid], papi_hpm);
    if(rv!=PAPI_OK) {
	PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	printf("IPM: %d papi_error hpm_stop %d %s\n",
	       task.mpi_rank,
	       rv,
	       errstring);
    }
#endif
}

#endif /* IPM_MONITOR_PTHREADS */


/*
7   7   -   -   -   -   5   5
  1   5   -   -   -   7   4
2   1   -   -   -   -   4   3
  2   -   -   -   -   -   3
1   5   -   -   -   -   7   4
  1   -   -   -   -   -   4
7   1   -   -   -   -   4   5
  7   5   -   -   -   7   5
*/
