/* loop over environ and pick out IPM_ vars */
/* report IPM_X if X is unrecognized as being a liukely mispellling */
/* it is an overlay on task.flags does not assume task.flags == 0 or reset it*/

#define MAXSIZE_ENVKEY  80

extern char **environ;

/*
** ipm_get_env resets the environment : should be called only once 
** ipm_trc_select: can be called multiple times for snapshotting
*/

static int ipm_trc_select(char *opt);

static int ipm_get_env(void) {
 char **cp,key[MAXSIZE_CMDLINE],*val,*eqs;
 char *uptr,*uptr1,*uptr2,*cptr1, *cptr2;
 int i,nenv,klen,got_valid_key,got_valid_val,rv;

 double xbuf;

 char *chkpt, *pbs_share;

 /* so we can also debug this routine */
 if(getenv("IPM_DEBUG")) {
  task.flags |= DEBUG;
 }

/* Jie's hack for rounded and checkpoint and mmap sharing*/
 if(getenv("IPM_ROUNDED")){
     IPM_ROUNDED = 1;
 }
 if(getenv("IPM_CHECKPOINT")) {
     IPM_CHECKPOINT = 1;
 }
 chkpt = getenv("IPM_CHKPT_INTERVAL");
 if (chkpt != NULL) {
     CHKPT_INTERVAL = atof(chkpt);
 }

 if(getenv("OMPI_COMM_WORLD_LOCAL_RANK") != NULL) {
     MPI_LOCAL_RANK = atoi(getenv("OMPI_COMM_WORLD_LOCAL_RANK"));
 }

 if(getenv("OMPI_COMM_WORLD_LOCAL_SIZE") != NULL) {
     MPI_LOCAL_SIZE = atoi(getenv("OMPI_COMM_WORLD_LOCAL_SIZE"));
 }
 int ret; 
 ret = sprintf(IPM_MMAP_FILE, "%s/%s.%s",
	 getenv("PBS_JOBFS"),
	 getenv("IPM_MMAP_FILE"),
	 getenv("OMPI_MCA_orte_ess_jobid"));
 if(ret < 0) {
     printf("IPM: %d error in generate ipm_mmap_file.\n", 
	    task.mpi_rank);
     fflush(stdout);     
 }
 
 pbs_share = getenv("IPM_SHARE_INTERVAL");
 if (pbs_share != NULL) {
     SHARE_INTERVAL = atof(pbs_share);
 }

 if(IPM_MMAP_FILE != NULL) {
     ret = 0;
    /* Open the file */
     map_fd = open(IPM_MMAP_FILE, O_RDWR|O_CREAT, (mode_t)0600);
    if(map_fd == -1) {
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_external error opening mmaped file\n", 
		   task.mpi_rank);
	    fflush(stdout);
	}	
    }

    /* set file size to the doulbe of the actual size */
    ret = lseek(map_fd, 2*MPI_LOCAL_SIZE*sizeof(struct ipm_stats), SEEK_SET);
    if ( ret == -1) {
        close(map_fd);
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_external error calling lseek to stretch the file size.\n", 
		   task.mpi_rank);
	    fflush(stdout);
	}	
    }


    /* write to the end of the file to make sure the file has the new size */
    ret = write(map_fd, "", 1);
    if (ret != 1) {
        close(map_fd);
	if(task.flags & DEBUG) {
	    printf("IPM: %d ipm_external error writing last byte of the file.\n", 
		   task.mpi_rank);
	    fflush(stdout);
	}	
    }


    /* map the sizeof(double) to map */

    share_map = mmap(0, MPI_LOCAL_SIZE*sizeof(struct ipm_stats), PROT_READ|PROT_WRITE, MAP_SHARED, map_fd, 0);

    if (share_map == MAP_FAILED) {
        close(map_fd);
	printf("IPM: %d ipm_external error mmapping.\n", 
	       task.mpi_rank);
	fflush(stdout);
    }

    if (share_map != MAP_FAILED) {
	share_map[MPI_LOCAL_RANK].pid=getpid();
	share_map[MPI_LOCAL_RANK].mtime=0.0;
	if(task.flags & DEBUG) {
	    fprintf(stderr, "initialize :: rank %d :: local_rank %d, pid %d, mtime %.3f.\n", task.mpi_rank, MPI_LOCAL_RANK, share_map[MPI_LOCAL_RANK].pid, share_map[MPI_LOCAL_RANK].mtime);    
	}
    }
 }

