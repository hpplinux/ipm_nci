/* Fortran entry points */

/* C entry points */
void ipm_region(int io, char *tag) {
    int i,region_tag=-1,region_next=-1;
    static IPM_TICK_TYPE 		T1,T2;
    static double t1,wt1,ut1,st1;
    static double                   region_wtime_final;
    static double                   region_utime_final;
    static double                   region_stime_final;



/*
 * capture state
 */

    IPM_TIME_GET(T1);
    if(!(task.flags & IPM_HPM_CANCELED)) {
	ipm_hpm_read(task.ipm_trc_hpm_region_final);
    }
    IPM_GETRUSAGE_SELF(task.ru_SELF_curr);
    region_wtime_final = IPM_TIME_SEC(T1);
    region_utime_final = IPM_UTIME_GET(task.ru_SELF_curr);
    region_stime_final = IPM_STIME_GET(task.ru_SELF_curr);
 
#ifdef DEVEL
    if(task.flags & DEBUG) {
	if(task.region_current >= 0) {
	    printf("IPM: %d ipm_region enter (%d, \"%s\") (was region %s %d/%d)\n",
		   task.mpi_rank,
		   io,
		   tag,
		   task.region_label[task.region_current],
		   task.region_current,
		   task.region_nregion);
	} else {
	    printf("IPM: %d ipm_region enter INIT (%d, \"%s\") (was region %s %d/%d)\n",
		   task.mpi_rank,
		   io,
		   tag,
		   "none",
		   task.region_current,
		   task.region_nregion);
	}
    }
#endif

/* 
 * determine which is the next region 
 */

    region_tag = -1;
    for(i=0;i<task.region_nregion;i++) {
	if(!strncmp(tag,task.region_label[i], MAXSIZE_LABEL)) {
	    region_tag = i; /* entering a new (prev known) region */
	}
    }

    if(region_tag < 0 && io == 1) { /* entering a new (prev unknown) region */
	if(task.region_nregion < MAXSIZE_REGION) {
	    region_tag = task.region_nregion;
	    strncpy(task.region_label[region_tag],tag,MAXSIZE_LABEL);
	    task.region_nregion++; 
	} else {
	    printf("IPM: %d ERROR too many IPM regions (MAXSIZE_REGION=%d)  \n",
		   task.mpi_rank,	
		   MAXSIZE_REGION);
	    exit(1);
	}
    }

    if(io != 1 && io != -1) {
	printf("IPM: %d ERROR incorrect second arg to ipm_region == %d  \n",
	       task.mpi_rank,	io);
	exit(1);
    }

/*
 * act on region_tag
 */

    if(task.region_current >= 0) {

/*
 * default case : accum prev against , set task.region_current
 */

	task.region_wtime[task.region_current]  +=
	    region_wtime_final - region_wtime_init;
	task.region_utime[task.region_current]  +=
	    region_utime_final - region_utime_init;
	task.region_stime[task.region_current]  +=
	    region_stime_final - region_stime_init;
	task.region_count[task.region_current] ++;

	if(task.flags & IPM_HPM_ACTIVE) {
	    for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
		task.hpm_count[task.region_current][task.hpm_eventset][i] +=
		    task.ipm_trc_hpm_region_final[i] - task.ipm_trc_hpm_region_init[i];
#ifdef DEVEL
		if(task.flags & DEBUG) {
		    printf("IPM: %d hpm_chk reg=%d->%d cnt=%d  %lld - %lld = %lld T = %lld \n",
			   task.mpi_rank, task.region_current, region_tag , i,
			   task.ipm_trc_hpm_region_final[i],
			   task.ipm_trc_hpm_region_init[i],
			   task.ipm_trc_hpm_region_final[i]-task.ipm_trc_hpm_region_init[i], 
			   task.hpm_count[task.region_current][task.hpm_eventset][i]);
		}
#endif
	    }
	}

	/* going global */
	if((region_tag > 0 && io == -1) || 
	   (region_tag ==0 && io ==  1)) {
	    task.region_current = 0;
	} else {
	    task.region_current = region_tag;
	}
    } else {
/*
 * special case of first entry ever to ipm_region expect "ipm_global"
 */
	task.region_current = region_tag;
    }

