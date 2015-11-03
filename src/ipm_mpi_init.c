int MPI_Init(int *argc, char ***argv) { /* standin */
 int i,rv;
 IPM_TICK_TYPE T1,T2;
 double t1,t2;

 task_argc = argc;
 task_argv = argv;
 if(strcmp(task.mpi_lang, "F")) {
 task.mpi_argc = *argc;
 sprintf(task.mpi_lang, "C");
 }

 if(0) { /* task.flags is unknown at this point */
  printf("IPM: %d C_MPI_Init(%d , ", task.mpi_rank, *argc);
  for(i=0;i<=*argc;i++) { printf("%s ", *(*argv+i)); }
  printf(" ) leave\n");
 }

/* hack for gpfs filesystems @ NERSC */
#ifdef CRAY_GPFS_BUG
 setenv ("MPICH_MPIIO_CB_ALIGN","1",1);
#endif

 IPM_TIME_GET(T1);
 rv=PMPI_Init(task_argc,task_argv); 
 ipm_mpi_init();
 IPM_TIME_GET(T2);

 IPM_TIME_OVERFLOW_CHECK(T1,T2);
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);
 IPM_TIME_RESOLUTION_CHECK(t1,t2);

 if(task.flags & DEBUG && 0) {
    if(strcmp(task.mpi_lang, "F")) {
       printf("IPM: %d C_MPI_Init(%d , ", task.mpi_rank, *argc);
       for(i=0;i<=*argc;i++) { printf("%s ", *(*argv+i)); }
       printf(" ) leave : elapsed=%f\n", t2-t1); 
    }
 }
 return rv;
}

int MPI_Init_thread(int *argc, char ***argv,int requested, int *provided) { 
 int i,rv;
 IPM_TICK_TYPE T1,T2;
 double t1,t2;

 task_argc = argc;
 task_argv = argv;
 if(strcmp(task.mpi_lang, "F")) {
 task.mpi_argc = *argc;
 sprintf(task.mpi_lang, "C");
 }

 if(0) { /* task.flags is unknown at this point */
  printf("IPM: %d C_MPI_Init(%d , ", task.mpi_rank, *argc);
  for(i=0;i<=*argc;i++) { printf("%s ", *(*argv+i)); }
  printf(" ) leave\n");
 }

/* hack for gpfs filesystems @ NERSC */
#ifdef CRAY_GPFS_BUG
 setenv ("MPICH_MPIIO_CB_ALIGN","1",1);
#endif

 IPM_TIME_GET(T1);
 rv=PMPI_Init_thread(task_argc,task_argv,requested,provided);
 ipm_mpi_init();
 IPM_TIME_GET(T2);

 IPM_TIME_OVERFLOW_CHECK(T1,T2);
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);
 IPM_TIME_RESOLUTION_CHECK(t1,t2);

 if (*provided == MPI_THREAD_MULTIPLE && requested == MPI_THREAD_MULTIPLE) {
/* print warning about IPM */
 if (!task.mpi_rank){
  printf ("IPM: Warning IPM is not thread-safe - Results obtained with\n \
MPI_THREAD_MULTIPLE maybe unreliable\n");
 }
 }

 if(task.flags & DEBUG) {
    if(strcmp(task.mpi_lang, "F")) {
       printf("IPM: %d C_MPI_Init_thread(%d , ", task.mpi_rank, *argc);
       for(i=0;i<=*argc;i++) { printf("%s ", *(*argv+i)); }
       printf(" ) leave : elapsed=%f\n", t2-t1);
    }
 }
 return rv;
}
#ifdef WRAP_FORTRAN 

void MPI_INIT_F(int *info) { /* standin */ 
 IPM_TICK_TYPE T1,T2;
 double t1,t2;

 fprintf(stdout,"NOTE: FORTRAN WRAPPER is enabled!!\n");
 fflush(stdout);

 task.mpi_argc = -1;
 task_argc = NULL;
 task_argv = NULL;
 sprintf(task.mpi_lang, "F");

 if(task.flags & DEBUG) { /* task.flags is unknown at this point */
  printf("IPM: %d F_MPI_Init(%d) enter \n", task.mpi_rank, *info);
 }

 IPM_TIME_GET(T1);
/* *info = ipm_mpi_init(); */
 *info = MPI_Init(0,(char ***)0);
 IPM_TIME_GET(T2);

 IPM_TIME_OVERFLOW_CHECK(T1,T2); /* will never overflow actually */
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);
 IPM_TIME_RESOLUTION_CHECK(t1,t2);

 if(task.flags & DEBUG) {
  printf("IPM: %d F_MPI_Init(%d) leave : elapsed=%f\n", task.mpi_rank, *info,t2-t1);
 }

}
void MPI_INIT_THREAD_F(MPI_Fint *required,MPI_Fint *provided,MPI_Fint *info) { /* standin */
 IPM_TICK_TYPE T1,T2;
 double t1,t2;


 task.mpi_argc = -1;
 task_argc = NULL;
 task_argv = NULL;
 sprintf(task.mpi_lang, "F");

 if(task.flags & DEBUG) { /* task.flags is unknown at this point */
  printf("IPM: %d F_MPI_Init_thread(%d) enter \n", task.mpi_rank, *info);
 }

 IPM_TIME_GET(T1);
/* *info = ipm_mpi_init(); */
 *info = MPI_Init_thread(0,(char ***)0,*required,provided);
 IPM_TIME_GET(T2);

 IPM_TIME_OVERFLOW_CHECK(T1,T2); /* will never overflow actually */
 t1 = IPM_TIME_SEC(T1);
 t2 = IPM_TIME_SEC(T2);
 IPM_TIME_RESOLUTION_CHECK(t1,t2);

 if(task.flags & DEBUG) {
  printf("IPM: %d F_MPI_Init_thread(%d) leave : elapsed=%f\n", task.mpi_rank, *info,t2-t1);
 }

}
#endif