/* end of Jie's hack for rounded and checkpoint and mmap sharing */

 if(task.flags & DEBUG) {
   printf("IPM: %d get_env enter\n", task.mpi_rank);
 }

/* so we know if user has specified their own output file or not*/
 sprintf(job.log_fname,"unset");
 cp = environ;

 while(*cp) {
  if(0) {printf("---> cp=%s\n", *cp);}
  if(!strncmp(*cp,"IPM_",4)) {
   got_valid_key = 0;
   got_valid_val=0;
   eqs = strchr(*cp,'=');
   klen = eqs - *cp;
   if(klen > MAXSIZE_ENVKEY) continue;
   strncpy(key,*cp,klen);
   key[klen] = '\0';
   val = eqs+1;

   if(0) {printf("---> cp=%s key=%s val=%s\n", *cp, key, val);}

   if(!strcmp(key,"IPM_DEBUG")) {
    got_valid_key = 1;
    got_valid_val = 1;
    if(val) {
     if(val[0] == '*') {
       task.flags |= DEBUG;
     } else {
      if(atoi(val) == task.mpi_rank) {
      task.flags |= DEBUG;
      }
     }
    }
   }

 if(!strcmp(key,"IPM_REPORT")) {
  got_valid_key = 1;
  cptr1 = strtok_r(val,",",&uptr);
  while(cptr1) {
   if(!strncmp(cptr1,"none",4)) {
     got_valid_val = 1;
     task.flags &= ~REPORT_TERSE;
     task.flags |= REPORT_NONE;
   }
   if(!strncmp(cptr1,"terse",5)) {
     got_valid_val = 1;
     task.flags |= REPORT_TERSE;
   }
   if(!strncmp(cptr1,"full",4)) {
     got_valid_val = 1;
     task.flags &= ~REPORT_TERSE;
     task.flags |= REPORT_FULL;
   }
   if(!strncmp(cptr1,"labelio",7)) {
     got_valid_val = 1;
     task.flags |= REPORT_LABELIO;
   }
   cptr1 = strtok_r(NULL,",",&uptr);
  }
 }

 if(!strcmp(key,"IPM_LOG")) {
  got_valid_key = 1;
  cptr1 = strtok_r(val,",",&uptr);
  while(cptr1) {
   if(!strncmp(cptr1,"none",4)) {
     got_valid_val = 1;
     task.flags &= ~LOG_TERSE;
     task.flags |= LOG_NONE;
   }
   if(!strncmp(cptr1,"terse",5)) {
     got_valid_val = 1;
     task.flags |= LOG_TERSE;
   }
   if(!strncmp(cptr1,"full",4)) {
     got_valid_val = 1;
     task.flags &= ~LOG_TERSE;
     task.flags |= LOG_FULL;
   }
   cptr1 = strtok_r(NULL,",",&uptr);
  }
 }


  if(!strcmp(key,"IPM_HPM")) {

      if(task.flags & DEBUG) {
	  printf("------ before -----\n");
	  printf("IPM: %d ipm_env PAPI eventset %i selected\n",task.mpi_rank,task.hpm_eventset);
	  printf("IPM: IPM_HPM value is %s.\n",val);
      }

      got_valid_key = 1;
      cptr1 = strtok_r(val,",",&uptr);
      i=0;
      while(cptr1) {
	  
	  task.hpm_eventset = atoi(cptr1); /* generic case ala pwr4 w/ many sets */
	  got_valid_val = 1;
	  
	  
	  if(!strncmp(cptr1,"no",2)) {
	      got_valid_val = 1;
	      task.hpm_eventset=-1;
	      task.flags |= IPM_HPM_CANCELED;
	      task.flags &= ~IPM_HPM_ACTIVE;
	  }
	  
	  if(!strncmp(cptr1,"scan",4)) {
	      got_valid_val = 1;
	      task.hpm_eventset=MAXSIZE_NEVENTSETS%task.mpi_rank;
	  }
#ifdef CPU_POWER4
	  if(!strncmp(cptr1,"5",1)) {
	      got_valid_val = 1;
	      task.hpm_eventset=2;
	  }
	  if(!strncmp(cptr1,"60",2)) {
	      got_valid_val = 1;
	      task.hpm_eventset=0;
	  }
	  if(!strncmp(cptr1,"59",2)) {
	      got_valid_val = 1;
	      task.hpm_eventset=1;
	  }
	  if(!strncmp(cptr1,"58",2)) {
	      got_valid_val = 1;
	      task.hpm_eventset=3;
	  }
	  if(!strncmp(cptr1,"53",2)) {
	      got_valid_val = 1;
	      task.hpm_eventset=4;
	  }
#endif

/* FIXME :: accept a list of PAPI_EVENTS */
/* here we assume all events have a _ in their string */
/* which seems resonable - but its not completely foolproof */
	  if(strchr(cptr1,'_')) {
	      got_valid_val = 1;
	      task.hpm_eventset=MAXSIZE_NEVENTSETS-1;
	      ipm_hpm_ename[MAXSIZE_NEVENTSETS-1][i%MAXSIZE_HPMCOUNTERS]=cptr1;
	  }

	  if(task.flags & DEBUG) {
	      printf("------ in -----\n");
	      printf("IPM: cptr1 is %s\n", cptr1);
	  }
	  
	  cptr1 = strtok_r(NULL,",",&uptr);
	  i++;

      }
      
      if(!(task.flags & IPM_HPM_CANCELED)) {
	  if(task.hpm_eventset<0) task.hpm_eventset=0;
      }

      if(task.flags & DEBUG) {
	  printf("------ after -----\n");
	  printf("IPM: %d ipm_env PAPI eventset %i selected\n",task.mpi_rank,task.hpm_eventset);
	  printf("IPM: IPM_HPM value is %s.\n",val);
      }

  }
  
  
 /* { NOOP */
 if(!strcmp(key,"IPM_MPI_NOOP")) {
  got_valid_key = 1;
  if(task.flags & DEBUG) {
   printf("IPM: %d MASK NOOP enter\n", task.mpi_rank);
   fflush(stdout);
  }

 cptr1 = strtok_r(val,"+",&uptr);
  while(cptr1) {
   printf("IPM: %d MASK NOOP %s\n", task.mpi_rank,cptr1);
   if(!strncmp(cptr1,"*",MAXSIZE_LABEL)) {
    got_valid_val = 1;
    if(task.flags & DEBUG) {
      printf("IPM: %d MASK NOOP *\n", task.mpi_rank);
    }
    for(i=1;i<=MAXSIZE_CALLS;i++) {
      task.call_mask[i] |= CALL_OPT_NOOP;
    }
   }
   for(i=1;i<=MAXSIZE_CALLS;i++) {
     if(!strncmp(cptr1,task.call_label[i],MAXSIZE_LABEL)) {
      got_valid_val = 1;
      task.call_mask[i] |= CALL_OPT_NOOP;
      if(task.flags & DEBUG) {
       printf("IPM: %d MASK NOOP %s\n", task.mpi_rank,task.call_label[i]);
      }
     }
    }
    cptr1 = strtok_r(NULL,"+",&uptr);
   }

  if(task.flags & DEBUG) {
   printf("IPM: %d MASK NOOP exit\n", task.mpi_rank);
   fflush(stdout);
  }
 }
 /* } NOOP */


 /* { PASS */
 if(!strcmp(key,"IPM_MPI_PASS")) {
  got_valid_key = 1;
  if(task.flags & DEBUG) {
   printf("IPM: %d MASK PASS enter\n", task.mpi_rank);
   fflush(stdout);
  }
 cptr1 = strtok_r(val,"+",&uptr);
  while(cptr1) {
   if(!strncmp(cptr1,"*",MAXSIZE_LABEL)) {
    got_valid_val = 1;
    if(task.flags & DEBUG) {
     printf("IPM: %d MASK PASS *\n", task.mpi_rank);
    }
    for(i=1;i<=MAXSIZE_CALLS;i++) {
      task.call_mask[i] |= CALL_OPT_PASS;
    }
   }
   for(i=1;i<=MAXSIZE_CALLS;i++) {
     if(!strncmp(cptr1,task.call_label[i],MAXSIZE_LABEL)) {
      got_valid_val = 1;
      task.call_mask[i] |= CALL_OPT_PASS;
     }
    }
    cptr1 = strtok_r(NULL,"+",&uptr);
   }

 if(task.flags & DEBUG) {
  for(i=1;i<=MAXSIZE_CALLS;i++) {
   if(task.call_mask[i] & CALL_OPT_PASS) {
    printf("IPM: %d MASK PASS %s\n", task.mpi_rank,task.call_label[i]);
   }
  }
 }

 if(task.flags & DEBUG) {
  printf("IPM: %d MASK PASS exit\n", task.mpi_rank);
  fflush(stdout);
 }
 }
 /* } PASS */


#ifndef IPM_DISABLE_LOG
 if(!strcmp(key,"IPM_MPI_TRACE")) {
  got_valid_key = 1;
  got_valid_val = ipm_trc_select(val);
 }
#endif


  if(!strcmp(key,"IPM_CODENAME")) {
   got_valid_key = 1;
   got_valid_val = 1;
   sprintf(job.codename, "%s", val);
  }

  if(!strcmp(key,"IPM_PRELOAD")) {
   got_valid_key = 1;
   got_valid_val = 1;
   task.flags |= IPM_PRELOAD;
  }

  if(!strcmp(key,"IPM_MPI_MOD")) {
   got_valid_key = 1;
   cptr1 = strtok_r(val,",",&uptr);
   while(cptr1) {

   if(!strncmp(cptr1,"bmw",4)) {
    got_valid_val = 1;
    task.flags |= IPM_MPI_BREAKWAIT;
   }

   if(!strncmp(cptr1,"bbs",4)) {
    got_valid_val = 1;
    task.flags |= IPM_MPI_PREBARFSYNC;
   }

   cptr1 = strtok_r(NULL,",",&uptr);
  }

  }

  if(!strcmp(key,"IPM_LOGFILE")) {
   got_valid_key = 1;
   got_valid_val = 1;
   strncpy(job.log_fname,val, MAXSIZE_FILENAME);
  } 

 if(!strcmp(key,"IPM_PARALLEL_IO")) {
   got_valid_key = 1;
   if(!strncmp(val,"n",1)) {
     got_valid_val = 1;
     task.flags &= ~PARALLEL_IO_LOG;
   }
   if(!strncmp(val,"y",1)) {
     got_valid_val = 1;
     task.flags |= PARALLEL_IO_LOG;
   }
  }

  if(!strcmp(key,"IPM_LOGDIR")) {
   got_valid_key = 1;
   got_valid_val = 1;
   strncpy(job.log_dir,val, MAXSIZE_FILENAME);
  } 

   if(!got_valid_key && !task.mpi_rank) {
    printf("IPM: %d environment variable %s is unrecognized \n",
	 task.mpi_rank,key);
   }

   if(!got_valid_val && got_valid_key) {
    printf("IPM: %d environment variable %s had unexpected value \"%s\" \n",
	task.mpi_rank,key, val);
   }

  } else {
/* non IPM_VAR */
  }
  cp ++;
 }