/*
 * switch on tracing if we are tracing a region
 */

    if(task.ipm_trc_nregion > 0) {
	for(i=0;i<task.ipm_trc_nregion;i++) {
	    if(!strncmp(task.region_label[task.region_current],task.ipm_trc_mask_region[i],MAXSIZE_LABEL)) {
		if(task.ipm_trc_mask_region_iti[i] == -1) {
		    task.flags |= IPM_TRC_ACTIVE;
		} else if (
		    task.ipm_trc_mask_region_iti[i] <= task.region_count[task.region_current] &&
		    task.ipm_trc_mask_region_itf[i] >= task.region_count[task.region_current]) {
		    task.flags |= IPM_TRC_ACTIVE;
		}
	    }
	}
    }

/*
 * mark begining of new region
 */ 

    if(task.region_current>0) {
	task.flags |= IPM_REGIONALIZED;
    } else {
	task.flags &= ~IPM_REGIONALIZED;
    }

    IPM_TIME_GET(T1);
    IPM_GETRUSAGE_SELF(task.ru_SELF_curr);
    region_wtime_init = IPM_TIME_SEC(T1);
    region_utime_init = IPM_UTIME_GET(task.ru_SELF_curr);
    region_stime_init = IPM_STIME_GET(task.ru_SELF_curr);
    if(!(task.flags & IPM_HPM_CANCELED)) {
	ipm_hpm_read(task.ipm_trc_hpm_region_init);
    }



#ifdef DEVEL
    if(task.flags & DEBUG) {
	printf("IPM: %d ipm_region exit (%d,\"%s\")  (now region %s %d/%d)\n", task.mpi_rank, io,tag, task.region_label[task.region_current], task.region_current, task.region_nregion);
	fflush(stdout);
    }
#endif

    return;
}


void ipm_event(char *tag) {
    static double stamp;
    static char lbuf[MAXSIZE_LABEL];
    static time_t ts;
    static IPM_RUSAGE_TYPE u;


    IPM_TIME_GTOD(stamp);
    IPM_GETRUSAGE_SELF(task.ru_SELF_curr);
    time(&ts);
    strftime(lbuf, 80, "%c", localtime(&ts));

    printf("IPM %d %s %d %s  %16.6f ---- sec %.3e MB \n", task.mpi_rank, tag, task.mpi_rank, lbuf, stamp, task.ru_SELF_curr.ru_maxrss*OOKB);

}

/* a general interface to the task data ?
   void ipm_taskdata(char *action, char *what, char *preg) {
   if(strcmp(action,"print")
   }
*/


/*
 * ipm_sync fills in parts of the structs that need updating
 * prior to any logging or reporting. assumes stamp_final is set.
 * FIXME above assumption could be removed to allow sync to be called
 * at intermiediate points. 
 */

