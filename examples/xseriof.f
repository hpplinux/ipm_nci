      subroutine fort_write_da(size,rank,ndata,data)
       implicit none
       integer*4        size,rank,munit
       integer*8	ndata,rl
       real*8		data(ndata)


       rl = 8*ndata
       munit = 11+rank
       open(unit=munit,form='unformatted',access='direct',recl=rl)
       write(unit=munit, rec=1) data

C       open(unit=11,file=ff,form='unformatted')
C       write(unit=11) data
       close(munit)
       return
      end

      subroutine fort_read_da(size,rank,ndata,data)
       implicit none
       integer*4        size,rank,munit
       integer*8	ndata,rl
       real*8		data(ndata)

       rl = 8*ndata
       munit = 11+rank
       open(unit=munit,form='unformatted',access='direct',recl=rl)
       read(unit=munit, rec=1) data
       close(munit)
       return
      end

C      subroutine readplds
C      parameter (width=720, height = 340)
C      character*1 lrec(width)
C      integer col, row, rlen, snow(width,height), x, y
C     record length is specified in words not bytes
C      rlen = width / 4
C     open file
C      open(unit=1, file='data.new', form='unformatted',
C     1     access='direct', recl=rlen)
C
C     read header and display
C      read(1) lrec
C      write(*,*) lrec
C      do row = 1, height
C        read(1) lrec
C        do col = 1, width
C          value = ichar(lrec(col))
C          if (value .lt. 0) value = value + 256
C          snow(col, row) = value
C        end do
C      end do
C      close(1)
C
C     query grid
C 100  continue
C      print *, 'enter x y from rdpix'
C      read *, x, y
C      col = x + 1
C      row = height - y
C      print '(a,i4,a,i4,a,i4)', 
C     1     'col=', col, ', row=', row, ', value=', snow(col,row)
C      goto 100
C
C      end
