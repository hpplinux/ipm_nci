#define do_flops(n) {for (i = 0; i < n; i++) {c += a * b;   } if(c<0.1 && c>0.2) printf("This statement is never reached.\n");}

#ifdef CPU_PPC450D
/* then we are on a bluegene P*/

/* this code is taken from the IBM redbook
   "IBM System Blue Gene Solution: 
    Performance Analysis Tools REDP-4256-01" and 
   figures out if you are the 0th task on EACH node
   as only 1 task per node should program and read the counters.
*/

#include <spi/kernel_interface.h>
#include <common/bgp_personality.h>

int node_rank;

void get_compute_node_id(int * node_number)
{
   int tx, ty, tz;
   int nx, ny, nz;
   _BGP_Personality_t personality;
   Kernel_GetPersonality(&personality, sizeof(personality));
   tx = personality.Network_Config.Xcoord;
   ty = personality.Network_Config.Ycoord;
   tz = personality.Network_Config.Zcoord;
   nx = personality.Network_Config.Xnodes;
   ny = personality.Network_Config.Ynodes;
   nz = personality.Network_Config.Znodes;
   *node_number = tx + ty*nx + tz*nx*ny;
}

void get_local_rank(int * local_rank)
{
   int node_id, procid;
   MPI_Comm node_comm;
   get_compute_node_id(&node_id);
   procid = Kernel_PhysicalProcessorID();
   PMPI_Comm_split(MPI_COMM_WORLD, node_id, procid, &node_comm);
   PMPI_Comm_rank(node_comm, local_rank);
}
#endif


static int ipm_hpm_test(int show_list) {
#ifdef HPM_PAPI
 char errstring[PAPI_MAX_STR_LEN];
 char event_name[PAPI_MAX_STR_LEN];
#endif


 int i,ii,j,k;
#ifndef HPM_NOOP
 volatile double a = 0.5, b = 2.2;
 double c = 0.11;
 int rv,nflop=1024;
 IPM_KEY_TYPE key;

 if(task.flags & IPM_HPM_CANCELED) {
  return 0;
 }

 if(task.flags & DEBUG) {
  for(j=0;j<MAXSIZE_NEVENTSETS;j++) {
   for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
    ii = ipm_hpm_eorder[j][i];
    printf("IPM: %d ipm_hpm_test eventset=%d event=%d ename=%s\n",
	task.mpi_rank,
	j,ii,ipm_hpm_ename[j][ii]);
   }
  }
 }

 for(i=0;i<MAXSIZE_REGION;i++) {
  for(j=0;j<MAXSIZE_NEVENTSETS;j++) {
   for(k=0;k<MAXSIZE_HPMCOUNTERS;k++) {
    task.hpm_count[i][j][k] = 0;
   }
  }
 }

#ifdef HPM_PAPI
/* Initialize the PAPI library */
 rv = PAPI_library_init(PAPI_VER_CURRENT);
 if (rv != PAPI_VER_CURRENT) {
 if(task.flags & DEBUG) {
  printf("IPM: %d ipm_hpm_test ERROR PAPI_library_init rv=%d!\n",task.mpi_rank,rv);
  }
  perror("PAPI_library_init");
  return 0;
 }

#ifdef CPU_PPC450D
  get_local_rank(&node_rank);
  if (node_rank != 0 ) return; 
#endif

  
  rv = PAPI_create_eventset(&papi_eventset[0]);
  if (rv != PAPI_OK) {
  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_hpm_test ERROR PAPI create_eventset rv=%d\n",task.mpi_rank,rv);
  }
   return 0;
  }

/* user defined HPM events are handled here */
/* In the case the user makes a mistake - it drops back to the corresponding */
/*  counter from set 1 & prints and error */

