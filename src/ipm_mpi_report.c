/*
 * ####
 * #  IPM report
 * ####
 */ 

#define FLUSH 

void ipm_report(char *tag) {
    FILE *log_fh;
    int i,j,k,ireg,jreg,itask,icall;
    MPI_Status s;
    int initialized=0, aggregated = 0;
    time_t ts;
    char cmd[MAXSIZE_TXTLINE];
    int preg[MAXSIZE_REGION];
    int cpop[MAXSIZE_NEVENTSETS];
    int g_cpop[MAXSIZE_NEVENTSETS];
    char egregious_mistake[MAXSIZE_LABEL];
    ipm_hash_ent *hbufp;
    double lflops;
    
    
/****************/
/* local totals */
/****************/
    
    double rbuf[MAXSIZE_AGGBUF], g_rbuf[3][MAXSIZE_AGGBUF];
    long long int cbuf[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS];
    long long int g_cbuf[3][MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS];
    double mpi_time[MAXSIZE_CALLS+1], g_mpi_time[MAXSIZE_CALLS+1];
    double mpi_count[MAXSIZE_CALLS+1], g_mpi_count[MAXSIZE_CALLS+1];
    
    
    aggregated = 0;
    
    if(task.flags & REPORT_NONE) {
	return;
    }
    
#ifdef DEVEL
    if(task.flags & DEBUG) {
	IPM_HASH_PRINT("IPM: HASH");
    }
#endif
    
    if(task.flags & IPM_APP_INTERRUPTED) { /* MPI is  not available */
	/******************************************/
	/* try to aggregate via log               */
	/* how to trust that the log is complete? */
	/******************************************/
	
	time(&ts);
	if(!task.mpi_rank) {
	    strftime(cmd, MAXSIZE_TXTLINE, "%c", localtime(&ts));
	    printf("IPM: 0 Your job was canceled or interrupted at %s\n", cmd);
	    sprintf(cmd, "ipm  -parse %s", job.log_fname);
	    log_fh = popen(cmd, "r"); 
	    while(fgets(cmd,MAXSIZE_TXTLINE,log_fh)) {
		printf("%s", cmd);
	    }
	    fclose(log_fh);
	    aggregated = 1; 
	    return;
	}
	
	/**************************************/
	/* no log or MPI so can not report    */
	/**************************************/
	/* this block is now uneeded */
	if(!task.mpi_rank) {
	    printf("IPM : could not generate a profile in IPM_MPI_Finalize\n");
	    printf("IPM : MPI has been interrupted by a signal and the system log\n");
	    printf("IPM : not found or incomplete. \n");
	    printf("IPM : A node or nodes may have crashed during run.\n");
	    printf("IPM : log = %s  \n",job.log_fname);
	}
	return;
    }
    
 /* ipm_log_project(&job, &task, &rep); */
    
/*

  FIXME 

  need a non-gather based reduction operator that gives rank 0 a list of
  all unique region labels invoked by any task. 

  MPI_Reduce(task.region_label, global_labels, my_op); 

  what's below avoids globally imposed ordering of regions (index mismatch),
  but each task must enter each region though not necessarily in the same order.
  Often true but an egregious mistake. 

  I have coded a custom MPI_Reduce operator that does a hash merge along 
  the lines of the above. The complexity was unapealing on several grounds
  and I went back to the current method. It also crashed in the MPI_Datatyping
  on NEC.

 */

/******IPM_ALL**********/

    rbuf[0] = task.wtime;
    rbuf[1] = task.utime;
    rbuf[2] = task.stime;
    rbuf[3] = task.mtime;
    rbuf[4] = task.mtime/task.wtime;
    if(task.flops == -1.0) {
	rbuf[5] = 0.0;
	rbuf[6] = 0.0;
    } else {
	rbuf[5] = 1.0;
	rbuf[6] = task.flops;
    }
    rbuf[7] = task.bytes;
    rbuf[8] = 1.0; /* something later */
    rbuf[9] = 1.0; /* something later */
    
    for(i=0;i<=MAXSIZE_CALLS;i++) {
	mpi_time[i] = 0.0;
	mpi_count[i] = 0.0;
	g_mpi_time[i] = 0.0;
	g_mpi_count[i] = 0.0;
    }
    
    for(i=0;i<=MAXSIZE_HASH;i++) {
	hbufp = &(task.hash[i]);
	if(hbufp->key != 0) {
	    icall = KEY_CALL(hbufp->key);
	    mpi_time[icall] += hbufp->t_tot;
	    mpi_count[icall] += hbufp->count;
	}
    }
    
    for(i=0;i<MAXSIZE_NEVENTSETS;i++) {
	cpop[i] = 0;
    }
    cpop[task.hpm_eventset] = 1;
    
    for(i=0;i<MAXSIZE_NEVENTSETS;i++) {
	for(j=0;j<MAXSIZE_HPMCOUNTERS;j++) {
	    g_cbuf[0][i][j] = 0;
	    g_cbuf[1][i][j] = 0;
	    g_cbuf[2][i][j] = 0;
	    cbuf[i][j] = 0;
	    for(k=0;k<MAXSIZE_REGION;k++) {
		cbuf[i][j] += task.hpm_count[k][i][j];
#ifdef DEVEL2
		printf("IPM: pre_rep1 reg=%d cnt=%d  %lld \n",
		       k, j,
		       task.hpm_count[k][i][j]);
#endif
	    }
	}
    }
    
  /* now reduce the above totals */

    PMPI_Reduce(rbuf,g_rbuf[0],MAXSIZE_AGGBUF,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    PMPI_Reduce(rbuf,g_rbuf[1],MAXSIZE_AGGBUF,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);
    PMPI_Reduce(rbuf,g_rbuf[2],MAXSIZE_AGGBUF,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    PMPI_Reduce(mpi_time,g_mpi_time,MAXSIZE_CALLS+1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    PMPI_Reduce(mpi_count,g_mpi_count,MAXSIZE_CALLS+1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    PMPI_Reduce(cpop,g_cpop,MAXSIZE_NEVENTSETS,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    PMPI_Reduce(cbuf,g_cbuf[0],MAXSIZE_NEVENTSETS*MAXSIZE_HPMCOUNTERS,MPI_LONG_LONG_INT,MPI_SUM,0,MPI_COMM_WORLD);
    PMPI_Reduce(cbuf,g_cbuf[1],MAXSIZE_NEVENTSETS*MAXSIZE_HPMCOUNTERS,MPI_LONG_LONG_INT,MPI_MIN,0,MPI_COMM_WORLD);
    PMPI_Reduce(cbuf,g_cbuf[2],MAXSIZE_NEVENTSETS*MAXSIZE_HPMCOUNTERS,MPI_LONG_LONG_INT,MPI_MAX,0,MPI_COMM_WORLD);
    
 job.start_ts = task.start_ts;
 job.final_ts = task.final_ts;
 
 
#define FH stdout
  if(!task.mpi_rank) {
      ipm_report_internal(FH, task.region_nregion, -1, "*",
			  task.flags, &task, &job, g_rbuf, g_cbuf, g_cpop, g_mpi_time, g_mpi_count); 
  }
  
/******IPM_ALL**********/

  if(task.region_nregion > 1) {
      index_doubles(MAXSIZE_REGION,preg,task.region_wtime);
      
      for(ireg=0;ireg<task.region_nregion;ireg++) {
	  
	  jreg = preg[ireg];
	  sprintf(egregious_mistake,"%s",task.region_label[jreg]);
	  PMPI_Bcast(egregious_mistake,MAXSIZE_LABEL,MPI_CHAR,0,MPI_COMM_WORLD);
	  if(task.mpi_rank) {
	      jreg = -1;
	      for(i=0;i<task.region_nregion;i++) {
		  if(!strncmp(egregious_mistake, task.region_label[i],MAXSIZE_LABEL)) {
     jreg = i; 
     break;
		  }
	      }
	  }
	  
	  if(jreg < 0) { 			/* this task did not enter the region */
	      for(i=0;i<MAXSIZE_AGGBUF;i++) {
		  rbuf[i] = 0.0;
	      }
	  } else { 				/* this is some other region */
	      rbuf[0] = task.region_wtime[jreg];
	      rbuf[1] = task.region_utime[jreg];
	      rbuf[2] = task.region_stime[jreg];
	      rbuf[3] = task.region_mtime[jreg];
	      rbuf[4] = task.region_mtime[jreg]/task.region_wtime[jreg];
	      
	      lflops = IPM_HPM_CALC_FLOPS(task.hpm_count[jreg][task.hpm_eventset]);
	      if(lflops == -1.0) {
		  rbuf[5] = 0.0;
		  rbuf[6] = 0.0;
	      } else {
		  rbuf[5] = 1.0;
		  rbuf[6] = lflops;
	      }
	      rbuf[7] = task.region_count[jreg];
	      rbuf[8] = 1.0; /* population of ranks in this region */
	      rbuf[9] = task.region_count[jreg];
	      
	      for(i=0;i<=MAXSIZE_CALLS;i++) {
		  mpi_time[i] = 0.0;
		  mpi_count[i] = 0.0;
		  g_mpi_time[i] = 0.0;
		  g_mpi_count[i] = 0.0;
	      }
	      
	      for(i=0;i<=MAXSIZE_HASH;i++) {
		  hbufp = &(task.hash[i]);
		  if(hbufp->key != 0) {
		      if(KEY_REGION(hbufp->key) == jreg) {
			  icall = KEY_CALL(hbufp->key);
			  mpi_time[icall] += hbufp->t_tot;
			  mpi_count[icall] += hbufp->count;
		      }
		  }
	      }

	      for(i=0;i<MAXSIZE_NEVENTSETS;i++) {
		  for(j=0;j<MAXSIZE_HPMCOUNTERS;j++) {
		      g_cbuf[0][i][j] = 0;
		      g_cbuf[1][i][j] = 0;
		      g_cbuf[2][i][j] = 0;
		      cbuf[i][j] = task.hpm_count[jreg][i][j];
#ifdef DEVEL
		      printf("IPM: pre_rep2 reg=%d cnt=%d  %lld \n",
			     jreg, j,
			     task.hpm_count[jreg][i][j]);
#endif
		  }
	      }
	      
	  }
	  
	  for(i=0;i<MAXSIZE_NEVENTSETS;i++) {
	      cpop[i] = 0;
	  }
	  cpop[task.hpm_eventset] = 1;
	  
	  /* now reduce the above totals */
	  
	  PMPI_Reduce(rbuf,g_rbuf[0],MAXSIZE_AGGBUF,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	  PMPI_Reduce(rbuf,g_rbuf[1],MAXSIZE_AGGBUF,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);
	  PMPI_Reduce(rbuf,g_rbuf[2],MAXSIZE_AGGBUF,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
	  PMPI_Reduce(mpi_time,g_mpi_time,MAXSIZE_CALLS+1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	  PMPI_Reduce(mpi_count,g_mpi_count,MAXSIZE_CALLS+1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	  PMPI_Reduce(cpop,g_cpop,MAXSIZE_NEVENTSETS,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	  PMPI_Reduce(cbuf,g_cbuf[0],MAXSIZE_NEVENTSETS*MAXSIZE_HPMCOUNTERS,MPI_LONG_LONG_INT,MPI_SUM,0,MPI_COMM_WORLD);
	  PMPI_Reduce(cbuf,g_cbuf[1],MAXSIZE_NEVENTSETS*MAXSIZE_HPMCOUNTERS,MPI_LONG_LONG_INT,MPI_MIN,0,MPI_COMM_WORLD);
	  PMPI_Reduce(cbuf,g_cbuf[2],MAXSIZE_NEVENTSETS*MAXSIZE_HPMCOUNTERS,MPI_LONG_LONG_INT,MPI_MAX,0,MPI_COMM_WORLD);

/*
 if(task.mpi_rank == 0) {
 for(i=0;i<MAXSIZE_AGGBUF;i++) {
  printf("g_rbuf %d %.6e %.6e %.6e\n",i,g_rbuf[0][i], g_rbuf[1][i], g_rbuf[2][i]);
 }
 }
*/

	  if(!task.mpi_rank) {
	      ipm_report_internal(FH, task.region_nregion, jreg, task.region_label[jreg],
				  task.flags, &task, &job, g_rbuf, g_cbuf, g_cpop, g_mpi_time, g_mpi_count); 
	  }
	  
      } /* each region known to task 0*/
  }

 /* ipm_report does not take care of last border line */
  if(!task.mpi_rank) fprintf(FH,"%79s","###############################################################################\n"); 
#undef FH
  
}

#include "ipm_report_internal.c"

