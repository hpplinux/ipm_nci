# edit make_config for platform and archiecture specifics

include ../make_config

WRAPMAKER_ARGS= $(IPM_LIB_OPTS)  $(FUNDERSCORE)

LIBDEPS_MPI= ipm_mpi_init.c ipm_mpi_finalize.c ipm_mpi_pcontrol.c \
             ipm_mpi_machtopo.c ipm_mpi_report.c ipm_mpi_sync.c \
             ipm_mpi_log.c ipm_trace.c
LIBDEPS_API= ipm_api.c ipm_bit_test.c ipm_env.c ipm_hash_test.c \
             ipm_jobinfo.c ipm_size_test.c 
LIBDEPS_UTL= ipm_execinfo.c  ipm_jobinfo.c ipm_memusage.c ipm_switchinfo.c\
             ipm_util.c ipm_report_internal.c 
LIBDEPS_HPM= ipm_hpm.c ipm_hpm_test.c
LIBDEPS=$(LIBDEPS_MPI) $(LIBDEPS_API) $(LIBDEPS_UTL) $(LIBDEPS_HPM) $(WRAPMAKER) 
#FOREIGN_OBJECTS=dfloatuk.o

all: ipm ipm libipm.a 

test-config:
	@echo $CFLAGS="$(CFLAGS)"
	@echo HPMLIB="$(HPMLIB)"
	@echo MPILIB="$(MPILIB)"

ipm: ipm.c ipm_report_internal.c ipm_hpm_test.c
	$(WRAPMAKER) $(WRAPMAKER_ARGS) ../ipm_key 
	$(CC) -g $(HPM_CLFAGS) $(CFLAGS) -I../include -o ../bin/ipm ipm.c   $(LFLAGS) $(HPM_LDFLAGS)

ipm_64: ipm.c ipm_report_internal.c ipm_hpm_test.c
	$(WRAPMAKER) $(WRAPMAKER_ARGS) ../ipm_key  
	$(CC) -g -q64 $(HPM_CFLAGS) $(CFLAGS) -D$(ARCH) -I../include -o ipm ipm.c  $(LFLAGS) $(HPM_LDFLAGS)
	mv ipm ../bin

libipm.a: $(WRAPMAKER)  ../ipm_key ../Makefile $(LIBDEPS)
	$(WRAPMAKER)  $(WRAPMAKER_ARGS) ../ipm_key  
	rm -f libipm.a
	$(MPICC) $(CFLAGS) $(HPM_CFLAGS) -g -c -I../include libipm.c
	$(AR) crv libipm.a libipm.o $(FOREIGN_OBJECTS)
	rm -f ../lib/libipm.a
	if test ! -d ../lib; then mkdir ../lib ;fi
	mv libipm.a ../lib

#Jie's temporary change for HPM version
libipm_hpm.a: $(WRAPMAKER)  ../ipm_key ../Makefile $(LIBDEPS)
	$(WRAPMAKER)  $(WRAPMAKER_ARGS) ../ipm_key  
	rm -f libipm_hpm.a
	$(MPICC) $(CFLAGS) $(HPM_CFLAGS) -g -c -I../include libipm.c
	$(AR) crv libipm_hpm.a libipm.o $(FOREIGN_OBJECTS)
	rm -f ../lib/libipm_hpm.a
	if test ! -d ../lib; then mkdir ../lib ;fi
	mv libipm_hpm.a ../lib
#end of Jie's temporary change
libipm_64.a: $(WRAPMAKER)  ../ipm_key ../Makefile  $(LIBDEPS)
	$(WRAPMAKER) $(WRAPMAKER_ARGS) ../ipm_key  
	export OBJECT_MODE=64; rm -f libipm_64.a
	export OBJECT_MODE=64; $(MPICC) -c  -I../include  $(CFLAGS) $(HPM_CFLAGS) libipm.c -g -o libipm_64.o
	export OBJECT_MODE=64; ar crv libipm_64.a libipm_64.o
	mv libipm_64.a ../lib

libipm_io.so: $(WRAPMAKER)  ../ipm_key ../Makefile  $(LIBDEPS)
	$(WRAPMAKER) $(WRAPMAKER_ARGS) ../ipm_key  
	$(CC) -D$(ARCH) $(CFLAGS) $(HPM_CFLAGS) -I../include -shared -g -o libipm_io.so libipm_io.c -ldl
	$(MPICC) -D$(ARCH) -DMPI $(CFLAGS) $(HPM_CFLAGS) -I../include -shared -g -o libipm_io_mpi.so libipm_io.c -ldl


libipm_pthreads.so: $(WRAPMAKER)  ../ipm_key ../Makefile  $(LIBDEPS) ipm_pthreads.c
	$(WRAPMAKER) $(WRAPMAKER_ARGS) ../ipm_key  
	$(MPICC) $(CFLAGS) -DIPM_MONITOR_PTHREADS=1 $(HPM_CFLAGS) -I../include -shared -g -o libipm_pthreads.so ipm_pthreads.c libipm.c -ldl


iowa: libipm_io.so
	$(CC) -D$(ARCH) $(CFLAGS) $(HPM_CFLAGS) -I../include -g -o iowa iowa.c -lpthread
	(cd ../examples; make xcmd_hello)
	LD_PRELOAD=./libipm_io.so ../examples/xcmd_hello
	rm iowa

bwr: bwr.c
	$(CC) -g -o bwr bwr.c

clean:
	rm -rf *.o *.a *.so ipm core* iowa bwr libipm.c libtiny.c libipm_io.c buildinfo.c


