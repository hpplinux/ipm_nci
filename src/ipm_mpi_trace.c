
#ifndef IPM_LOG_USEMULTI
#ifndef IPM_LOG_USELOCKS
#ifndef IPM_LOG_USETMPFS
#ifndef IPM_LOG_USEMPI
#define IPM_LOG_USEMPI
#endif
#endif
#endif
#endif

static void ipm_mpi_trace(IPM_KEY_TYPE key, double T) {

    if(task.ipm_trc_count >= task.ipm_trc_count_max) {
	task.flags &= ~IPM_TRC_ACTIVE;
	/* FIXME this would be a good place to roll the trace buffer over if 
	   it's being used as a ring buffer, or dump it out on a one time basis
	   if it's being used as a snapshot buffer */
	return; 
    }

/*
  if(task.flags & DEBUG)  {
  printf("IPM: %d TRACE_ENT %s %d %d %d\n",
  task.mpi_rank,
  task.call_label[KEY_CALL(key)],
  KEY_RANK(key),
  KEY_BYTE(key),
  KEY_REGION(key));
  }
*/

 
    task.ipm_trc_buf[task.ipm_trc_count].key = key;
#ifdef IPM_ENABLE_PROFLOW
    task.ipm_trc_buf[task.ipm_trc_count].callsite = 0;
#endif
    ipm_hpm_read(task.ipm_trc_buf[task.ipm_trc_count].hpm); 
    task.ipm_trc_buf[task.ipm_trc_count].t = T;

    task.ipm_trc_count++;
    return;
}


static void ipm_trc_mask_time_tron() {
    task.flags |= IPM_TRC_ACTIVE;
    signal(SIGALRM, ipm_trc_mask_time_troff);
    alarm(task.ipm_trc_mask_time_final);
}

static void ipm_trc_mask_time_troff() {
    task.flags &= ~IPM_TRC_ACTIVE;
}