/*
 for(i=1;i<=MAXSIZE_CALLS;i++) {
  if(task.call_mask[i]) {
   printf("IPM: env call %d %s mask %d\n",
	 task.mpi_rank, task.call_label[i], task.call_mask[i]);
  }
 }
*/

 if(task.flags & DEBUG) {
  printf("IPM: %d get_env exit\n", task.mpi_rank);
 }

 return 1;
}

static int ipm_trc_select(char *opt) {

 char *uptr,*uptr1,*uptr2,*cptr1, *cptr2;
 int i,nenv,klen,got_valid_key,got_valid_val,rv;
 double xbuf;

  if(task.flags & DEBUG) {
   printf("IPM: %d TRACE IPM_MPI_TRACE=%s \n", task.mpi_rank, opt);
  }

  /* turn on all tracing, then mask out by region, call, time */

  task.flags |= IPM_TRC_ACTIVE;
  for(i=1;i<=MAXSIZE_CALLS;i++) {
   task.call_mask[i] |= CALL_OPT_TRACE;
  }

  for(i=0;i<MAXSIZE_REGION;i++) {
   task.ipm_trc_mask_region_iti[i] = 1;
   task.ipm_trc_mask_region_itf[i] = -1;
   task.ipm_trc_mask_region[i][0] = '\0';
  }

  cptr1 = strtok_r(opt,",",&uptr);
  while(cptr1) {

   if(task.flags & DEBUG) {
    printf("IPM: %d TRACE get_project %s\n", task.mpi_rank,cptr1);
   }

   if(!strncmp(cptr1,"region:",7)) {
    got_valid_val = 1;
    cptr2 = strtok_r(cptr1+7,"+",&uptr2);
    while(cptr2) {
     if(strchr(cptr2,'[') && strchr(cptr2,'-') && strchr(cptr2,']')) {
         /* trace passes iti to itf through given region */
      for(i=0;i<strlen(cptr2);i++) {
       if(cptr2[i] == '[') cptr2[i] = ' ';
       if(cptr2[i] == '-') cptr2[i] = ' ';
       if(cptr2[i] == ']') cptr2[i] = ' ';
      }
      rv=sscanf(cptr2,"%s %d %d ",
         task.ipm_trc_mask_region[task.ipm_trc_nregion],
         &task.ipm_trc_mask_region_iti[task.ipm_trc_nregion],
         &task.ipm_trc_mask_region_itf[task.ipm_trc_nregion]);
     } else { /* trace all passes through given region */
      strncpy(task.ipm_trc_mask_region[task.ipm_trc_nregion], cptr2, MAXSIZE_LABEL);
      task.ipm_trc_mask_region_iti[task.ipm_trc_nregion] = -1;
      task.ipm_trc_mask_region_itf[task.ipm_trc_nregion] = -1;
     }
     if(task.ipm_trc_nregion >= MAXSIZE_REGION) {
      printf("IPM: %d ERROR too many regions specified in IPM_MPI_TRACE\n",
         task.mpi_rank);
     }
     task.ipm_trc_nregion++;
     cptr2 = strtok_r(NULL,"+",&uptr2);
    }
   }

   if(!strncmp(cptr1,"rank:",5)) {
    got_valid_val = 1;
    task.flags |= IPM_TRC_CANCELED;
    task.flags &= ~IPM_TRC_ACTIVE;
    cptr2 = strtok_r(cptr1+5,"+",&uptr2);
    while(cptr2) {
     if(!strcmp(cptr2,"even") && task.mpi_rank%2 == 0) {
         task.flags &= ~IPM_TRC_CANCELED;
         task.flags |= IPM_TRC_ACTIVE;
     }
     if(!strcmp(cptr2,"odd") && task.mpi_rank%2 != 0) {
         task.flags &= ~IPM_TRC_CANCELED;
         task.flags |= IPM_TRC_ACTIVE;
     }
     if(atoi(cptr2) == task.mpi_rank) {
         task.flags &= ~IPM_TRC_CANCELED;
         task.flags |= IPM_TRC_ACTIVE;
     }
     cptr2 = strtok_r(NULL,"+",&uptr2);
    }
     if(task.flags & DEBUG) {
      if(task.flags & IPM_TRC_ACTIVE) {
       printf("IPM: %d TRACE rank yes \n", task.mpi_rank);
      } else {
       printf("IPM: %d TRACE rank no \n", task.mpi_rank);
      }
     }
   }


   if(!strncmp(cptr1,"time:",5)) {
    got_valid_val = 1;
    rv = sscanf(cptr1,"time:[%lf+%lf]", &task.ipm_trc_mask_time_init, &task.ipm_trc_mask_time_final);
    if(rv == 2 && task.ipm_trc_mask_time_init >= 0.0 && task.ipm_trc_mask_time_final >= 0.0) {
     if(task.flags & DEBUG) {
      printf("IPM: %d got_trace time(%f %f) \n",
        task.mpi_rank,
        task.ipm_trc_mask_time_init,
        task.ipm_trc_mask_time_final);
     }
    }
   }


   if(!strncmp(cptr1,"call:",5)) {
    got_valid_val = 1;
    for(i=1;i<=MAXSIZE_CALLS;i++) {
     task.call_mask[i] &= ~CALL_OPT_TRACE; /* undo the above assumption */
    }
    cptr2 = strtok_r(cptr1+5,"+",&uptr2);
    while(cptr2) {
     for(i=1;i<=MAXSIZE_CALLS;i++) {
      if(!strncmp(cptr2,task.call_label[i],MAXSIZE_LABEL)) {
       task.call_mask[i] |= CALL_OPT_TRACE;
      }
     }

     if(!strncmp(cptr2,"MPI_P2P",MAXSIZE_LABEL)) {
      for(i=1;i<=MAXSIZE_CALLS;i++) {
       if(task.call_mask[i] & (CALL_SEM_DATA_TX || CALL_SEM_DATA_RX || CALL_SEM_DATA_TXRX || CALL_SEM_CS_ASYNC)) {
        task.call_mask[i] |= CALL_OPT_TRACE;
       }
      }
     }

     if(!strncmp(cptr2,"MPI_COL",MAXSIZE_LABEL)) {
      for(i=1;i<=MAXSIZE_CALLS;i++) {
       if(task.call_mask[i] & (CALL_SEM_CS_FSYNC || CALL_SEM_CS_PSYNC)) {
        task.call_mask[i] |= CALL_OPT_TRACE;
       }
      }
     }
     cptr2 = strtok_r(NULL,"+",&uptr2);
    }
   }


#ifndef IPM_DISABLE_LOG
   if(!strncmp(cptr1,"mem:",4)) {
    got_valid_val = 1;
    rv=sscanf(cptr1+4,"%lf",&xbuf);
    task.ipm_trc_count_max = 1024*1024*xbuf/sizeof(struct ipm_trc_ent);
   }
#endif 

   cptr1 = strtok_r(NULL,",",&uptr);
  }


  if(task.flags & DEBUG) {
   if(task.flags & IPM_TRC_ACTIVE) {
   if(task.ipm_trc_mask_time_init == -1.0 && task.ipm_trc_mask_time_final==-2.0) {
    printf("IPM: %d TRACE mask_time=* \n", task.mpi_rank);
   } else  {
   printf("IPM: %d TRACE mask_time=%f,%f \n",
        task.mpi_rank,
        task.ipm_trc_mask_time_init,
        task.ipm_trc_mask_time_final);
   }
   printf("IPM: %d TRACE mask_region=", task.mpi_rank);
   if(task.ipm_trc_nregion) {
    for(i=0;i<task.ipm_trc_nregion;i++) {
     printf("%s[%d-%d]+",
        task.ipm_trc_mask_region[i],
        task.ipm_trc_mask_region_iti[i],
        task.ipm_trc_mask_region_itf[i]);
    }
    printf("\n");
   } else {
    printf("*\n");
   }
   rv = 0;
   for(i=1;i<=MAXSIZE_CALLS;i++) {
    if(task.call_mask[i] & CALL_OPT_TRACE)  rv = 1;
   }
   if(rv == 0) {
    printf("IPM: %d TRACE mask_call=*\n", task.mpi_rank);
   } else {
    printf("IPM: %d TRACE mask_call=", task.mpi_rank);
    for(i=1;i<=MAXSIZE_CALLS;i++) {
     if(task.call_mask[i] & CALL_OPT_TRACE) {
      printf("%s+", task.call_label[i]);
     }
    }
    printf("\n");
   }
   printf("IPM: %d TRACE active=%d \n",
        task.mpi_rank, 1);
   } else {
   printf("IPM: %d TRACE active=%d \n",
        task.mpi_rank, 0);
   }
  }

 return got_valid_val;

}

