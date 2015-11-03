void ipm_log(void) { /* called by all tasks (or as many as possible) */
    int i,ii,rv,icall,ibytes,irank,ireg,kreg;
    int ipm_log_fd=-1, fan_out=0;
    int log_rank=-1, search_offset=0, token=1;
    FILE *ipm_mpi_log_fh;
    MPI_File ipm_mpiio_log_fh;
    DIR  *ipm_mpi_log_dp;
    struct dirent *de;
    struct stat file_stat;
    IPM_KEY_TYPE key,ikey;
    char *cp, txt[MAXSIZE_TXTLINE];
    char tmp_fname[MAXSIZE_FILENAME];
    char tmp_pref[MAXSIZE_FILENAME];
    char tmp_cmd[MAXSIZE_FILENAME];
    double b_flops;
    double stamp1, stamp2, stamp3, stamp4;
    MPI_Status s[4];
    MPI_Info outinfo;
    
    
    if(task.flags & IPM_WROTELOG) return;
    memset((void *)txt,0,(size_t)(MAXSIZE_TXTLINE*sizeof(char)));
    task.flags |= IPM_WROTELOG;
    
    /* only one chance, even if we fail at this point we should not return  */
    
    if(task.flags & DEBUG) {
	printf("IPM: %d log enter job.cookie=%s username=%s \n",
	       task.mpi_rank,
	       job.cookie,
	       job.username); fflush(stdout);
    }
    
/* 
** bail
*/
    
    
    if(strcmp(job.log_dir, "/dev/null") == 0 ) {
	if(task.flags & DEBUG) {
	    printf("IPM: %d log exit due to LOGDIR=/dev/null", task.mpi_rank);
	}
	return;
    }
    
    if(stat(job.log_dir,&file_stat)) {
	if(!task.mpi_rank) {
	    printf("IPM: %d log IPMLOG_DIR %s not available using $CWD \n",
		   task.mpi_rank,  job.log_dir);
	}
	sprintf(job.log_dir, "./");
    }
    
    
/*
** Aggregation method #1 : Multiple Files - No Aggregation 
IPM_LOG_USEMULTI
** Aggregation method #2 : Single File    - Locks
IPM_LOG_USELOCKS
** Aggregation method #3 : Single File    - SMP & /tmp
IPM_LOG_USETMPFS
** Aggregation method #4 : Single File    - MPI  - default
IPM_LOG_USEMPI
*/
    
#ifndef IPM_LOG_USEMULTI
#ifndef IPM_LOG_USELOCKS
#ifndef IPM_LOG_USETMPFS
#ifndef IPM_LOG_USEMPI
#endif
#endif
#endif
#endif
    
    
#ifdef IPM_LOG_USEMULTI
    sprintf(job.log_fname,"%s/%s.%s.%d", 
	    job.log_dir, 
	    job.username, 
	    job.cookie,
	    task.mpi_rank);
#else 
    if (!strcmp(job.log_fname,"unset")){
	sprintf(job.log_fname,"%s/%s.%s.%d", 
		job.log_dir, 
		job.username, 
		job.cookie,0);
    }
    else
    {
	sprintf(tmp_fname,"%s/%s",job.log_dir,job.log_fname);
	sprintf(job.log_fname,"%s",tmp_fname);
    }
#endif
    
    if(task.flags & DEBUG) {
	printf("IPM: %d log IPMLOG_DIR=%s FNAME=%s \n",
	       task.mpi_rank, 
	       job.log_dir, 
	       job.log_fname);
    }
/*
** Aggregation method #1 : Multiple Files - No Aggregation  {
*/
    
#ifdef IPM_LOG_USEMULTI
/* simplest case no locking just write each file. Parallel FS may 
   have metadata storm for N file creates */
    
    ipm_mpi_log_fh = fopen(job.log_fname,"w");
    if(ipm_mpi_log_fh == NULL) {
	printf("IPM: %d log fopen failed fname=%s \n",
	       task.mpi_rank, 
	       job.log_fname); fflush(stdout);
    }
    rv = fprintf(ipm_mpi_log_fh,
		 "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<ipm>\n"); 
    ipm_log_write_task(&job, &task, txt, ipm_mpi_log_fh);
    rv = fprintf(ipm_mpi_log_fh,
		 "</ipm>\n"); 
    fclose(ipm_mpi_log_fh);
    chmod(job.log_fname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    
    return;
#endif
/* } */
    
/*
** Aggregation methods #2 : Single File    - Locks {
*/
#ifdef IPM_LOG_USELOCKS
    signal(SIGALRM, ipm_alarm_log_block);
    alarm(30);
    IPM_TIME_GTOD(stamp1);
    if(task.flags & DEBUG) {
	printf("IPM: %d log block_lock fname=%s fd=%d stamp=%f \n",
	       task.mpi_rank, 
	       job.log_fname,
	       ipm_log_fd,
	       stamp1
	    ); fflush(stdout);
    }
    
    IPM_FILE_LOCK(job.log_fname,ipm_log_fd);
    
    IPM_TIME_GTOD(stamp2);
    if(task.flags & DEBUG) {
	printf("IPM: %d log block_lock fname=%s fd=%d stamp=%12.6f delta=%.3e \n",
	       task.mpi_rank, 
	       job.log_fname,
	       ipm_log_fd,
	       stamp2, stamp2-stamp1
	    ); fflush(stdout);
    }
    
    
    alarm(0);
    signal(SIGALRM, SIG_DFL);
    
    ipm_mpi_log_fh = fdopen(ipm_log_fd,"w+");
    
    if(!ipm_mpi_log_fh || !ipm_log_fd) {
/* fail silently */
	return;
    }
    
/* got log fh */
    fseek(ipm_mpi_log_fh,0,SEEK_END);
    ipm_log_write_task(&job, &task, txt, ipm_mpi_log_fh);
    
    IPM_TIME_GTOD(stamp3);
    if(task.flags & DEBUG) {
	printf("IPM: %d log write fname=%s fd=%d stamp=%12.6f delta=%.3e \n",
	       task.mpi_rank, 
	       job.log_fname,
	       ipm_log_fd,
	       stamp3, stamp3-stamp2
	    ); fflush(stdout);
    }
    
    
    fflush(ipm_mpi_log_fh);
    IPM_FILE_UNLOCK(job.log_fname,ipm_log_fd);
    
    IPM_TIME_GTOD(stamp4);
    if(task.flags & DEBUG) {
	printf("IPM: %d log unlock fname=%s fd=%d stamp=%12.6f delta=%.3e \n",
	       task.mpi_rank, 
	       job.log_fname,
	       ipm_log_fd,
	       stamp4, stamp4-stamp3
	    ); fflush(stdout);
    }
    
#endif
/* } */
    
/*
** Aggregation method #3 : Single File    - SMP & /tmp {
*/
#ifdef IPM_LOG_USETMPFS
    
    if(task.flags & IPM_MPI_FINALIZING) {
	if(task.mpi_size == 1) { /* special easy case  now uneeded */
	}
	
	sprintf(tmp_fname,"/tmp/%s.%s.%d",
		job.username,
		job.cookie,
		task.mpi_rank);
	ipm_mpi_log_fh = fopen(tmp_fname,"w");
	if(ipm_mpi_log_fh == NULL) {
	    printf("IPM: %d log fopen failed fname=%s \n",
		   task.mpi_rank, 
		   tmp_fname); fflush(stdout);
	}
	chmod(tmp_fname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	ipm_log_write_task(&job, &task, txt, ipm_mpi_log_fh);
	fclose(ipm_mpi_log_fh);
	
	
	/* host local ring barrier so that /tmp is all good */
	
	if(task.intra_size > 1) {
	    if(task.intra_root == task.mpi_rank) {
		PMPI_Send(&i,1,MPI_INT,task.intra_right,0,MPI_COMM_WORLD);
		PMPI_Recv(&i,1,MPI_INT,task.intra_left,0,MPI_COMM_WORLD,s);
	    } else {
		PMPI_Recv(&i,1,MPI_INT,task.intra_left,0,MPI_COMM_WORLD,s);
		PMPI_Send(&i,1,MPI_INT,task.intra_right,0,MPI_COMM_WORLD);
	    }
	}
	
	if(task.intra_root == task.mpi_rank) {
	    if(job.nhosts > 1 && task.mpi_rank) {
		PMPI_Recv(&i,1,MPI_INT,task.inter_left,0,MPI_COMM_WORLD,s);
	    }
	    /* sh -c lacks PATH on some system so remove popen&system where possible */
	    
	    sprintf(tmp_cmd, "/usr/bin/cat /tmp/%s.%s.* >> %s",
		    job.username,
		    job.cookie,
		    job.syslog_fname);
	    system(tmp_cmd);
	    
	    chmod(job.syslog_fname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	    sprintf(tmp_cmd, "/usr/bin/rm -f /tmp/%s.%s.* ",
		    job.username,
		    job.cookie);
	    system(tmp_cmd);
	    
/* ugh! duplicating cat and rm is yucky
   sprintf(tmp_pref,"%s.%s", job.username, job.cookie);
   dp=opendir("/tmp");
   if(dp) {
   while(de=readdir(dp))!=NULL){
   if(!strncmp(de->d_name,tmp_pref, strlen(tmp_fname))) {
   sprintf(tmp_fname,"/tmp/%s", de->d_name);
   fopen(tmp_fname,"r"
   read in pieces and write them to the intra-node file
   delete the .rank file
   } 
   }
   }
*/
	    
	    if(job.nhosts > 1 && task.inter_right != 0) {
		PMPI_Send(&i,1,MPI_INT,task.inter_right,0,MPI_COMM_WORLD);
	    }
	    
	    
	} 
	return;
    }
#endif
/* } */
    
/*
** Aggregation method #4 : Single File    - MPI  {
*/
#ifdef IPM_LOG_USEMPI
    
    if (task.flags & PARALLEL_IO_LOG ) {
	int buff_size=0;
	MPI_Offset file_offset=0;
	int64_t buff_sum=0;
	int malloc_flag,malloc_sum;
	char* buffer=NULL;
	MPI_Info info;
	

/* measure size of buff required */
	buff_size=ipm_log_write(&job, &task, txt, buffer,0,1);
	
	malloc_flag=1;
	buffer = (char*)malloc(buff_size+1);
	if (buffer == NULL) {
	    malloc_flag=0;
	} else {
	    rv=ipm_log_write(&job, &task, txt, buffer,buff_size,1);
	}
	
	/*see whether malloc suceeded across all mpi tasks */
	PMPI_Allreduce(&malloc_flag,&malloc_sum,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
	if (malloc_sum == task.mpi_size)  {/* use parallel IO */
	    
	    if(task.flags & DEBUG && !task.mpi_rank) {
		printf("IPM: %d IPM report parallel IO used\n", task.mpi_rank);
	    }
	    
	    PMPI_Info_create(&info);
#ifndef CRAY_GPFS_BUG
	    PMPI_Info_set(info,"access_style","write_once");
	    PMPI_Info_set(info,"collective_buffering","true");
	    PMPI_Info_set(info,"file_perm","0644");
	    PMPI_Info_set(info,"romio_cb_read","true");
	    PMPI_Info_set(info,"cb_align","2");
	    PMPI_Info_set(info,"romio_cb_write","true");
	    PMPI_Info_set(info,"cb_config_list","*:1");
	    PMPI_Info_set(info,"striping_factor","80");
	    
	    PMPI_Info_set(info,"IBM_largeblock_io","true");     
#endif
	    
/* with allowing the user to choose the filename - can overwrite an old */
/* file - which would be fine if MPI-IO allowed TRUNC - but it doesn't */
/* so we just delete so that we don't end up with trailing garbage  */
	    if (!task.mpi_rank) rv=PMPI_File_delete ( job.log_fname,MPI_INFO_NULL);
	    rv=PMPI_Barrier(MPI_COMM_WORLD);
	    
	    rv = PMPI_File_open( MPI_COMM_WORLD, job.log_fname, MPI_MODE_WRONLY | MPI_MODE_CREATE,info,  &ipm_mpiio_log_fh );
	    if (rv)
	    {
		printf("IPM: %d syslog fopen failed fname=%s \n",
		       task.mpi_rank, 
		       job.log_fname); fflush(stdout);
		return;
	    }
	    /* workaround for cases when MPI_INTEGER8 is not defined */
#ifndef MPI_INTEGER8
#define MPI_INTEGER8 MPI_LONG_LONG_INT
#endif
	    
	    if (task.mpi_size > 1) {
		if (task.mpi_rank == 0) {
		    buff_sum+=(int64_t)buff_size;
		    PMPI_Send (&buff_sum,1,MPI_INTEGER8,1,0,MPI_COMM_WORLD);
		    file_offset=0;
		} else if (task.mpi_rank == (task.mpi_size-1)) {
		    PMPI_Recv (&buff_sum,1,MPI_INTEGER8,task.mpi_rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		    file_offset=(MPI_Offset)buff_sum;
		} else{
		    PMPI_Recv (&buff_sum,1,MPI_INTEGER8,task.mpi_rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		    file_offset=(MPI_Offset)buff_sum;
		    buff_sum+=(int64_t)buff_size;
		    PMPI_Send (&buff_sum,1,MPI_INTEGER8,task.mpi_rank+1,0,MPI_COMM_WORLD);
		}
	    }
	    
	    rv=PMPI_File_set_view(ipm_mpiio_log_fh,file_offset,MPI_CHAR, MPI_CHAR,"native",info);
	    
/*write info*/
	    rv=PMPI_File_write_all(ipm_mpiio_log_fh,buffer,buff_size,MPI_CHAR,MPI_STATUS_IGNORE);
	    
	    
	    rv = PMPI_File_close( &ipm_mpiio_log_fh );
	    PMPI_Barrier(MPI_COMM_WORLD);
/* Some MPI-IO implimentations (cray) permissions are not setable with hints */
	    if (task.mpi_rank == 0) chmod (job.log_fname,0744);
	    free (buffer); 
	    return;
	} else {
	    if (! task.mpi_rank) printf("IPM: %d Allocation of IO Buffer failed on one or more tasks\n",task.mpi_rank);
	}
    }
/*parallel IO failed */
    if (! task.mpi_rank) printf("IPM: %d Using serial IO\n",task.mpi_rank);
    
      /*************************************/
      /* write log from rank zero using MPI*/
      /*************************************/
    if(task.mpi_rank==0) {
	ipm_mpi_log_fh = fopen(job.log_fname,"w+");
	if(ipm_mpi_log_fh == NULL) {
	    printf("IPM: %d syslog fopen failed fname=%s \n",
		   task.mpi_rank, 
		   job.log_fname); fflush(stdout);
	}
	chmod(job.log_fname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	
	ipm_log_write(&job, &task, txt,ipm_mpi_log_fh ,0,0);
	/* we now pollute the local profile state irrevocably in the interest
	   of keeping a memory footprint  which is a constant independent of
	   concurrency */
	
	/* task 0 initiates a volley of Sends via a handshake */
	for(i=1;i<job.ntasks;i++) {
	    PMPI_Send(&token,1,MPI_INT,i,0,MPI_COMM_WORLD); 
	    PMPI_Recv(&job,sizeof(struct ipm_jobdata),MPI_BYTE,i,0,MPI_COMM_WORLD,s+0);
	    PMPI_Recv(&task,sizeof(struct ipm_taskdata),MPI_BYTE,i,1,MPI_COMM_WORLD,s+1);
	    PMPI_Recv(&(txt[0]),MAXSIZE_TXTLINE,MPI_CHAR,i,1,MPI_COMM_WORLD,s+1);
	    
	    ipm_log_write(&job, &task, txt,ipm_mpi_log_fh ,0,0);
	}
	fclose(ipm_mpi_log_fh);
    } else {
	PMPI_Recv(&token,1,MPI_INT,0,0,MPI_COMM_WORLD,s+0);
	PMPI_Send(&job,sizeof(struct ipm_jobdata),MPI_BYTE,0,0,MPI_COMM_WORLD);
	PMPI_Send(&task,sizeof(struct ipm_taskdata),MPI_BYTE,0,1,MPI_COMM_WORLD);
	PMPI_Send(&(txt[0]),MAXSIZE_TXTLINE,MPI_CHAR,0,1,MPI_COMM_WORLD);
    }
    PMPI_Barrier(MPI_COMM_WORLD);
    
    return;
#endif
    
    return;
}


static int lock_test(void) { /* anonymnous collective */
 FILE *fh;
 int fd,rv,log_rank=-1;
 double t0,t1,t2;
 char fn[MAXSIZE_FILENAME];

 sprintf(fn,"lockfile.%s",job.cookie);

 if(!task.mpi_rank) {
  unlink(fn);
 }

 PMPI_Barrier(MPI_COMM_WORLD);
 IPM_TIME_GTOD(t0);
 IPM_FILE_LOCK(fn,fd);
 fh = fdopen(fd,"w+");
 IPM_TIME_GTOD(t1);
 rv = fscanf(fh,"%d", &log_rank);
 if(rv == 1) {
  log_rank ++;
 } else {
  log_rank = 0;
 }
 fseek(fh,0,SEEK_SET);
 fprintf(fh, "%d                 ", log_rank);
 IPM_FILE_UNLOCK(fn,fd);
 IPM_TIME_GTOD(t2);
 printf("IPM: %d lock_test log_rank=%d file=%s wait=%.6f got=%.6f done=%.6f\n",        task.mpi_rank,
        log_rank,
        fn,
        t0,
        t1,
        t2);
 return 0;
}



