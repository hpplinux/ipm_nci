/* Jie's hack */
/*     printf("IPM: %d MAXSIZE_HASH = %d\n", task.mpi_rank,MAXSIZE_HASH); */
/*     printf("IPM: %d MAXSIZE_HASHLIMIT = %d\n", task.mpi_rank,MAXSIZE_HASHLIMIT); */
/* #ifdef IPM_ENABLE_PROFLOW */
/*     printf("IPM_ENABLE_PROFLOW is defined!!\n"); */
/* #endif */
/* end of Jie's hack */

    if(task.flags & DEBUG && !task.mpi_rank) {
	printf("IPM: %d MAXSIZE_LABEL = %d\n", task.mpi_rank,MAXSIZE_LABEL);
	printf("IPM: %d MAXSIZE_HOSTNAME = %d\n", task.mpi_rank,MAXSIZE_HOSTNAME);
	printf("IPM: %d MAXSIZE_USERNAME = %d\n", task.mpi_rank,MAXSIZE_USERNAME);
	printf("IPM: %d MAXSIZE_FILENAME = %d\n", task.mpi_rank,MAXSIZE_FILENAME);
	printf("IPM: %d MAXSIZE_TXTLINE = %d\n", task.mpi_rank,MAXSIZE_TXTLINE);
	printf("IPM: %d MAXSIZE_CMDLINE = %d\n", task.mpi_rank,MAXSIZE_CMDLINE);
	printf("IPM: %d MAXSIZE_EVENTS = %d\n", task.mpi_rank,MAXSIZE_EVENTS);
	printf("IPM: %d MAXSIZE_HASH = %d\n", task.mpi_rank,MAXSIZE_HASH);
	printf("IPM: %d MAXSIZE_HASHLIMIT = %d\n", task.mpi_rank,MAXSIZE_HASHLIMIT);
	printf("IPM: %d MAXSIZE_CALLS = %d\n", task.mpi_rank,MAXSIZE_CALLS);
	printf("IPM: %d MAXSIZE_NEVENTSETS = %d\n", task.mpi_rank,MAXSIZE_NEVENTSETS);
	printf("IPM: %d MAXSIZE_HPMCOUNTERS = %d\n", task.mpi_rank,MAXSIZE_HPMCOUNTERS);
	
	if(!task.mpi_rank) {
	    printf("IPM: %d sizeof(taskdata)=%lld sizeof(jobdata)=%lld sizeof(hash)=%lld\n",
		   task.mpi_rank,
		   (long long int)sizeof(task),
		   (long long int)sizeof(job),
		   (long long int)sizeof(ipm_hash_ent)*MAXSIZE_HASH);
	}
	
	fflush(stdout);
    }
