# edit make_config for platform and archiecture specifics

include ../make_config

DEPS=../lib/libipm.a

TEST_CFLAGS=-I../include $(CFLAGS) -DIPM
TEST_LFLAGS=-L../lib $(LIPM) $(LFLAGS) $(MPILIB)


all: xapp xapp_simple xapp_simple_linked xapp_pcontrol xapp_fort_region xapp_fort xapp_c++ xapp_wait xbench xapp_comm xprobe_mpi xapp_hello xserio xcmd_hello

# xpapi avail flop xapp_comm

xapp_simple: xapp_simple.c $(DEPS)
	echo $(LFLAGS)
	$(MPICC) xapp_simple.c -o xapp_simple $(MPILIB) $(HPM_CFLAGS) $(HPM_LDFLAGS)

xapp_simple_linked: xapp_simple.c $(DEPS)
	echo $(LFLAGS)
	$(MPICC) xapp_simple.c -o xapp_simple_linked $(MPILIB) $(HPM_CFLAGS) $(HPM_LDFLAGS) $(TEST_LFLAGS) -lipm

xapp_wait: xapp_wait.c $(DEPS)
	echo $(LFLAGS)
	$(MPICC) xapp_wait.c -o xapp_wait $(MPILIB)
	$(MPICC) $(TEST_CFLAGS) -g xapp_wait.c -o xapp_wait_ipm $(TEST_LFLAGS) -lipm

xapp_hello: xapp_hello.c 
	$(MPICC) xapp_hello.c -o xapp_hello

xcmd_hello: xcmd_hello.c 
	$(CC) xcmd_hello.c -o xcmd_hello

xprobe_mpi: xprobe_mpi.c $(DEPS)
	$(MPICC) -g xprobe_mpi.c -o xprobe_mpi 
	$(MPICC) $(TEST_CFLAGS) -g xprobe_mpi.c -o xprobe_mpi_ipm

xapp_comm: xapp_comm.c $(DEPS)
	$(MPICC) $(TEST_CFLAGS) -g xapp_comm.c -o xapp_comm $(TEST_LFLAGS)

xapp_omp_mpi: xapp_omp_mpi.c $(DEPS)
	$(MPICC) $(TEST_CFLAGS) -g xapp_omp_mpi.c -o xapp_omp_mpi $(TEST_LFLAGS)

xapp_omp: xapp_omp.c $(DEPS)
	$(MPICC) $(TEST_CFLAGS) -g xapp_omp.c -o xapp_omp $(TEST_LFLAGS)

xapp_pcontrol: xapp_pcontrol.c $(DEPS)
	$(MPICC) $(TEST_FLAGS) -g xapp_pcontrol.c -o xapp_pcontrol $(TEST_LFLAGS)

xapp_region: xapp_region.c $(DEPS)
	$(MPICC) $(TEST_FLAGS) -g xapp_region.c -o xapp_region $(TEST_LFLAGS)

xapp_fort_region: xapp_fort_region.F $(DEPS)
	$(MPIF77) $(TEST_FLAGS) -g xapp_fort_region.F -o xapp_fort_region $(TEST_LFLAGS)

xapp_fort: xapp_fort.F $(DEPS)
	$(MPIF77) $(TEST_FLAGS) -g xapp_fort.F -o xapp_fort $(TEST_LFLAGS)

xapp_c++: xapp_c++.cc $(DEPS)
	$(MPICCC) $(TEST_FLAGS) -g xapp_c++.cc -o xapp_c++ $(LIPM) $(TEST_LFLAGS)

xapp: xapp.c $(DEPS)
	$(MPICC) $(TEST_FLAGS) -g xapp.c -o xapp $(LIPM) $(TEST_LFLAGS)

xbench: xbench.c $(DEPS)
	$(MPICC)  $(CFLAGS) -D$(ARCH) -I../include $(TEST_FLAGS) $(LFLAGS) -g xbench.c -o xbench -L../src $(LIPM) $(HPM_LFLAGS)

xbenchbench: xbench.c $(DEPS)
	$(MPICC)  $(CFLAGS) -D$(ARCH) -I../include $(TEST_FLAGS) $(LFLAGS) -g xbench.c -o xbenchbench -L../src 

xpapi: xpapi.c $(DEPS)
	$(CC) -D$(ARCH) $(CFLAGS) -o xpapi xpapi.c $(LFLAGS) $(EXTRA_LFLAGS)

xserio: xserio.c xseriof.o
	$(CC) -D$(ARCH) $(CFLAGS) -c xserio.c $(LFLAGS) $(EXTRA_LFLAGS)
	$(F77) -D$(ARCH) $(FFLAGS) -c xseriof.f $(LFLAGS) $(EXTRA_LFLAGS)
	$(CC) -D$(ARCH) $(CFLAGS) -o xserio xserio.o xseriof.o $(LFLAGS) $(LFLAGS_CLF) -lgfortran $(EXTRA_LFLAGS)

clean:
	rm -rf *.o xapp xapp_simple xapp_fort xapp_c++ xbench xfapp_simple xapp_region xapp_pcontrol xapp_fort_region xapp_simple_lite xapp_simple_linked xapp_comm xbenchbench xapp_signal papi_mpi xprobe_mpi xapp_hello_static xapp_hello xprobe_mpi_ipm xserio xcmd_hello hello xapp_wait_ipm xapp_wait xapp_fort_region_ipm