/* This is a change in logic from before - PAPI events are now selected as strings */
/* in the ipm_hpm.h file then converted to events codes here */
/* this allows  1) native event use 2) the uneccesary duplication of info in ipm_hpm.h */
  for (i=0;i< MAXSIZE_HPMCOUNTERS;i++){
      if (ipm_hpm_ename[task.hpm_eventset][i] != "            ") {
	  rv = PAPI_event_name_to_code(ipm_hpm_ename[task.hpm_eventset][i], &papi_event[task.hpm_eventset][i]);
	  if (rv != PAPI_OK) {
	      PAPI_perror(rv, errstring, PAPI_MAX_STR_LEN);
	      printf("IPM: %d papi_error error in user defined event rv=%d \"%s\"\n",
		     task.mpi_rank,
		     rv,
		     PAPI_strerror(rv));
	      PAPI_event_name_to_code(ipm_hpm_ename[0][i], &papi_event[task.hpm_eventset][i]);
	      ipm_hpm_ename[task.hpm_eventset][i]=ipm_hpm_ename[0][i];
	  }
      } else {
	  papi_event[task.hpm_eventset][i] =-1;
      }
  }

  for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
      ii = ipm_hpm_eorder[task.hpm_eventset][i];
      if (papi_event[task.hpm_eventset][ii] != -1) {
	  rv = PAPI_add_event(papi_eventset[0], papi_event[task.hpm_eventset][ii]);
	  if (rv != PAPI_OK) {
	      if(task.flags & DEBUG) {
		  printf("IPM: %d ipm_hpm_test ERROR PAPI_add_event %s (%ld) rv=%d %s\n",
			 task.mpi_rank,
			 ipm_hpm_ename[task.hpm_eventset][ii],
			 (long int)papi_event[ii],rv,PAPI_strerror(rv));
	      }
	      return 0;
	  }
      }
  }

  rv = PAPI_start(papi_eventset[0]);
  if (rv != PAPI_OK) {
  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_hpm_test ERROR PAPI_start rv=%d\n",task.mpi_rank,rv);
  }
   return 0;
  }

#endif

#ifdef HPM_PMAPI
#ifdef AIX51
 rv = pm_init(PM_VERIFIED|PM_UNVERIFIED|PM_CAVEAT|PM_GET_GROUPS, &pmapi_info, &pmapi_groups_info);
#else 
 rv = pm_initialize(PM_VERIFIED|PM_UNVERIFIED|PM_CAVEAT|PM_GET_GROUPS, &pmapi_info, &pmapi_groups_info,PM_CURRENT);
#endif
 
 if(rv) {
  if(task.flags & DEBUG) {
  printf("IPM: %d ipm_hpm_test ERROR pm_initialize rv=%d\n",task.mpi_rank,rv); 
  }
   pm_error("ipm_hpm_test : pm_initialize",rv);
   return 0;
 }

 for(i=0;i<pmapi_info.maxpmcs;i++)
            pmapi_prog.events[i]=COUNT_NOTHING;
   pmapi_prog.mode.w          = 0;
   pmapi_prog.mode.b.user     = 1;
   pmapi_prog.mode.b.proctree = 1;
/* power3 does not use pmapi groups power4 and up do */
#ifndef POWER3
   pmapi_prog.mode.b.is_group  = 1;
   pmapi_prog.events[0]       = task.hpm_eventset;
#ifdef CPU_POWER6
/* use all the pm_hpm* groups 186 - 195 */ 
    pmapi_prog.events[0] = 185 + task.hpm_eventset;
    int event;
    i= pmapi_prog.events[0];
    if(task.flags & DEBUG) {
       printf ("IPM: %d Group #%d: %s\n",task.mpi_rank,i, pmapi_groups_info.event_groups[i].short_name);
       printf ("IPM: %d Group name: %s\n", task.mpi_rank,pmapi_groups_info.event_groups[i].long_name);
       printf ("IPM: %d Group description: %s\n",task.mpi_rank, pmapi_groups_info.event_groups[i].long_name);
       printf ("IPM: %d Group members:\n",task.mpi_rank);
    }
    for (int counter = 0; counter < pmapi_info.maxpmcs; counter++) {
          /* get the event id from the list */
          event = pmapi_groups_info.event_groups[i].events[counter];
          if ((event == COUNT_NOTHING) || (pmapi_info.maxevents[counter] == 0))
             printf("event %2d: No event\n", event);
          else {  
             /* find pointer to the event */
             for (j = 0; j < pmapi_info.maxevents[counter]; j++) { 
                evp = pmapi_info.list_events[counter]+j;  
                if (event == evp->event_id) {     
                   break;    
                }             
             }             
             if(task.flags & DEBUG) {
                printf("IPM: %d Counter %2d, ", task.mpi_rank,counter+1);
                printf("event %2d: %s", event, evp->short_name);
                printf(" : %s\n", evp->long_name);
             }
             strcpy (ipm_hpm_ename[task.hpm_eventset][counter],evp->short_name); 
          }
    } 

#else
   if(task.hpm_eventset == 0) { pmapi_prog.events[0]       = 60; }
   if(task.hpm_eventset == 1) { pmapi_prog.events[0]       = 59; }
   if(task.hpm_eventset == 2) { pmapi_prog.events[0]       =  5; }
   if(task.hpm_eventset == 3) { pmapi_prog.events[0]       = 58; }
   if(task.hpm_eventset == 4) { pmapi_prog.events[0]       = 53; }
