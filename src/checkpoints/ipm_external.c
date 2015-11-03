/*
 * This is an additional functionality for IPM, which:
 *
 * (1) enables support for checkpoint-restart applications. (ipm_checkpoints.c)
 * (2) share some information with ANUPBS with mmap(this file)
 *
 * Author: Jie Cai @ ANUSF, Jun. 2011
 */

double prev_share_stamp=0.0;

int ipm_pbs_share_mmap() {

    int  i;

    double mtime;
        
    /* this needs to be moved in to headers and ipm_env */
    if(IPM_MMAP_FILE == NULL || share_map == MAP_FAILED){
	return -1;
    }

    /*
     * write ipm stats into memory
     */
    for(i=0;i<=MAXSIZE_HASH;i++) {
	if(task.hash[i].key != 0) {
	    mtime += task.hash[i].t_tot;
	}
    }

    share_map[MPI_LOCAL_RANK].mtime = (unsigned long)(mtime*1e6);
//    share_map[MPI_LOCAL_RANK].pid = getpid();
    if(task.flags & DEBUG) {
	fprintf(stderr, "rank %d :: local_rank %d, pid %d, mtime %lu.\n", task.mpi_rank, MPI_LOCAL_RANK, share_map[MPI_LOCAL_RANK].pid, share_map[MPI_LOCAL_RANK].mtime);
    }

    return 0;
}



static int ipm_external_entry() {
    
    int ret;
    double ctime;
    IPM_TIME_GTOD(ctime);

    if(SHARE_INTERVAL <= 0) {
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_external_entry: invalid share interval (%s) detected.\n", 
		   task.mpi_rank, SHARE_INTERVAL);
	    fflush(stdout);
	}
	return -1;
    }

    if(prev_share_stamp == 0.0) {
	prev_share_stamp = ctime;
    }
    
    if ( (ctime - prev_share_stamp) < SHARE_INTERVAL) {
	return 0;
    }

    if(task.flags & DEBUG) {
	printf("IPM: %d SHARED_INTERVAL is %.2f, difference is %.2f.\n", 
	       task.mpi_rank, SHARE_INTERVAL, ctime-prev_share_stamp);
	fflush(stdout);
    }

    prev_share_stamp = ctime;
    
    
    ret = ipm_pbs_share_mmap();
    
    return ret;
}

