      PROGRAM hello 

      IMPLICIT NONE
      INCLUDE 'mpif.h'
      INTEGER:: rank, size, ierr
      Character:: region(21)


      CALL MPI_INIT( ierr )
      CALL MPI_COMM_RANK( MPI_COMM_WORLD, rank, ierr )
      CALL MPI_COMM_SIZE( MPI_COMM_WORLD, size, ierr )
      call MPI_Pcontrol(0,"enter_region(abcdefghijklmnopqrst)"//char(0));
      call MPI_Pcontrol(0,"get_region()"//char(0),region);
      PRINT *, "in  ", rank , size, region
      call MPI_Pcontrol(0,"exit_region(abcdefghijklmnopqrst)"//char(0));
      call MPI_Pcontrol(0,"get_region()"//char(0),region);
      PRINT *, "out ", rank , size, region
      call MPI_Barrier(MPI_COMM_WORLD, ierr)
c      call ipm_start("region_c//0");
      call MPI_Pcontrol(1,"region_a"//char(0));
      call MPI_Barrier(MPI_COMM_WORLD, ierr)
c      call ipm_stop("region_c//0");
      call MPI_Pcontrol(-1,"region_a"//char(0));


      CALL MPI_FINALIZE(ierr)

      END PROGRAM hello
