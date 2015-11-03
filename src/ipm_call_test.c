
 if((task.flags & DEBUG) && !task.mpi_rank) {


 /* diagnostics for information kept in the call mask :: bits */

    printf("IPM: %3d call_mask %-20.20s        bits=",task.mpi_rank,"CALL_MASK_STATE");
    IPM_SHOWBITSR(CALL_MASK_STATE);
    printf("\n");

    printf("IPM: %3d call_mask %-20.20s        bits=",task.mpi_rank,"CALL_MASK_OPT");
    IPM_SHOWBITSR(CALL_MASK_OPT);
    printf("\n");

    printf("IPM: %3d call_mask %-20.20s        bits=",task.mpi_rank,"CALL_MASK_BPREC");
    IPM_SHOWBITSR(CALL_MASK_BPREC);
    printf("\n");

    printf("IPM: %3d call_mask %-20.20s        bits=",task.mpi_rank,"CALL_OPT_TRACE");
    IPM_SHOWBITSR(CALL_OPT_TRACE);
    printf("\n");

    printf("IPM: %3d call_mask %-20.20s        bits=",task.mpi_rank,"CALL_OPT_PASS");
    IPM_SHOWBITSR(CALL_OPT_PASS);
    printf("\n");

    printf("IPM: %3d call_mask %-20.20s        bits=",task.mpi_rank,"CALL_OPT_NOOP");
    IPM_SHOWBITSR(CALL_OPT_NOOP);
    printf("\n");

 /* diagnostics for information kept in the call mask :: bits */

  for(j=0;j<=MAXSIZE_CALLS;j++) {
   if(!task.mpi_rank) {
    printf("IPM: %3d call_mask %-20.20s id=%3d bits=",task.mpi_rank,task.call_label[j],j);
    IPM_SHOWBITSR(task.call_mask[j]);
    printf("\n");
   }
  }

 /* diagnostics for information kept in the call mask :: flags */

/* xt3 and sp crash with this , there is an error below */

/*
  for(j=1;j<=MAXSIZE_CALLS;j++) {
   if(!task.mpi_rank) {
    printf("IPM: %3d call_mask %-20.20s id=%3d flags=",
	task.mpi_rank,task.call_label[j],j);
    IPM_SHOWCALL(task.call_mask[j]);
    printf("\n");
   }
  }
*/

 }

