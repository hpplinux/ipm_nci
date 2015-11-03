static void ipm_log_write(struct ipm_jobdata *j, struct ipm_taskdata *t, char *literal, FILE *fh) {
 int i,ii,rv; 
 double b_flops;
 FILE *in_fh;
 int ireg, kreg,ib;
 char **envp;
 char cmd[MAXSIZE_TXTLINE], *cp;
 int icall, ibytes, irank, log_rank=-1;
 ipm_hash_ent hbuf,*hbufp;


/*********************************************/
/* emit a task profile to ipm_mpi_log_fh  */
/*********************************************/

   rv = fprintf(fh, "<task ipm_version=\"%s\" cookie=\"%s\" mpi_rank=\"%d\" mpi_size=\"%d\" stamp_init=\"%.6f\" stamp_final=\"%.6f\" username=\"%s\" groupname=\"%s\" flags=\"%lld\" pid=\"%d\" >\n",
	IPM_VERSION,
	j->cookie,
	t->mpi_rank,
	t->mpi_size,
	t->stamp_init,
	t->stamp_final,
	j->username,
	j->groupname,
	t->flags,
	t->pid);


   rv = fprintf(fh,
	 "<job nhosts=\"%d\" ntasks=\"%d\" start=\"%d\" final=\"%d\" cookie=\"%s\" code=\"%s\" >%s</job>\n",
        j->nhosts,
        j->ntasks,
	(int)t->stamp_init,
	(int)t->stamp_final,
	j->cookie,
	j->codename,
	j->id);

   rv = fprintf(fh,
	 "<host mach_name=\"%s\" mach_info=\"%s\" >%s</host>\n",
	IPM_HPCNAME,
	t->mach_info,
	t->hostname);
   
   
   if(t->flops == -1.0) {
    b_flops = -1.0;
   } else {
    b_flops = t->flops*OOGU;
   }

   
   rv = fprintf(fh,
         "<perf wtime=\"%.5e\" utime=\"%.5e\" stime=\"%.3e\" mtime=\"%.5e\" gflop=\"%.5e\" gbyte=\"%.5e\" ></perf>\n",
	t->wtime,
	t->utime,
	t->stime,
	t->mtime,
	b_flops,
	t->bytes*OOGB);
	
   rv = fprintf(fh,
         "<switch bytes_tx=\"%.5e\" bytes_rx=\"%.5e\" > %s </switch>\n",
	t->switch_bytes_tx,
	t->switch_bytes_rx,
	t->switch_name);

   rv = fprintf(fh, "<cmdline realpath=\"%s\" >%s</cmdline>\n",
	 t->mpi_realpath,
	 t->mpi_cmdline);

#ifndef JIE_HACK_EXECINFO
#define JIE_HACK_EXECINFO
#endif

#ifdef JIE_HACK_EXECINFO
   rv = ipmprintf(fh,buff_in,pflag, "<exec><pre>\n");
   
   /* get the PBS_O_WORKDIR env and prepare ldd output file full path 
    * and file output file full path from generated file from mpirun */
   char *workdir, *jobid, ldd_file[1024], exe_file[1024];
   char f1[] = "executable_file_info.txt";
   char f2[] = "executable_ldd_info.txt";
   workdir = getenv("TMPDIR");
   jobid = getenv("PBS_JOBID");

   snprintf(exe_file, sizeof(exe_file), "%s/%s_%s", workdir, jobid, f1);
   snprintf(ldd_file, sizeof(ldd_file), "%s/%s_%s", workdir, jobid, f2);

   /* file information should read from 
    * $PBS_O_WORKDIR/executable_file_info.txt */  
   FILE *fp=NULL;
   
   fp = fopen(exe_file, "r");
   
   if(fp) {       
       char tmp[MAXSIZE_TXTLINE];
       while(fgets(tmp, MAXSIZE_TXTLINE, fp)) {
	   rv=ipmprintf(fh, buff_in, pflag, "%s", tmp);
       }
       fclose(fp);
   } else {
       printf("in ipm_log_write_ascii, file: %s -- Open failed.\n", exe_file);
       fflush(stdout);
       rv = ipmprintf(fh,buff_in,pflag, "unknown\n");
   }
   rv = ipmprintf(fh,buff_in,pflag, "</pre></exec>\n");

   /* ldd information should read from 
    * $PBS_O_WORKDIR/executable_ldd_info.txt */
   rv = ipmprintf(fh,buff_in,pflag, "<exec_bin><pre>\n");
   
   fp = NULL;
   fp = fopen(ldd_file, "r");

   if(fp) {       
       char tmp[MAXSIZE_TXTLINE];
       while(fgets(tmp, MAXSIZE_TXTLINE, fp)) {
	   rv=ipmprintf(fh, buff_in, pflag, "%s", tmp);
       }
       fclose(fp);      
   } else {
       printf("in ipm_log_write_ascii, file: %s -- Open failed.\n", ldd_file);
       fflush(stdout);
       rv = ipmprintf(fh,buff_in,pflag, "unknown\n"); 
   }
   rv = ipmprintf(fh,buff_in,pflag, "</pre></exec_bin>\n");   
#endif /* end of #ifdef JIE_HACK_EXECINFO */


#ifndef IPM_DISABLE_EXECINFO

/* the fgets here should go away FIXME */
    rv = fprintf(fh, "<exec><pre>\n");
    if(strcmp(t->mpi_realpath,"unknown")) {
     sprintf(cmd, "/usr/bin/file  %s\n", t->mpi_realpath);
     in_fh = popen(cmd,"r"); 
     if(in_fh) {
      while(fgets(cmd,MAXSIZE_TXTLINE,in_fh)) {
       fprintf(fh, "%s", cmd);
      }
      pclose(in_fh);
     }
    } else {
     rv = fprintf(fh, "unknown\n");
    }
    rv = fprintf(fh, "</pre></exec>\n");

    
    rv = fprintf(fh, "<exec_bin><pre>\n");
    if(strcmp(t->mpi_realpath,"unknown")) {
    sprintf(cmd,"");
#ifdef LINUX_X86
     sprintf(cmd, "/usr/bin/ldd  %s\n", t->mpi_realpath);
#endif
#ifdef LINUX_AMD64
     sprintf(cmd, "/usr/bin/ldd  %s\n", t->mpi_realpath);
#endif
#ifdef AIX_POWER
      sprintf(cmd, "/usr/bin/dump -X32_64 -H %s\n", t->mpi_realpath);
#endif
     if(strlen(cmd) > 1) {
      in_fh = popen(cmd,"r"); 
      if(in_fh) {
       while(fgets(cmd,MAXSIZE_TXTLINE,in_fh)) {
        fprintf(fh, "%s", cmd);
       }
       pclose(in_fh);
      }
     }  else {
      rv = fprintf(fh, "unknown\n");
     }
    } else {
     rv = fprintf(fh, "unknown\n"); 
    }
    rv = fprintf(fh, "</pre></exec_bin>\n");
#endif

   if(strlen(literal) > 0 ) {
      rv = fprintf(fh, "%s\n", literal);
   }

   if(1) {
   if(t->mpi_rank == 0) {
    envp = environ;
    while(*envp) {
     if(strstr(*envp,"=")) {
      rv = fprintf(fh, "<env>%s</env>\n", *envp);
      envp++;
     } else {
      break;
     }
    }
   }
   }

#define FORMAT_RU(key,u) {\
   fprintf(fh,\
	"<%s>%.4e %.4e %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld</%s>\n",\
	key,\
        u.ru_utime.tv_sec + 1.0e-6*u.ru_utime.tv_usec,\
        u.ru_stime.tv_sec + 1.0e-6*u.ru_stime.tv_usec,\
	(long long int)u.ru_maxrss,\
	(long long int)u.ru_ixrss,\
	(long long int)u.ru_idrss,\
	(long long int)u.ru_isrss,\
	(long long int)u.ru_minflt,\
	(long long int)u.ru_majflt,\
	(long long int)u.ru_nswap,\
	(long long int)u.ru_inblock,\
	(long long int)u.ru_oublock,\
	(long long int)u.ru_msgsnd,\
	(long long int)u.ru_msgrcv,\
	(long long int)u.ru_nsignals,\
	(long long int)u.ru_nvcsw,\
	(long long int)u.ru_nivcsw,\
        key);\
}
   FORMAT_RU("ru_s_ti",t->ru_SELF_init);
   FORMAT_RU("ru_s_tf",t->ru_SELF_final);
   FORMAT_RU("ru_c_ti",t->ru_CHILD_init);
   FORMAT_RU("ru_c_tf",t->ru_CHILD_final);

