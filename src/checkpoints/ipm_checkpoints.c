/*
 * This is an additional functionality for IPM, which:
 *
 * (1) enables support for checkpoint-restart applications. (this file)
 * (2) share some information with ANUPBS (ipm_external.c)
 *
 * Author: Jie Cai @ ANUSF, Jun. 2011
 */

double prev_cp_stamp=0.0;

#include "ipm_chkpt_log.c"

static int ipm_mpi_checkpoint(void) { 
                                    
    IPM_TICK_TYPE          T1,T2;
    long long int  checkpoint_hpm[MAXSIZE_HPMCOUNTERS];
    double cp_interval=0;
    
    if(task.flags & DEBUG) {
	printf("IPM: %d ipm_mpi_checkpoint enter\n", task.mpi_rank);
	fflush(stdout);
    }

    if(IPM_CHECKPOINT == 0) {
	return 0;
    }    
    
    IPM_TIME_GTOD(task.stamp_final);
    time(&task.final_ts);
    task.stamp_final_mpi = task.stamp_final;
    task.final_ts_mpi = task.final_ts;

    if(CHKPT_INTERVAL <= 0) {
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_mpi_checkpoint: invalid checkpoint interval (%f) detected.\n", 
		   task.mpi_rank, CHKPT_INTERVAL);
	    fflush(stdout);
	}
	return -1;
    }

    if(prev_cp_stamp == 0.0) {
	prev_cp_stamp = task.stamp_init;
    } 

    if( (task.stamp_final - prev_cp_stamp) < CHKPT_INTERVAL) {
	return 0;
    }

    prev_cp_stamp = task.stamp_final;

    ipm_region(1,"ipm_noregion");

    /* if(task.flags & IPM_HPM_ACTIVE) { /\* maybe *\/ */
    /*     ipm_hpm_read(checkpoint_hpm); */
    /* } */
    
    if(!( task.flags & IPM_MPI_INITIALIZED)) {
	printf("IPM: %d ERROR IPM already ended (region %s)\n",
	       task.mpi_rank, task.region_label[task.region_current]);
    }
    
    
    if(task.flags & IPM_REGIONALIZED) {
	ipm_region(-1,task.region_label[task.region_current]);
    }
    
    task.region_current = 0;
    
    ipm_get_switchbytes(&(task.switch_bytes_tx),&(task.switch_bytes_rx));
    task.switch_bytes_tx = task.switch_bytes_tx - switch_bytes_tx_init;
    task.switch_bytes_rx = task.switch_bytes_rx - switch_bytes_rx_init;
    
    
    IPM_TIME_BRACKET(ipm_sync());
    if(task.flags & DEBUG) {
	printf("IPM: %d ipm_sync took %.3e seconds\n",
	       task.mpi_rank,IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1)); fflush(stdout);
    }
    

/* this is where reduce is used to print report ipm in standard output */
    /* IPM_TIME_BRACKET(ipm_report("IPM_MPI_FINALIZING"));  */
    /* report_delta = IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1); */
    /* if(task.flags & DEBUG) { */
    /* 	printf("IPM: %d ipm_report took %.3e seconds\n", */
    /* 	       task.mpi_rank,IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1)); fflush(stdout); */
    /* } */

#ifndef IPM_DISABLE_LOG
    unsetenv("LD_PRELOAD");
    if(task.ipm_trc_count) {
	PMPI_Barrier(MPI_COMM_WORLD);
	IPM_TIME_BRACKET(ipm_trc_dump());
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_trc_dump took %.3e seconds\n",
		   task.mpi_rank,IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1)); fflush(stdout);
	}
    }
    if(!(task.flags & LOG_NONE)) {
	IPM_TIME_BRACKET(ipm_chkpt_log()); /* destroys taskdata on rank 0 */ 
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_chkpt_log took %.3e seconds\n",
		   task.mpi_rank,IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1)); fflush(stdout);
	}
    }
#endif
    
    return 1;
}

