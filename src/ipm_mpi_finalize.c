/* Jie's Hack for executable information */
void remove_exe_info_files() {
    char *workdir, *jobid, ldd_file[1024], exe_file[1024];
    char f1[] = "executable_file_info.txt";
    char f2[] = "executable_ldd_info.txt";
    workdir = getenv("TMPDIR");
    jobid = getenv("PBS_JOBID");
    snprintf(exe_file, sizeof(exe_file), "%s/%s_%s", workdir, jobid, f1);
    snprintf(ldd_file, sizeof(ldd_file), "%s/%s_%s", workdir, jobid, f2);
    remove(exe_file);
    remove(ldd_file);
}
/* end of Jie's Hack */

int MPI_Finalize(void) {
 IPM_TICK_TYPE T1,T2;
 double t1=0,t2=0;
 int rv;
 if(task.flags & DEBUG) {
  printf("IPM: %d MPI_Finalize enter \n",task.mpi_rank);
 }
 ipm_finalize();
 IPM_TIME_GET(T1);
 rv = ipm_mpi_finalize();
 IPM_TIME_GET(T2);
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);
 IPM_TIME_RESOLUTION_CHECK(t1,t2);
 if(task.flags & DEBUG) {
  printf("IPM: %d MPI_Finalize leave : elapsed=%f\n", task.mpi_rank, t2-t1);
 }
 return rv;
}

#ifdef WRAP_FORTRAN
void MPI_FINALIZE_F(int *info) {
 IPM_TICK_TYPE T1,T2;
 double t1,t2;
 if(task.flags & DEBUG) {
  printf("IPM: %d mpi_finalize enter \n",task.mpi_rank);
 }
 IPM_TIME_GET(T1);
 *info = ipm_mpi_finalize();
 IPM_TIME_GET(T2);

 IPM_TIME_OVERFLOW_CHECK(T1,T2); /* will never overflow actually */
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);
 IPM_TIME_RESOLUTION_CHECK(t1,t2);

 if(task.flags & DEBUG) {
  printf("IPM: %d mpi_finalize leave : elapsed=%f\n",task.mpi_rank, t2-t1);
 }
}
#endif

static int ipm_mpi_finalize(void) { /* called once per execution */
                                    /* called from MPI_[Ff]inalize*/
    int                    rv,final_rv=-1;
    IPM_TICK_TYPE          T1,T2;
    char                   fname[80];
    
    if(task.flags & DEBUG) {
	printf("IPM: %d ipm_mpi_finalize enter\n", task.mpi_rank);
	fflush(stdout);
    }
    
/*******************/
/* set stamp_final */
/*******************/
    
    IPM_TIME_GTOD(task.stamp_final);
    time(&task.final_ts);
    task.stamp_final_mpi = task.stamp_final;
    task.final_ts_mpi = task.final_ts;
    task.flags &= ~IPM_APP_RUNNING;
    task.flags |= IPM_APP_COMPLETED;
    
    ipm_region(1,"ipm_noregion");
    if(task.flags & IPM_HPM_ACTIVE) { /* maybe */
	ipm_hpm_stop();
    }
    
    task.flags |= IPM_MPI_FINALIZING;
    
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
    
    IPM_TIME_BRACKET(ipm_report("IPM_MPI_FINALIZING")); 
    report_delta = IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1);
    if(task.flags & DEBUG) {
	printf("IPM: %d ipm_report took %.3e seconds\n",
	       task.mpi_rank,IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1)); fflush(stdout);
    }
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
	IPM_TIME_BRACKET(ipm_log()); /* destroys taskdata on rank 0 */ 
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_log took %.3e seconds\n",
		   task.mpi_rank,IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1)); fflush(stdout);
	}
    }
#endif
    
/* This is causing crashes on some archtectures */
/* Not sure why - some weird interaction with particular MPI's*/
/* MPI_Group_free(&ipm_world_group); */

    IPM_TIME_BRACKET(final_rv=PMPI_Finalize());
    if(task.flags & DEBUG) {
	printf("IPM: %d PMPI_Finalize took %.3e seconds\n",
	       task.mpi_rank,IPM_TIME_SEC(T2)-IPM_TIME_SEC(T1)); fflush(stdout);
    }
    
    task.flags |= IPM_MPI_FINALIZED;
    task.flags &= ~IPM_APP_RUNNING;
    
    /* Jie's Hack for executable information */
// remove_exe_info_files(); //in TMPDIR, no need to manually remove it.
    /* end of Jie's hack */

    /* unmap shared mmaps */

    if( IPM_MMAP_FILE != NULL ) {
	if ( munmap(share_map, MPI_LOCAL_SIZE*sizeof(struct ipm_stats)) == -1 ) {
	    printf("IPM: %d ipm_external error unmapping.\n",
    	       task.mpi_rank);
	    fflush(stdout);
	}
	
	close(map_fd);
    }
    
    return final_rv;
}