#undef FORMAT_RU

   
   rv = fprintf(fh, "<call_mask >\n");
   for(icall=0;icall<=MAXSIZE_CALLS;icall++) {
    if( (~CALL_BUF_PRECISION(icall) & CALL_MASK_BPREC) ) {
     rv = fprintf(fh, "<call prec=\"%lld\" > </call>\n",
	(~CALL_BUF_PRECISION(icall) & CALL_MASK_BPREC));
    
    }
   }
   rv = fprintf(fh, "</call_mask >\n");

   rv = fprintf(fh, "<regions n=\"%d\" >\n",t->region_nregion);
 for(ireg=0;ireg<t->region_nregion;ireg++) {
   rv = fprintf(fh,
	 "<region label=\"%s\" nexits=\"%lld\" wtime=\"%.4e\" utime=\"%.4e\" stime=\"%.4e\" mtime=\"%.4e\" >\n ",
	t->region_label[ireg],
	t->region_count[ireg],
	t->region_wtime[ireg],
	t->region_utime[ireg],
	t->region_stime[ireg],
	t->region_mtime[ireg]);
   rv = fprintf(fh,
	"<hpm api=\"%s\" ncounter=\"%d\" eventset=\"%d\" gflop=\"%.4e\" >\n",
#ifdef HPM_PAPI
	"PAPI",
#endif
#ifdef HPM_PMAPI
	"PMAPI",
#endif
#ifdef HPM_NOOP
	"NOOP",
#endif
	MAXSIZE_HPMCOUNTERS,
	t->hpm_eventset,
	IPM_HPM_CALC_FLOPS(t->hpm_count[ireg][t->hpm_eventset])*OOGU);
   if(t->flags & IPM_HPM_CANCELED) {
    rv = fprintf(fh,"CANCELED");
   } else {

    for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {
     ii = ipm_hpm_eorder[t->hpm_eventset][i];
     rv = fprintf(fh, "<counter name=\"%s\" > %lld </counter>\n",
         ipm_hpm_ename[t->hpm_eventset][ii],
	 t->hpm_count[ireg][t->hpm_eventset][ii]
	);
   }
  }
  rv = fprintf(fh, "</hpm>\n");

   for(icall=0;icall<=MAXSIZE_CALLS;icall++) {
    if(t->call_mcount[ireg][icall] > 0.0) {

     rv=fprintf(fh,"<func name=\"%s\" count=\"%lld\" > %.4e </func>\n",
	t->call_label[icall],
	t->call_mcount[ireg][icall],
	t->call_mtime[ireg][icall]);

    }
   }

   rv = fprintf(fh,"</region>\n");
 }
   rv = fprintf(fh, "</regions>\n");

  if(task.flags & LOG_FULL) {
   rv = fprintf(fh,
	"<hash nlog=\"%lld\" nkey=\"%lld\" >\n",t->hash_nlog, t->hash_nkey);
   for(i=0;i<=MAXSIZE_HASH;i++) {
    hbufp = &(t->hash[i]);
    if(hbufp->key != 0) {
      kreg = KEY_REGION(hbufp->key);
      icall = KEY_CALL(hbufp->key);
      ibytes = KEY_BYTE(hbufp->key);
      irank = KEY_RANK(hbufp->key);
      if(icall <= 0 || icall > MAXSIZE_CALLS || ibytes < 0 || (irank > -1 && irank >= t->mpi_size)) {
   if(task.flags & DEBUG) {
 printf("IPM: %5d HENT_ERR key_test \n",task.mpi_rank);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key); printf(" k=%lld\n",hbufp->key);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_REGION); printf(" k&region=%d\n",kreg);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_CALL); printf(" k&call=%d,%s\n",icall,task.call_label[icall]);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_RANK); printf(" k&rank=%d\n",irank);
 printf("IPM: %5d HENT_ERR key_test ",task.mpi_rank); IPM_SHOWBITS(hbufp->key&KEY_MASK_BYTE); printf(" k&byte=%d\n",ibytes);
 printf("IPM: %5d HENT_ERR key_test \n",task.mpi_rank);
  fflush(stdout);
    }
   }
 
   if(t->call_mask[icall] & CALL_SEM_RANK_NONE) {
    irank = -1;
   }
  
   if(irank > t->mpi_size) {
    irank = -1;
   }

   if(kreg <0 || kreg >= MAXSIZE_REGION) {
      printf("IPM: %d Hash error key=%lld region=%d > MAXSIZE_REGION\n", 
       t->mpi_rank,
       hbufp->key,
       KEY_REGION(hbufp->key)
      );
   }

