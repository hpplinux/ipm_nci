/* FIXME */
/* the ugly - in order to minimize memory footprint we fully reduce as much
   as possible. If we knew a priori that we could do a gather from all tasks
   then this could be treated as an array of taskdata structs. The memory
   required to do so scales with concurrency. the bad - I use anonymous
   vectors for which one must know the relation between the index and the
   value stored there. the good - there are only 10 (MAXSIZE_AGGBUF) of them 
   to keep track of, often in triples which have been reduced as
   0=total, 1=min, 2=max  -- sincere apologies.  
 
   I intend to clean this up via a struct for report data - Sep 2005
   this should solve the bad but not the ugly.
*/

/* note ipm_report_internal is called by rank zero after setting said
   ugly arrays right in e.g. ipm_report. The idea is to keep report 
   format information seperated in this function. In this way other 
   interfaces can generate similar reports, e.g. the ipm binary itself
   can call ipm_report_internal. */
   

int ipm_report_internal(FILE *fh, int nregion, int jreg, char *region_label,
			unsigned long long int flags,
			struct ipm_taskdata *task,
			struct ipm_jobdata *job,
			double g_rbuf[3][MAXSIZE_AGGBUF],
			long long int g_cbuf[3][MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS],
			int g_cpop[MAXSIZE_NEVENTSETS],
			double g_mpi_time[MAXSIZE_CALLS+1],
			double g_mpi_count[MAXSIZE_CALLS+1]) {
 
    int i,ii,j,reg_ntasks;
    double mtime_tot=0.0;
    char start_date_buf[2*MAXSIZE_LABEL] = "unknown";
    char stop_date_buf[2*MAXSIZE_LABEL] = "unknown";
    char state_buf[2*MAXSIZE_LABEL] = "unknown";
    int idx[MAXSIZE_CALLS+1];
    int icall,jcall;
    ipm_hash_ent *hbufp;
    double global_flop_factor,global_wall;
    char hbuf[MAXSIZE_TXTLINE+1];

    reg_ntasks = g_rbuf[0][8];
    if(g_rbuf[0][5] == 0.0) {
	/* no rank collected flops counters, odd case, so it goes */
	global_flop_factor = 0.0;
    } else {
	global_flop_factor = (reg_ntasks/g_rbuf[0][5]);
    }
                                                                                
    /* possibly a matter of definition for global_wall    */
    /* if MPI_Finalize collective --> use maximum wall time  */
    global_wall = g_rbuf[2][0];
 
  
    fflush(stdout);

    strftime(start_date_buf, MAXSIZE_LABEL, "%m/%d/%y/%T", localtime(&(job->start_ts)));
    strftime(stop_date_buf, MAXSIZE_LABEL, "%m/%d/%y/%T", localtime(&(job->final_ts)));
  

    if(flags & IPM_APP_RUNNING) {
	sprintf(state_buf, APP_STATE_RUNNING);
    } else if(flags & IPM_APP_INTERRUPTED) {
	sprintf(state_buf, APP_STATE_INTERRUPTED);
    } else if(flags & IPM_APP_COMPLETED) {
	sprintf(state_buf, APP_STATE_COMPLETED);
    } else {
	sprintf(state_buf, "unknown");
    }
 
    if(jreg==-1) {
	sprintf(hbuf,"###############################################################################\n");
	strncpy(hbuf+2,"IPMv",4);
	strncpy(hbuf+6,job->ipm_version,strlen(job->ipm_version));
	fprintf(fh, hbuf);
/* old
   fprintf(fh,"##IPMv%s###################################################################\n",job->ipm_version); 
*/
	fprintf(fh,"# \n"); 
 
/*
  rv = strlen(job->cmdline);
  if(rv > 55) {
  rv = strlen(job->cmdline) - 55);
  } else {
  rv = 0
  }
*/
	fprintf(fh,"# command : %s (%s)\n", task->mpi_cmdline, state_buf ); 
	sprintf(hbuf,"%s/%s",
		task->hostname,
		task->mach_info);
	fprintf(fh,"# host    : %-30s mpi_tasks : %d on %d nodes\n",
		hbuf,
		job->ntasks,
		job->nhosts);
	fprintf(fh,"# start   : %17s              wallclock : %f sec\n",
		start_date_buf, global_wall); 
	fprintf(fh,"# stop    : %17s              %%comm     : %-.2f \n",
		stop_date_buf, 100*g_rbuf[0][3]/(global_wall*job->ntasks)); 
	if(flags & IPM_HPM_CANCELED) {
	    fprintf(fh,"# gbytes  : %.5e total              gflop/sec : NA \n",
		    OOGB*g_rbuf[0][7]);
	} else {
	    fprintf(fh,"# gbytes  : %.5e total              gflop/sec : %.5e total\n",
		    OOGB*g_rbuf[0][7],
		    OOGU*g_rbuf[0][6]*global_flop_factor/global_wall);
	}
                                                                                
/*
  fprintf(fh,"# http://www.nersc.gov/nusers/ipm/jv.php?%s.%s\n",
  job->username,
  job->cookie); 
*/
	fprintf(fh,"#\n");
    } 

    if(flags & REPORT_TERSE) {
	if(nregion > 1) {
	    if(jreg == -1) {
		fprintf(fh,"# region  :                [ntasks]     <wall>        <mpi>       [gflop/sec]\n"); 
		return(1);
	    }
	    if(!(flags & IPM_HPM_CANCELED)) {
		fprintf(fh,"# %-21.21s %6d  %13.4f %13.4f      %.4e \n",
			region_label,
			reg_ntasks,
			g_rbuf[0][0]/reg_ntasks,
			g_rbuf[0][3]/reg_ntasks,
			OOGU*g_rbuf[0][6]*global_flop_factor/(g_rbuf[0][0]/reg_ntasks));
/* added reg_ntasks above in last line in IPM0.87 */
	    } else {
		fprintf(fh,"# %-21.21s %6d  %13.4f %13.4f          NA \n",
			region_label,
			reg_ntasks,
			g_rbuf[0][0]/reg_ntasks,
			g_rbuf[0][3]/reg_ntasks);
	    }
	}
	return 1;
    }
 
    if(flags & REPORT_FULL)  {
	fprintf(fh,"%79s","##############################################################################\n"); 
	fprintf(fh,"# region  : %s       [ntasks] = %6d\n",
		region_label,
		reg_ntasks);
    }
#define FLT_REP_FMT "# %-20.20s %13g %13g %13g %13g\n"
#define PCT_REP_FMT "# %-20.20s               %13g %13g %13g\n"
#define MPI_REP_FMT "# %-20.20s %13g %13g        %6.2f       %6.2f\n"
 
    fprintf(fh,"#\n");
    fprintf(fh,"#                           [total]         <avg>           min           max \n");
 
    fprintf(fh,FLT_REP_FMT,"entries",
	    g_rbuf[0][9],
	    g_rbuf[0][9]/reg_ntasks,
	    g_rbuf[1][9],
	    g_rbuf[2][9]); 
    fprintf(fh,FLT_REP_FMT,"wallclock",
	    g_rbuf[0][0],
	    g_rbuf[0][0]/job->ntasks,
	    g_rbuf[1][0],
	    g_rbuf[2][0]); 
    fprintf(fh,FLT_REP_FMT,"user",
	    g_rbuf[0][1],
	    g_rbuf[0][1]/job->ntasks,
	    g_rbuf[1][1],
	    g_rbuf[2][1]); 
    fprintf(fh,FLT_REP_FMT,"system",
	    g_rbuf[0][2],
	    g_rbuf[0][2]/job->ntasks,
	    g_rbuf[1][2],
	    g_rbuf[2][2]); 
    fprintf(fh,FLT_REP_FMT,"mpi",
	    g_rbuf[0][3],
	    g_rbuf[0][3]/job->ntasks,
	    g_rbuf[1][3],
	    g_rbuf[2][3]); 
    fprintf(fh,PCT_REP_FMT,"%comm",
	    100*g_rbuf[0][3]/(job->ntasks*global_wall),
	    100*g_rbuf[1][4],
	    100*g_rbuf[2][4]); 
    if(!(flags & IPM_HPM_CANCELED)) {
	fprintf(fh,FLT_REP_FMT,"gflop/sec",
		OOGU*g_rbuf[0][6]*global_flop_factor/global_wall,
		(OOGU*g_rbuf[0][6]*global_flop_factor/job->ntasks)/global_wall,
		OOGU*g_rbuf[1][6]/global_wall,
		OOGU*g_rbuf[2][6]/global_wall); 
    }
    if(jreg==-1) {
	fprintf(fh,FLT_REP_FMT,"gbytes",
		OOGB*g_rbuf[0][7],
		OOGB*g_rbuf[0][7]/job->ntasks,
		OOGB*g_rbuf[1][7],
		OOGB*g_rbuf[2][7]); 
    }

    if(!(flags & IPM_HPM_CANCELED)) {
	fprintf(fh,"#\n");
	for(j=0;j<MAXSIZE_NEVENTSETS;j++) {
	    if(g_cpop[j] > 0) {
		for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
		    ii = ipm_hpm_eorder[j][i];
		    if(g_cbuf[0][j][ii] > 0) {
			if (strlen(ipm_hpm_ename[j][ii])<21){
			    fprintf(fh, "# %-20.20s %13g %13g %13g %13g\n",
				    ipm_hpm_ename[j][ii],
				    1.0*g_cbuf[0][j][ii],
				    1.0*(long long int)(g_cbuf[0][j][ii]/(1.0*g_cpop[j])),
				    1.0*g_cbuf[1][j][ii],
				    1.0*g_cbuf[2][j][ii]);
			}else{
			    fprintf(fh, "# %s\n                       %13g %13g %13g %13g\n",
				    ipm_hpm_ename[j][ii],
				    1.0*g_cbuf[0][j][ii],
				    1.0*(long long int)(g_cbuf[0][j][ii]/(1.0*g_cpop[j])),
				    1.0*g_cbuf[1][j][ii],
				    1.0*g_cbuf[2][j][ii]);
			}
		    }
		}
	    }
	}
    }
    fprintf(fh,"#\n");
 
    if(g_rbuf[0][3] != 0.0) {
	index_doubles(MAXSIZE_CALLS+1,idx,g_mpi_time);
	mtime_tot = 0.0;
 
	fprintf(fh,"#                            [time]       [calls]        <%%mpi>      <%%wall>\n");
	for(icall=0;icall<=MAXSIZE_CALLS;icall++) {
	    jcall = idx[icall];
	    if(g_mpi_time[jcall] == 0.0) break;
	    fprintf(fh,MPI_REP_FMT,
		    task->call_label[jcall],
		    g_mpi_time[jcall],
		    g_mpi_count[jcall],
		    100*g_mpi_time[jcall]/g_rbuf[0][3],
		    100*g_mpi_time[jcall]/g_rbuf[0][0]
		);
	    mtime_tot += g_mpi_time[jcall];
	}
	if(mtime_tot>0.0 && flags & DEBUG) {
	    fprintf(fh,MPI_REP_FMT,
		    "MPI_TOTALTIME",
		    mtime_tot,
		    0.0,
		    100*mtime_tot/g_rbuf[0][3],
		    100*mtime_tot/g_rbuf[0][0]
		);
	}
 
    }
 
 
    return 0;
}
#undef FLUSH

