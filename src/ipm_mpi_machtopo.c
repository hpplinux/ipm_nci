/* synchronizing collective */
static int ipm_get_machtopo(void) {
 int i,j, ileft, iright;
 MPI_Status s;
 char hn[MAXSIZE_HOSTNAME],**hlist;
 int *local_roots, *local_sizes, *local_ranks;

 if(task.mpi_size == 1) {
  strncpy(task.mpi_hname_root,task.hostname, MAXSIZE_HOSTNAME);
  job.nhosts = 1;
  task.intra_size = 1;
  task.intra_root = task.mpi_rank;
  task.intra_left = task.mpi_rank;
  task.intra_right = task.mpi_rank;
  task.inter_left = task.mpi_rank;
  task.inter_right = task.mpi_rank;
  return 1;
 }

#ifdef LINUX_BGL
#else 

 if(1) { /* this code section does a send flood to rank 0
            some machines may run out of resources for this
            communication pattern */
 /*
  *
  * HOSTNAME based topoolgy -> appropriate for SMP clusters 
  *
  */

 /* serially fake a gather in memory < MAXSIZE_HOSTNAME * ntasks */
 /* one time cost so I estimate that time is cheaper than memory */

/* send host name to rank zero and get back root rank for that node */
/* first rank to report a new name gets root on that node */

 if(task.mpi_rank) {
  PMPI_Send(task.hostname,MAXSIZE_HOSTNAME,MPI_CHAR,0,task.mpi_rank,MPI_COMM_WORLD);
  PMPI_Recv(&task.intra_root,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
 } else {

  job.nhosts = 1;

  local_roots = (int *)malloc((size_t)(task.mpi_size*sizeof(int)));
  local_sizes = (int *)malloc((size_t)(task.mpi_size*sizeof(int)));
  hlist = (char **)malloc((size_t)(task.mpi_size*sizeof(char *)));
  if(!hlist && !local_sizes) {
   printf("IPM: %d ipm_get_machtopo out of memory hlist\n", task.mpi_rank); 
  }

  for(i=0;i<task.mpi_size;i++) {
   local_roots[i] = -1;
   local_sizes[i] = 0;
  }

  local_roots[0] = 0;
  local_sizes[0] = 1;

  hlist[0] = (char *)malloc((size_t)(MAXSIZE_HOSTNAME*sizeof(char)));
  if(!hlist[0]) {
   printf("IPM: %d ipm_get_machtopo out of memory hlist[0]\n", task.mpi_rank); 
  }
  strncpy(hlist[0],task.hostname, MAXSIZE_HOSTNAME);
  for(i=1;i<task.mpi_size;i++) {
   PMPI_Recv(hn,MAXSIZE_HOSTNAME,MPI_CHAR,i,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
   for(j=0;j<job.nhosts;j++) {
    if(strncmp(hn,hlist[j],MAXSIZE_HOSTNAME)==0) {
    if(task.flags & DEBUG) {
     printf("IPM: %d ipm_get_machtopo got known host %s\n", task.mpi_rank,hn); 
    }
     break;
    }
    if(j == job.nhosts-1) {
     hlist[j+1] = (char *)malloc((size_t)(MAXSIZE_HOSTNAME*sizeof(char)));
     if(task.flags & DEBUG) {
     printf("IPM: %d ipm_get_machtopo got new host %s\n", task.mpi_rank,hn); 
     }
     if(!hlist[j+1]) {
      printf("IPM: %d ipm_get_machtopo out of memory hlist[%d]\n", task.mpi_rank,i); 
     }
     strncpy(hlist[j+1],hn, MAXSIZE_HOSTNAME);
     local_roots[j+1] = i;
     job.nhosts++;
    }
   }
   local_sizes[j] ++;
   PMPI_Send(&local_roots[j],1,MPI_INT,i,0,MPI_COMM_WORLD);
  }
 }

 /* now know local_roots on all local_root ranks */

 if(task.mpi_rank == task.intra_root) {
  if(task.mpi_rank) {
   PMPI_Recv(&task.intra_size,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
   PMPI_Recv(&task.inter_left,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
   PMPI_Recv(&task.inter_right,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
 } else {
   for(j=1;j<job.nhosts;j++) {
    ileft = local_roots[j-1]; 
    iright = ((j == job.nhosts -1)?(local_roots[0]):(local_roots[j+1]));
    PMPI_Send(&local_sizes[j],1,MPI_INT,local_roots[j],0,MPI_COMM_WORLD);
    PMPI_Send(&ileft,1,MPI_INT,local_roots[j],0,MPI_COMM_WORLD);
    PMPI_Send(&iright,1,MPI_INT,local_roots[j],0,MPI_COMM_WORLD);
   }

   task.intra_size = local_sizes[0];
   task.inter_left = local_roots[job.nhosts-1];
   task.inter_right = local_roots[1];
  }
 }

 /* now all local roots know their size and inter_left and inter_right */

 if(task.mpi_rank == task.intra_root) {
  local_ranks = (int *) malloc((size_t)(task.intra_size*sizeof(int)));
  local_ranks[0] = task.mpi_rank;
  for(j=1;j<task.intra_size;j++) {
   PMPI_Recv(&(local_ranks[j]),1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
  }
 } else {
   PMPI_Send(&task.mpi_rank,1,MPI_INT,task.intra_root,0,MPI_COMM_WORLD);
 }

 /* now all local roots know all the local ranks  */

/*
 if(task.mpi_rank == task.intra_root) {
  printf("ranks %d :: ", task.mpi_rank);
  for(j=0;j<task.intra_size;j++) {
   printf("%d ", local_ranks[j]);
  }
  printf("\n");
 }
*/

 if(task.mpi_rank == task.intra_root) {
  local_ranks[0] = task.mpi_rank;
  for(j=1;j<task.intra_size;j++) {
   ileft = local_ranks[j-1]; 
   iright = ((j == task.intra_size -1)?(local_ranks[0]):(local_ranks[j+1]));
   PMPI_Send(&task.inter_left,1,MPI_INT,local_ranks[j],1,MPI_COMM_WORLD);
   PMPI_Send(&task.inter_right,1,MPI_INT,local_ranks[j],2,MPI_COMM_WORLD);
   PMPI_Send(&task.intra_size,1,MPI_INT,local_ranks[j],3,MPI_COMM_WORLD);
   PMPI_Send(&ileft,1,MPI_INT,local_ranks[j],4,MPI_COMM_WORLD);
   PMPI_Send(&iright,1,MPI_INT,local_ranks[j],5,MPI_COMM_WORLD);
  }
   task.intra_left = local_ranks[task.intra_size-1];
   task.intra_right = local_ranks[1];
 } else {
   PMPI_Recv(&task.inter_left,1,MPI_INT,task.intra_root,1,MPI_COMM_WORLD,&s);
   PMPI_Recv(&task.inter_right,1,MPI_INT,task.intra_root,2,MPI_COMM_WORLD,&s);
   PMPI_Recv(&task.intra_size,1,MPI_INT,task.intra_root,3,MPI_COMM_WORLD,&s);
   PMPI_Recv(&task.intra_left,1,MPI_INT,task.intra_root,4,MPI_COMM_WORLD,&s);
   PMPI_Recv(&task.intra_right,1,MPI_INT,task.intra_root,5,MPI_COMM_WORLD,&s);
 }

 strncpy(task.mpi_hname_root, task.hostname, MAXSIZE_HOSTNAME); /* 0 */
 PMPI_Bcast(&task.mpi_hname_root,MAXSIZE_HOSTNAME,MPI_CHAR,0,MPI_COMM_WORLD);
 PMPI_Bcast(&job.nhosts,1,MPI_INT,0,MPI_COMM_WORLD);
 
 if(task.flags & DEBUG) {
  printf("IPM: %d TOPO inter %d %d %d %d intra %d %d %d %d\n",
	 task.mpi_rank,
	 job.nhosts, 0,
	 task.inter_left, 
	 task.inter_right, 
	 task.intra_size, 
	 task.intra_root, 
	 task.intra_left, 
	 task.intra_right);
 }

 if(!task.mpi_rank) { /* free memory used on task zero */
  for(j=0;j<job.nhosts-1;j++) {
   free((char *) hlist[j]);
  }
  free((char *) hlist);
  free((char *) local_roots);
  free((char *) local_sizes);
 }

 if(task.mpi_rank == task.intra_root) {
   free((char *) local_ranks);
  }

 /* I feel sick. */
 return 1;
 }
#endif


 return 1;
}