void ipm_sync(void) {

    int j,i, ireg,icall;
    double b_flops = 0.0;
    if(task.flags & DEBUG) {
	printf("IPM: %d ipm_sync enter \n", task.mpi_rank);
    }


    /* start: sync task struct with ephemeral data */

    IPM_GETRUSAGE_SELF(task.ru_SELF_final);
    IPM_GETRUSAGE_CHILD(task.ru_CHILD_final);

    task.utime = IPM_UTIME_GET(task.ru_SELF_final);
    task.stime = IPM_STIME_GET(task.ru_SELF_final);

/* old  --> remove in 0.86
   IPM_HASH_GET_COMMTIME(task.hash,task.mtime);
   for(ireg=0;ireg<task.region_nregion;ireg++) {
   IPM_HASH_GET_COMMTIME_REGION(task.hash,task.region_mtime[ireg],ireg);
   }
*/


    /* make the call buckets right */

    for(j=0;j<MAXSIZE_REGION;j++) {
	for(i=1;i<=MAXSIZE_CALLS;i++) {
	    task.call_mcount[j][i] = 0;
	    task.call_mtime[j][i] = 0.0;
	}
    }

    for(i=0;i<=MAXSIZE_HASH;i++) {
	if(task.hash[i].key != 0) {
	    ireg=KEY_REGION(task.hash[i].key);
	    icall=KEY_CALL(task.hash[i].key);
	    if(icall == 0) {
		printf("INTERNAL_ERROR\n");
	    }
	    task.call_mtime[ireg][icall] += task.hash[i].t_tot;
	    task.call_mcount[ireg][icall] += task.hash[i].count;
	    task.region_mtime[ireg] += task.hash[i].t_tot;
	    task.mtime += task.hash[i].t_tot;
	}
    }
    /* Jie Hacked here to adjust wallclock */
#define JIE_DEBUG 0

    double adj_dur = 0.0f, sr_dur=0.0f;
    int ret = 0;

    if(task.flags & DEBUG || JIE_DEBUG) {
	printf("in ipm_sync before Jie's hack.\n");
	fflush(stdout);
    }

    if(task.flags & DEBUG || JIE_DEBUG) {
	printf("in ipm_sync before duration_adjust function call.\n");
	fflush(stdout);
    }

    ret = duration_adjust(task.stamp_init, task.stamp_final, &adj_dur, &sr_dur, WTIME_IN_USE);

    if(task.flags & DEBUG || JIE_DEBUG) {
	printf("out -- suspend-resume detected, ret is %d.\n", ret);
	fflush(stdout);
    }

 
    if( (task.stamp_final-task.stamp_init >= 10) &&
	(ret == 0) && 
	(sr_dur > 0) ) {
	printf("unexpected ret (%d) value and sr_dur (%.2f) for duration_adjust, "
	       "when task.stamp_final(%.2f)-task.stamp_init(%.2f) (%.2f) >= 10s.\n",
	       ret, sr_dur, task.stamp_final, task.stamp_init, task.stamp_final-task.stamp_init);
	fflush(stdout);
    }

    if (ret == 0) {
	task.wtime = task.stamp_final - task.stamp_init;
    
    } else if (ret != 0) {
	task.wtime = task.stamp_final - task.stamp_init - sr_dur;

	if(task.mpi_rank == 0) {
	 
	    char *workdir, *jobid, exe_file[1024];
	    char f1[] = "executable_file_info.txt";
	    FILE *fp;
	    workdir = getenv("TMPDIR");
	    jobid = getenv("PBS_JOBID");
	    snprintf(exe_file, sizeof(exe_file), "%s/%s_%s", workdir, jobid, f1);
	    fp = fopen(exe_file, "a+");
	    if(fp) {
		fprintf(fp, "<br>WARNING: PBS Suspend-Resume is detected (%.2f sec), profile results is adjusted. Profile data may contain some uncertain issues.\n", sr_dur);
		fclose(fp);
	    } else {
		printf("in ipm_sync(), file: %s -- Open failed.\n", exe_file);
	    }
	}
     
	if(task.flags & DEBUG || JIE_DEBUG) {
	    printf("in -- suspend-resume detected, ret is %d, sr_dur is %.2f.\n", ret, sr_dur);
	    fflush(stdout);
	}
    }


    if( task.wtime < (task.utime + task.stime) )
	task.wtime = task.utime + task.stime;

    if(task.mtime > task.wtime)
	task.mtime = task.wtime;

    if(task.flags & DEBUG) {
	printf("in ipm_sync after Jie's hack.\n");
	fflush(stdout);
    }

    /************** end Jie's Hack *********************************/


    ipm_get_procmem(&task.bytes);


    if(task.flags & DEBUG) {
	printf("------ in ipm_api.c -----\n");
	printf("IPM: %d ipm_env PAPI eventset %i selected, eventset_current is %d\n",
	       task.mpi_rank,task.hpm_eventset, eventset_current);
    }

    task.flops = -1.0;
    b_flops = 0.0;
    if(!(task.flags & IPM_MPI_CANCELED)) {
	for(i=0;i<task.region_nregion;i++) {
	    b_flops = IPM_HPM_CALC_FLOPS(task.hpm_count[i][task.hpm_eventset]);
	    if(b_flops != -1) {
		if(task.flops == -1.0) {task.flops = 0.0;}
		task.flops += b_flops;
	    }
	}
    }

/*  printf("IPM: %d in ipm_sync task.flops is%f. \n", task.mpi_rank, task.flops); */
/*  fflush(stdout); */

    /* end: sync task struct with ephemeral data */


    if(task.flags & DEBUG) {
	printf("IPM: %d ipm_sync exit \n", task.mpi_rank);
    }

}