#endif
#else 
   for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
    pmapi_prog.events[i] = pmapi_event[task.hpm_eventset][i];
   }
#endif
   rv = pm_set_program_mythread(&pmapi_prog);
   if(rv) {
  if(task.flags & DEBUG) {
    printf("IPM: %d ipm_hpm_test ERROR pm_set_program_mythread  rv=%d\n",task.mpi_rank,rv); 
    pm_error("ipm_hpm_test : pm_set_program_mythread",rv);
  }
    return 0;
   }
   rv = pm_start_mythread();
   if(rv) {
  if(task.flags & DEBUG) {
    printf("IPM: %d ipm_hpm_test ERROR pm_start_mythread  rv=%d\n",task.mpi_rank,rv); 
    pm_error("ipm_hpm_test : pm_start_mythread",rv);
    }
    return 0;
   }
#endif

/*  do_flops(nflop); */

/*
  for(i=0;i<1000024; i++) {
  IPM_MPI_KEY(key,15,127,i,(INT_MAX-1));
  }
*/

  if(show_list) {
   for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
   ii = ipm_hpm_eorder[task.hpm_eventset][i];
   printf("IPM: %d ipm_hpm_test event_set %d event_name %s\n", task.mpi_rank,
	task.hpm_eventset,
	ipm_hpm_ename[task.hpm_eventset][ii]);
   }
  }

#ifdef HPM_PAPI
  rv = PAPI_read(papi_eventset[0], task.hpm_count[0][task.hpm_eventset] );
  if (rv != PAPI_OK) {
  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_hpm_test ERROR PAPI_read rv=%d\n",task.mpi_rank,rv);
  }
   return 0;
  }
  rv = PAPI_reset(papi_eventset[0]);
  if (rv != PAPI_OK) {
  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_hpm_test ERROR PAPI_reset rv=%d\n",task.mpi_rank,rv);
  }
   return 0;
  }
  rv = PAPI_stop(papi_eventset[0], task.hpm_count[0][task.hpm_eventset]);
  if(rv  != PAPI_OK) {
  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_hpm_test ERROR PAPI_stop rv=%d\n",task.mpi_rank,rv);
  }
   return 0;
  }
#endif

#ifdef HPM_PMAPI
   rv = pm_stop_mythread();
   if(rv) {
  if(task.flags & DEBUG) {
    printf("IPM: %d ipm_hpm_test ERROR pm_stop_mythread  rv=%d\n",task.mpi_rank,rv); 
  }
    return 0;
   }
   rv = pm_get_data_mythread(&pmapi_data);
   if(rv) {
  if(task.flags & DEBUG) {
    printf("IPM: %d ipm_hpm_test ERROR pm_get_data_mythread  rv=%d\n",task.mpi_rank,rv); 
  }
    return 0;
   }
   for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
    task.hpm_count[0][task.hpm_eventset][i] = pmapi_data.accu[i];
   }
#endif

 if(task.flags & DEBUG) {
   for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
    ii = ipm_hpm_eorder[task.hpm_eventset][i];
  if(task.flags & DEBUG) {
    printf("IPM: %d ipm_hpm_test count %s %lld\n",
	task.mpi_rank,
	ipm_hpm_ename[task.hpm_eventset][ii],
	task.hpm_count[0][task.hpm_eventset][ii]);
   }
   }
 }

 for(i=0;i<MAXSIZE_REGION;i++) {
  for(j=0;j<MAXSIZE_NEVENTSETS;j++) {
   for(k=0;k<MAXSIZE_HPMCOUNTERS;k++) {
    task.hpm_count[i][j][k] = 0;
   }
  }
 }
#ifdef HPM_PAPI
 PAPI_cleanup_eventset(papi_eventset[0]);
 papi_eventset[0] = PAPI_NULL;
 /*PAPI_shutdown();*/
#endif
#ifdef HPM_PMAPI
   rv = pm_delete_program_mythread();
   if(rv) {
  if(task.flags & DEBUG) {
    printf("IPM: %d ipm_hpm_test ERROR pm_delete_program_mythread  rv=%d\n",task.mpi_rank,rv); 
  }
    return 0;
   }
#endif

 if(task.flags & DEBUG) {
    printf("IPM: %d ipm_hpm_test SUCCESS\n",task.mpi_rank); 
 }

  return 1 ;
#else 
/* above else is NOOP */
  return 0 ;
#endif
}

#undef do_flops

