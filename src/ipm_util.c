static int index_doubles(int n, int *idx, double *val) {
 int i,j,k;

 for(i=0;i<n;i++) {
  idx[i] = i;
 }

 for(i=0;i<n;i++) {
  for(j=0;j<n;j++) {
   if(val[idx[i]] > val[idx[j]]) {
    k = idx[i];
    idx[i] = idx[j];
    idx[j] = k;
   }
  }
 }

 return 0;
}

static int ipm_hash_cull(char *tag) {
 int i,j,done,most_dense_call=0,found,nkey_save=0;
 int icall,ireg,irank,ibyte;
 int jcall,jreg,jrank,jbyte;
 long long int most_dense_count=0;
 unsigned long long int optnstate,bufprec;
 double cur_mpi_time=0.0, worst_case;
 double time_in, time_out, mtime_current;
 ipm_hash_ent *hbuf,*hbuf_save;
 IPM_KEY_TYPE key,hkey;
 int ib; /* remove this when SHOWBITS goes away */

 IPM_TIME_GTOD(time_in);

 if(task.flags & DEBUG) {
  printf("IPM: %d ipm_mpi_util enter %s nkeys=%lld time=%.6f\n",
        task.mpi_rank,tag, task.hash_nkey, time_in);
  fflush(stdout);
 }

 if(!strcmp(tag,"MAXSIZE_HASHLIMIT")) {
#ifndef IPM_DISABLE_HASHCOMPRESS

  for(j=0;j<MAXSIZE_REGION;j++) {
   for(i=1;i<=MAXSIZE_CALLS;i++) {
     task.call_mcount[j][i] = 0;
     task.call_mtime[j][i] = 0.0;
    }
   }

   for(i=0;i<=MAXSIZE_HASH;i++) {
    icall = KEY_CALL(task.hash[i].key);
    task.call_mtime[0][icall] +=  task.hash[i].t_tot;
    task.call_mcount[0][icall] += 1;
   }
  
  for(i=1;i<=MAXSIZE_CALLS;i++) {
/*
    if(task.call_mcount[0][i] > 0 ) {
     printf("call density %d %lld\n",i,task.call_mcount[0][i]);
    }
*/
    if(task.call_mcount[0][i] > most_dense_count)  {
     most_dense_count = task.call_mcount[0][i];
     most_dense_call = i;
    }
  }


  /* make copy of the table */
  nkey_save = task.hash_nkey;
  hbuf_save =
    (ipm_hash_ent *)malloc((size_t)((MAXSIZE_HASH+1)*sizeof(ipm_hash_ent)));
  if(!hbuf_save) {
   printf(" %d %lld\n", most_dense_call ,  most_dense_count);
   printf("IPM: %d out of memory in ipm_mpi_util\n",
	task.mpi_rank);
  }
  memcpy(hbuf_save,task.hash,(MAXSIZE_HASH+1)*sizeof(ipm_hash_ent));
  IPM_HASH_CLEAR(task.hash);
  task.hash_nkey = 0; 
  
  /* now modify the mask */
/*
 printf("IPM: %d precision down call = %d count = %lld\n",
	 task.mpi_rank,most_dense_call,most_dense_count);
 printf("IPM: %d mask_o ",task.mpi_rank);
 IPM_SHOWBITS(task.call_mask[most_dense_call]);
 printf("\n");
*/
 CALL_BUF_PRECISION_DOWN(most_dense_call);
/*
 printf("IPM: %d mask_n ",task.mpi_rank);
 IPM_SHOWBITS(task.call_mask[most_dense_call]);
 printf("\n");
*/
 if(!(task.call_mask[most_dense_call] & CALL_MASK_BPREC)) {
  printf("IPM: %d ERROR precision underflow call = %d,%s\n",
	task.mpi_rank, most_dense_call, task.call_label[most_dense_call]);
  fflush(stdout);
  exit(1);
 }

 /* insert the saved data into the table w/ new mask */

  for(i=0;i<=MAXSIZE_HASH;i++) {
   if(hbuf_save[i].key) {
    ireg = KEY_REGION(hbuf_save[i].key);
    icall = KEY_CALL(hbuf_save[i].key);
    irank = KEY_RANK(hbuf_save[i].key);
    ibyte = KEY_BYTE(hbuf_save[i].key);
    IPM_MPI_KEY(key,ireg,icall,irank,ibyte);
/*
    if(0 || icall == most_dense_call) {
     printf("%d %d --> ", icall, ibyte);
     ibyte = KEY_BYTE(key);
     printf("%d\n", ibyte);
     IPM_SHOWBITS(hbuf_save[i].key); printf("\n");
     IPM_SHOWBITS(key); printf("\n");
    }
*/
    IPM_HASH_HKEY(task.hash,key,hkey);
    task.hash[hkey].count += hbuf_save[i].count;
    task.hash[hkey].t_tot += hbuf_save[i].t_tot;
    if(hbuf_save[i].t_max > task.hash[hkey].t_max)
	 task.hash[hkey].t_max = hbuf_save[i].t_max;
    if(hbuf_save[i].t_min < task.hash[hkey].t_min)
	 task.hash[hkey].t_min = hbuf_save[i].t_min;
   }
  }

  free((char *)hbuf_save);

  IPM_TIME_GTOD(time_out);
  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_mpi_util exit  %s nkeys=%lld ncull=%lld delta=%f \n",
        task.mpi_rank,
	tag,
	task.hash_nkey,
	nkey_save-task.hash_nkey,
	time_out-time_in);
   fflush(stdout);
  }

