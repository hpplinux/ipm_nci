
 if((task.flags & DEBUG) && !task.mpi_rank) {

  printf("IPM: %d key_test IPM_MPI_KEY(key,255,32,65535,%d)\n",task.mpi_rank, INT_MAX-1);
  IPM_MPI_KEY(key,255,32,65535,(INT_MAX-1));
  printf("IPM: %d key_test k=%lld\n",task.mpi_rank,key);
  printf("IPM: %d key_test KEY_REGION(k)=%d\n",task.mpi_rank, KEY_REGION(key));  printf("IPM: %d key_test KEY_CALL(k)=%d\n",task.mpi_rank, KEY_CALL(key));
  printf("IPM: %d key_test KEY_RANK(k)=%d\n",task.mpi_rank, KEY_RANK(key));
  printf("IPM: %d key_test KEY_BYTE(k)=%d\n",task.mpi_rank, KEY_BYTE(key));
  printf("IPM: %d key_test k        ",task.mpi_rank); IPM_SHOWBITS(key); printf("\n");
  printf("IPM: %d key_test k&region ",task.mpi_rank); IPM_SHOWBITS(key&KEY_MASK_REGION); printf("\n");
  printf("IPM: %d key_test k&call   ",task.mpi_rank); IPM_SHOWBITS(key&KEY_MASK_CALL); printf("\n");
  printf("IPM: %d key_test k&rank   ",task.mpi_rank); IPM_SHOWBITS(key&KEY_MASK_RANK); printf("\n");
  printf("IPM: %d key_test k&byte   ",task.mpi_rank); IPM_SHOWBITS(key&KEY_MASK_BYTE); printf("\n");
  fflush(stdout);
 }


