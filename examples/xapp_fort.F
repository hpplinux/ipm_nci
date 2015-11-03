      PROGRAM hello 

      IMPLICIT NONE
      INCLUDE 'mpif.h'
      INTEGER:: rank, size, ierr
      INTEGER:: data


      CALL MPI_INIT( ierr )
      CALL MPI_COMM_RANK( MPI_COMM_WORLD, rank, ierr )
      CALL MPI_COMM_SIZE( MPI_COMM_WORLD, size, ierr )
!      PRINT *, rank , size
      data = 1
      call MPI_Barrier(MPI_COMM_WORLD, ierr)
      call MPI_Bcast(data, 1, MPI_INTEGER, 0, MPI_COMM_WORLD, ierr)
      call MPI_Pcontrol(1,"region_c"//char(0));
      call MPI_Barrier(MPI_COMM_WORLD, ierr)
      call MPI_Pcontrol(-1,"region_c"//char(0));


      CALL MPI_FINALIZE(ierr)

      END PROGRAM hello
