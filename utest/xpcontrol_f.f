      PROGRAM pcontrol

100   format(a,f10.4)

       IMPLICIT NONE
       INTEGER i
       character bingo(100)


       PRINT *, "region:sfup"
       call mpi_pcontrol(0,"region:sfup"//char(0),bingo)

      END PROGRAM