#ifdef IPM_ENABLE_PROFLOW
      rv = fprintf(fh,
	"<hent key=\"%lld\" key_last=\"%lld\" call=\"%s\" bytes=\"%d\" orank=\"%d\" region=\"%d\" count=\"%lld\" >%.4e %.4e %.4e</hent>\n",
	 hbufp->key,
	 hbufp->key_last,
	 t->call_label[icall],
	 ibytes,
	 irank,
	 KEY_REGION(hbufp->key),
	 hbufp->count,
	 hbufp->t_tot,
         hbufp->t_min,
         hbufp->t_max);
#else
      rv = fprintf(fh,
	"<hent key=\"%lld\" call=\"%s\" bytes=\"%d\" orank=\"%d\" region=\"%d\" count=\"%lld\" >%.4e %.4e %.4e</hent>\n",
	 hbufp->key,
	 t->call_label[icall],
	 ibytes,
	 irank,
	 KEY_REGION(hbufp->key),
	 hbufp->count,
	 hbufp->t_tot,
	 hbufp->t_min,
         hbufp->t_max);
#endif
     }
    }
    rv = fprintf(fh,"</hash>\n");
   }
   IPM_TIME_GTOD(stamp_current);
   rv = fprintf(fh, "<internal rank=\"%d\" log_i=\"%12.6f\" log_t=\"%.4e\" report_delta=\"%.4e\" fname=\"%s\" logrank=\"%d\" ></internal>\n",
	t->mpi_rank,
	stamp_current,
	stamp_current-t->stamp_final,
        report_delta,
	j->log_fname,
	log_rank);
   rv = fprintf(fh,"</task>\n\n");
   fflush(fh);
 return;
}