static void ipm_trc_dump_internal();
static void ipm_trc_dump() {
    int i,j,jj,fd,rv;
    char tmp_fname[MAXSIZE_FILENAME];
    char tmp_cmd[MAXSIZE_FILENAME];
    FILE *fh, *ipm_mpi_trc_fh;
    MPI_Status s;

/*
  for(i=0;i<task.ipm_trc_count;i++) {
  #ifdef IPM_ENABLE_PROFLOW
  printf("IPM: %d TRACE %.6e %p %s %d %d %d ",
  #else
  printf("IPM: %d TRACE %.6e - %s %d %d %d ",
  #endif
  task.mpi_rank,
  task.ipm_trc_buf[i].t-task.ipm_trc_time_init,
  #ifdef IPM_ENABLE_PROFLOW
  task.ipm_trc_buf[i].callsite,
  #endif
  task.call_label[KEY_CALL(task.ipm_trc_buf[i].key)],
  KEY_BYTE(task.ipm_trc_buf[i].key),
  KEY_REGION(task.ipm_trc_buf[i].key),
  KEY_RANK(task.ipm_trc_buf[i].key));
  for(j=0;j<MAXSIZE_HPMCOUNTERS;j++) {
  printf("%lld ", task.ipm_trc_buf[i].hpm[j]);
  }
  printf("\n");
  }
*/


#ifdef IPM_LOG_USEMPI
/*
** Aggregate using MPI {
*/



/*
** }
*/
#endif

/*
** Aggregate using node local filesystem {
*/

#ifdef IPM_LOG_USETMPFS

    if(task.mpi_size == 1) { /* special easy case  now uneeded */
    }

    sprintf(tmp_fname,"/tmp/%s.trace.%s.%d",
	    job.username,
	    job.cookie,
	    task.mpi_rank);
    ipm_mpi_trc_fh = fopen(tmp_fname,"w");
    if(ipm_mpi_trc_fh == NULL) {
	printf("IPM: %d trc_dump fopen failed fname=%s \n",
	       task.mpi_rank,
	       tmp_fname); fflush(stdout);
    }
    chmod(tmp_fname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    if(!(task.flags & IPM_HPM_CANCELED)) {
	rv = fprintf(ipm_mpi_trc_fh, "<trace ipm_version=\"%s\" cookie=\"%s\" mpi_rank=\"%d\" mpi_size=\"%d\" stamp_init=\"%.6f\" stamp_final=\"%.6f\"  username=\"%s\" flags=\"%lld\" nhpmcounters=\"%d\" >\n",        IPM_VERSION,
		     job.cookie,
		     task.mpi_rank,
		     task.mpi_size,
		     task.stamp_init,
		     task.stamp_final,
		     job.username,
		     task.flags,
		     MAXSIZE_HPMCOUNTERS);
    } else {
	rv = fprintf(ipm_mpi_trc_fh, "<trace ipm_version=\"%s\" cookie=\"%s\" mpi_rank=\"%d\" mpi_size=\"%d\" stamp_init=\"%.6f\" stamp_final=\"%.6f\"  username=\"%s\" flags=\"%lld\" nhpmcounters=\"NA\" >\n",        IPM_VERSION,
		     job.cookie,
		     task.mpi_rank,
		     task.mpi_size,
		     task.stamp_init,
		     task.stamp_final,
		     job.username,
		     task.flags);
    }

    for(i=0;i<task.ipm_trc_count;i++) {
	rv = fprintf(ipm_mpi_trc_fh,"%.8f %d %d %d %d ",
		     task.ipm_trc_buf[i].t-task.ipm_trc_time_init,
		     KEY_CALL(task.ipm_trc_buf[i].key),
		     KEY_BYTE(task.ipm_trc_buf[i].key),
		     KEY_REGION(task.ipm_trc_buf[i].key),
		     KEY_RANK(task.ipm_trc_buf[i].key));
	if(!(task.flags & IPM_HPM_CANCELED)) {
	    for(j=0;j<MAXSIZE_HPMCOUNTERS;j++) {
		rv = fprintf(ipm_mpi_trc_fh,"%lld ", task.ipm_trc_buf[i].hpm[j]);
	    }
	}
	rv = fprintf(ipm_mpi_trc_fh,"\n");
    }

    rv = fprintf(ipm_mpi_trc_fh,"</trace>\n");
    fflush(ipm_mpi_trc_fh);

    fclose(ipm_mpi_trc_fh);


    /* host local ring barrier so that /tmp is all good */

    if(task.intra_size > 1) {
	if(task.intra_root == task.mpi_rank) {
	    PMPI_Send(&i,1,MPI_INT,task.intra_right,0,MPI_COMM_WORLD);
	    PMPI_Recv(&i,1,MPI_INT,task.intra_left,0,MPI_COMM_WORLD,&s);
	} else {
	    PMPI_Recv(&i,1,MPI_INT,task.intra_left,0,MPI_COMM_WORLD,&s);
	    PMPI_Send(&i,1,MPI_INT,task.intra_right,0,MPI_COMM_WORLD);
	}
    }

    if(task.intra_root == task.mpi_rank) {
	if(job.nhosts > 1 && task.mpi_rank) {
	    PMPI_Recv(&i,1,MPI_INT,task.inter_left,0,MPI_COMM_WORLD,&s);
	}
	sprintf(tmp_cmd, "/usr/bin/cat /tmp/%s.trace.%s.* >> %s.trace",
		job.username,
		job.cookie,
		job.LOG_fname);
	system(tmp_cmd);
	chmod(job.LOG_fname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	sprintf(tmp_cmd, "/usr/bin/rm -f /tmp/%s.trace.%s.* ",
		job.username,
		job.cookie);
	system(tmp_cmd);
	if(job.nhosts > 1 && task.inter_right != 0) {
	    PMPI_Send(&i,1,MPI_INT,task.inter_right,0,MPI_COMM_WORLD);
	}


    }

/*
** }
*/
#endif 

#ifdef IPM_LOG_USELOCKS 
/*
** Aggregate using locks -- least prefered {
*/

    IPM_FILE_LOCK(job.LOG_fname,fd);
    fh =  fdopen(fd,"w+");
    fseek(fh,0,SEEK_END);

#define IPM_TRC_DUMP_HEADER (fh,t,j,mx_hpm) {				\
	rv = fprintf(fh, "<trace ipm_version=\"%s\" cookie=\"%s\" mpi_rank=\"%d\" mpi_size=\"%d\" stamp_init=\"%.6f\" stamp_final=\"%.6f\"  username=\"%s\" flags=\"%lld\" nhpmcounters=\"%d\" >\n", \
		     IPM_VERSION, j.cookie, t.mpi_rank, t.mpi_size, t.stamp_init, \
		     t.stamp_final, j.username, t.flags, mx_hpm);	\
    }

    rv = fprintf(fh, "<trace ipm_version=\"%s\" cookie=\"%s\" mpi_rank=\"%d\" mpi_size=\"%d\" stamp_init=\"%.6f\" stamp_final=\"%.6f\"  username=\"%s\" flags=\"%lld\" nhpmcounters=\"%d\" >\n",
		 IPM_VERSION,
		 job.cookie,
		 task.mpi_rank,
		 task.mpi_size,
		 task.stamp_init,
		 task.stamp_final,
		 job.username,
		 task.flags,
		 MAXSIZE_HPMCOUNTERS);

    for(i=0;i<task.ipm_trc_count;i++) {
	rv = fprintf(fh,"%.6e %d %d %d %d ",
		     task.ipm_trc_buf[i].t-task.ipm_trc_time_init,
		     KEY_CALL(task.ipm_trc_buf[i].key),
		     KEY_BYTE(task.ipm_trc_buf[i].key),
		     KEY_REGION(task.ipm_trc_buf[i].key),
		     KEY_RANK(task.ipm_trc_buf[i].key));
	for(j=0;j<MAXSIZE_HPMCOUNTERS;j++) {
	    jj = ipm_hpm_eorder[task.hpm_eventset][j];
	    rv = fprintf(fh,"%lld ", task.ipm_trc_buf[i].hpm[jj]);
	}
	rv = fprintf(fh,"\n");
    }

    rv = fprintf(fh,"</trace>\n");
    fflush(fh);

    IPM_FILE_UNLOCK(job.LOG_fname,fd);

/*
** }
*/
#endif

    return;

}