#else 
/* this code makes the mistake of deleting from the table w/o correcting for how
   that changes the hash function (we are double hashing). The net effect is to 
   make possible multiple entries for the same (call,rank,size), which is 
   not good but could be recoverable.
  IPM_HASH_GET_COMMTIME(task.hash,mtime_current);
  worst_case = mtime_current/task.hash_nkey;
  done = 0;
  while(!done) {
   for(i=0;i<=MAXSIZE_HASH;i++) {
   hbufp = &(task.hash[i]);
   if(hbufp->key != 0 && hbufp->t_tot < worst_case) {
    icall = KEY_CALL(hbufp->key);
    ireg = KEY_REGION(hbufp->key);
    task.call_mtime[ireg][icall] +=  hbufp->t_tot;
    task.call_mcount[ireg][icall] +=  hbufp->count;
    hbufp->key=0;
    hbufp->count=0;
    hbufp->t_tot= hbufp->t_min = hbufp->t_max = 0.0;
    task.hash_nkey--;
   }
    if(task.hash_nkey < MAXSIZE_HASH/2) break;
   }
    if(task.hash_nkey < MAXSIZE_HASH/2) done = 1;
    worst_case *= 1.1;
  }
  IPM_TIME_GTOD(time_out);
  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_mpi_util exit %s nkeys=%lld  delta=%f\n",
         task.mpi_rank,tag, task.hash_nkey, time_out-time_in);
   fflush(stdout);
  }
*/

/* set stamp_final */
 IPM_TIME_GTOD(task.stamp_final_mpi);
 time(&task.final_ts_mpi);
 fprintf(stderr, "IPM: %d ERROR internal hash exceeded %d entries in %.3e seconds \n",task.mpi_rank, MAXSIZE_HASHLIMIT,task.stamp_final_mpi-task.stamp_init);
 fprintf(stderr, "IPM: %d ERROR internal hash exceeded, please contact dskinner\@nersc.gov\n", task.mpi_rank);

 fprintf(stderr, "IPM: %d ERROR internal hash exceeded, please contact dskinner\@nersc.gov\n", task.mpi_rank);
 fflush(stderr);
#ifndef IPM_DISABLE_DYNWRAP
 for(i=1;i<=MAXSIZE_CALLS;i++) {
  ipm_mpi_link(i,"pass");
 }
#endif
#endif
  }
  return 0;
 }


