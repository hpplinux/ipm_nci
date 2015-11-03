# edit make_config for platform and archiecture specifics

include make_config 

#all: configure libipm.a $(TARGETS)
all: configure libipm_hpm.a $(TARGETS)

configure: make_config
	@if [ ! -f ./include/config.h ]; then echo " "; echo "run configure first"; echo " "; exit 1; fi

tarball:
	make distclean
	(cd ..; find ipm | egrep 'CVS$$' > nocvs)
	(cd ..; tar --exclude-from=nocvs -cvf - ipm | gzip -9c > ipm_$(IPM_VERSION).tgz)
	@rm -f ../nocvs
	@echo " "
	@(echo "tarball is ../ipm_$(IPM_VERSION).tgz")
	@echo " "

lego-svn:
	rm -rf /svn/ipm/
	cd /svn/; svnadmin create ipm
	make clean
	svn import -m "Initial Import" . svn+ssh://lego/svn/ipm/

ipm: 
	cd src; make ipm


libipm.a: 
	@echo 
	@echo building IPM v$(IPM_VERSION) for $(MACH) $(ARCH) with $(COMPILER) compilers
	@echo 
	@echo  CFLAGS = $(CFLAGS)
	@echo 
	cd src; make  "CFLAGS=$(CFLAGS)" libipm.a

# Jie's temporary change for HPM version
libipm_hpm.a: 
	@echo 
	@echo building IPM v$(IPM_VERSION) for $(MACH) $(ARCH) with $(COMPILER) compilers
	@echo 
	@echo  CFLAGS = $(CFLAGS)
	@echo 
	cd src; make  "CFLAGS=$(CFLAGS)" libipm_hpm.a
#end of Jie's temporary change

examples: ipm libipm.a 
	@cd examples; make 

test: test-run-linked

test-run-linked: 
	@ cd examples; make xapp_simple_linked
	$(MPIRUN) ./examples/xapp_simple_linked $(MPIRUN_FLAGS) 

test-timer: config.h
	cd utest; gmake xtimer
	./utest/xtimer -x 1 

test-fortran: libipm.a 
	@ cd examples; make xapp_fort_region
	$(MPIRUN) ./examples/xapp_fort_region -r $(MPIRUN_FLAGS) 

test-nightly: libipm.a 
	@ cd examples; make xapp_simple
	IPM_REPORT=full $(MPIRUN) ./examples/xapp_simple -r $(MPIRUN_FLAGS) 
	@ cd examples; make xapp_fort_region
	IPM_REPORT=full $(MPIRUN) ./examples/xapp_fort_region -r $(MPIRUN_FLAGS)
	@ cd examples; make xapp_c++
	IPM_REPORT=full $(MPIRUN) ./examples/xapp_c++ -r $(MPIRUN_FLAGS) 

tiny: libipm.a 
	$(MPICC) -g -I./include -o ./examples/xapp_hello_static examples/xapp_hello.c src/libtiny.c 
	$(MPICC) -g -I./include/ -c -o src/libtiny.o src/libtiny.c 
	$(MPICC) -g $(SOLDFLAGS) -o lib/libtiny.so src/libtiny.o 
	$(MPICC) -g -o ./examples/xapp_hello examples/xapp_hello.c 
	@ echo " "
	@ echo "mpiexec -n 2 ./examples/xapp_hello "
	@ echo "mpiexec -n 2 ./examples/xapp_hello_static "
	@ echo "LD_PRELOAD=lib/libtiny.so mpiexec -n 2 ./examples/xapp_hello "
	@ echo " "

utest: libipm.a 
	@ cd utest; make all

test-region: libipm.a 
	@ cd examples; make xapp_region
	$(MPIRUN) ./examples/xapp_region $(MPIRUN_FLAGS) -r 254

test-serial: libipm.a 
	@ cd examples; make xapp_serial
	./examples/xapp_serial

test-flops: libipm.a
	@ cd examples; make xflop_mpi
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 10
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 100
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1000
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 10000

test-mem: libipm.a
	@ cd examples; make xflop_mpi
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1 -m 1
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1 -m 10
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1 -m 100
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1 -m 1000
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1 -m 10000
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1 -m 100000
	export IPM_REPORT=full; $(MPIRUN) ./examples/xflop_mpi $(MPIRUN_FLAGS) -n 1 -m 1000000

test-reg: libipm.a ipm
	@ cd examples; make xapp_simple
	$(MPIRUN) ./examples/xapp_simple -r $(MPIRUN_FLAGS) 

test-trc: libipm.a ipm
	@ cd examples; make xapp_simple
	(export IPM_MPI_TRACE="region:region_z:[0-4]+region_a,time:(0.0+1.0),call=MPI_Allreduce+MPI_Bcast,mem:2.0" ; $(MPIRUN) ./examples/xapp_simple -r $(MPIRUN_FLAGS) )

test-ipm: ipm xflop
	rm -f .ipm_FAKECOOKIE
	$(MPIRUN) ./src/ipm ./bin/xflop -n 1024 $(MPIRUN_FLAGS)

test-flop: 
xflop: 
	cd examples; make xflop
	cp examples/xflop bin

test-bench: libipm.a
	@ cd examples; make xbench
	$(MPIRUN) ./examples/xbench $(MPIRUN_FLAGS) >& bench.log

test-benchbench: libipm.a
	@ cd examples; make xbenchbench
	$(MPIRUN) ./examples/xbenchbench $(MPIRUN_FLAGS) >& benchbench.log

check-score: 
	@echo "#include <stdio.h>" 			> test.c
	@echo "#include <stdlib.h>" 			>> test.c
	@echo "#include <unistd.h>" 			>> test.c
	@echo "#include <mpi.h>" 			>> test.c
	@echo "int main(int argc, char *argv[]) {" 	>> test.c
	@echo " int            rank,size;" 		>> test.c
	@echo " MPI_Init(&argc, &argv);" 		>> test.c
	@echo " MPI_Barrier(MPI_COMM_WORLD);" 		>> test.c
	@echo " MPI_Finalize();" 			>> test.c
	@echo " return 0;" 				>> test.c
	@echo "}" 					>> test.c
	@$(MPICC) -o test test.c
	@nm ./test | grep -i mpi_init | awk '{print "MPI_Init in C   :",$$0}'
	@rm test test.c

	@echo "      PROGRAM hello" 				> test.f
	@echo "      IMPLICIT NONE" 				>> test.f
	@echo "      INTEGER ierr" 				>> test.f
	@echo "      INCLUDE 'mpif.h'" 				>> test.f
	@echo "      CALL MPI_INIT( ierr )" 			>> test.f
	@echo "      call MPI_Barrier(MPI_COMM_WORLD, ierr)" 	>> test.f
	@echo "      CALL MPI_FINALIZE(ierr)" 			>> test.f
	@echo "      END PROGRAM hello"				>> test.f
	@$(MPIF77) -o test test.f
	@nm ./test | grep -i mpi_init | awk '{print "MPI_Init in F77 :",$$0}'
	@rm test test.f

	@echo "#include <mpi.h>"				> test.cc
	@echo "#include <iostream>"				>> test.cc
	@echo "using namespace std;"				>> test.cc
	@echo "int main(int argc, char *argv[]) {"		>> test.cc
	@echo " MPI::Init(argc, argv);"				>> test.cc
	@echo " MPI::COMM_WORLD.Barrier();"			>> test.cc
	@echo " MPI::Finalize();"				>> test.cc
	@echo "return 0;"					>> test.cc
	@echo "}"						>> test.cc
	@$(MPICCC) -o test test.cc
	@nm ./test | grep -i mpi_init | awk '{print "MPI_Init in C++ :",$$0}'
	@rm test test.cc



ibm: ibm-lib 
	cd src; make ipm_64
	@echo "--------------------"
	@echo "newgrp usg"
	@echo "rm -f /usr/common/usg/ipm/$(IPM_VERSION)/lib/*"
	@echo "rm -f /usr/common/usg/ipm/$(IPM_VERSION)"
	@echo "mkdir /usr/common/usg/ipm/$(IPM_VERSION)"
	@echo "mkdir /usr/common/usg/ipm/$(IPM_VERSION)/lib"
	@echo "mkdir /usr/common/usg/ipm/$(IPM_VERSION)/bin"
	@echo "mkdir /usr/common/usg/ipm/$(IPM_VERSION)/include"
	@echo "cp lib/*.a /usr/common/usg/ipm/$(IPM_VERSION)/lib/"
	@echo "cp include/ipm.h /usr/common/usg/ipm/$(IPM_VERSION)/include"
	@echo "cp bin/* /usr/common/usg/ipm/$(IPM_VERSION)/bin"
	@echo "fix_usg /usr/common/usg/ipm/$(IPM_VERSION)/"
	@echo "--------------------"

ibm-lib:
	make clean
	rm -rf core.*
	rm -rf aix
	cp -r lib_util/aix ./aix 
	./configure
	cd src; unset OBJECT_MODE; make libipm.a
	cd src; unset OBJECT_MODE; make libipm_64.a
	cp lib/libipm.a aix/libipm_32.a
	cp lib/libipm_64.a aix/libipm_64.a
	rm -f lib/libipm.a
	rm -f lib/libipm.so*
	rm -f bin/ipm
	unset OBJECT_MODE; ./bin/mklib96 aix/libipm_32.a aix/libipm_64.a lib/libipm.a
	cd aix; unset OBJECT_MODE; ./rebuild_libmpi 32
	cd aix; unset OBJECT_MODE; ./rebuild_libmpi 64
	unset OBJECT_MODE; ./bin/mklib96 aix/32/libmpi_r.a aix/64/libmpi_r.a aix/libmpi_r.a
#       ./bin/mklib96 aix/32/libmpi_X.a aix/64/libmpi_X.a aix/libmpi_X.a
#       rm -rf aix/32 aix/64
	mv aix/libmpi_r.a lib
	@echo "rebuilt IBM MPI library is lib/libmpi_r.a"
#       mv aix/libmpi_X.a lib


davinci: davinci-lib
davinci-lib: libipm.a
	rm  -f lib/libipm*.so
	$(MPICC) -shared -g -o lib/libipm_$(IPM_VERSION).so src/libipm.o /usr/lib/libmpi.so $(davinci_PAPI_ROOT)/lib/shared/libpapi.so $(MPILIB)
	cd lib; ln -s libipm_$(IPM_VERSION).so libipm.so
	@echo "--------------------"
	@echo "--------------------"

eagle: eagle-lib
eagle-lib: libipm.a
	rm  -f lib/libipm*.so
	$(MPICC) -shared -g -o lib/libipm_$(IPM_VERSION).so src/libipm.o /usr/lib/libmpi.so $(eagle_PAPI_ROOT)/lib/libpapi.so $(MPILIB)
	cd lib; ln -s libipm_$(IPM_VERSION).so libipm.so
	@echo "--------------------"
	@echo "--------------------"

jacquard: jacquard-lib
jacquard-lib: libipm.a
	rm  -f lib/libipm*.so
	$(CC) -shared -g -o lib/libipm_$(IPM_VERSION).so src/libipm.o $(MPIHOME)/lib/shared/libmpich.so $(MPIHOME)/lib/libfmpich.a -lpathfortran   -L$(jacquard_PAPI_ROOT)/lib -lpapi
	cd lib; ln -s libipm_$(IPM_VERSION).so libipm.so
	@echo ""
	@echo "--------------------"
	@echo "rm -rf /usr/common/usg/ipm/$(IPM_VERSION)"
	@echo "install -d /usr/common/usg/ipm/$(IPM_VERSION)/lib/"
	@echo "install -d /usr/common/usg/ipm/$(IPM_VERSION)/bin/"
	@echo "cp lib/*  /usr/common/usg/ipm/$(IPM_VERSION)/lib/"
	@echo "cp bin/*  /usr/common/usg/ipm/$(IPM_VERSION)/bin/"
	@echo "fix_usg /usr/common/usg/ipm/$(IPM_VERSION)"
	@echo "--> export LD_PRELOAD=/usr/common/usg/ipm/$(IPM_VERSION)/lib/libipm.so"
	@echo "--------------------"
	@echo ""


shared:	CFLAGS +=  -fPIC
shared: libipm.a
	@echo "------shared-lib-build-----------"
	if test -h ./lib/libipm.so ; then rm -f lib/*.so  ; fi
	$(MPICC) $(SOLDFLAGS) -g -o lib/libipm_$(IPM_VERSION).so src/libipm.o $(MPILIB)  $(HPMLIB) $(LDFLAGS)
	cd lib; ln -s libipm_$(IPM_VERSION).so libipm.so

#Jie's temporary change for HPM version
shared_hpm:	CFLAGS +=  -fPIC
shared_hpm: libipm_hpm.a
	@echo "------shared-lib-build-----------"
	if test -h ./lib/libipm_hpm.so ; then rm -f lib/*.so  ; fi
	$(MPICC) $(SOLDFLAGS) -g -o lib/libipm_$(IPM_VERSION)_hpm.so src/libipm.o $(MPILIB)  $(HPMLIB) $(LDFLAGS)
	cd lib; ln -s libipm_$(IPM_VERSION)_hpm.so libipm_hpm.so

#end of Jie's temporary change

install-version: ipm libipm.a
	rm -rf $(INSTALL_ROOT)/$(IPM_VERSION)
	install -d $(INSTALL_ROOT)/$(IPM_VERSION)/lib/
	install -d $(INSTALL_ROOT)/$(IPM_VERSION)/bin/
	install -d $(INSTALL_ROOT)/$(IPM_VERSION)/include/
	cp lib/*.a   $(INSTALL_ROOT)/$(IPM_VERSION)/lib/
	if test -h ./lib/libipm.so ; then cp lib/*.so  $(INSTALL_ROOT)/lib/; fi
	cp bin/ipm  $(INSTALL_ROOT)/$(IPM_VERSION)/bin/
	cp ipm_key  $(INSTALL_ROOT)/$(IPM_VERSION)/include

install: ipm libipm.a 
#	rm -rf $(INSTALL_ROOT)/
	install -d $(INSTALL_ROOT)/lib/
	install -d $(INSTALL_ROOT)/bin/
	install -d $(INSTALL_ROOT)/include/
	cp lib/*.a   $(INSTALL_ROOT)/lib/
	if test -h ./lib/libipm.so ; then cp lib/*.so  $(INSTALL_ROOT)/lib/; fi
#Jie's temporary change for HPM
	if test -h ./lib/libipm_hpm.so ; then cp lib/*.so  $(INSTALL_ROOT)/lib/; fi
#end of Jie's temporary change
	cp bin/ipm  $(INSTALL_ROOT)/bin/
	cp bin/ipm_parse  $(INSTALL_ROOT)/bin/
	cp ipm_key  $(INSTALL_ROOT)/include

install-prefix: install

iowa: libipm_io.so
	cd src; make iowa


nightly: $(NIGHTLY_TASKS)


printconfig :
	@echo "";
	@echo "IPM_VERSION         == ${IPM_VERSION}";
	@echo "OS                  == ${OS}";
	@echo "ARCH                == ${ARCH}";
	@echo "COMPILER            == ${COMPILER}";
	@echo "CC                  == ${CC}";
	@echo "CXX                 == ${CXX}";
	@echo "F77                 == ${F77}";
	@echo "F90                 == ${F90}";
	@echo "MPICC               == ${MPICC}" ;
	@echo "MPIF77              == ${MPIF77}";
	@echo "AR                  == ${AR}";
	@echo "CFLAGS              == ${CFLAGS}";
	@echo "LDFLAGS             == ${LDFLAGS}";
	@echo "SOLDFLAGS           == ${SOLDFLAGS}";
	@echo "WRAPMAKER           == ${WRAPMAKER}";
	@echo "FUNDERSCORE         == ${FUNDERSCORE}";
	@echo "IPM_KEYFILE         == ${IPM_KEYFILE}";
	@echo "MPI_EXEC_DYNAMIC    == ${MPI_EXEC_DYNAMIC}";
	@echo "INSTALL_ROOT        == ${INSTALL_ROOT}";
	@echo "TIMER               == ${STIME}";

clean:
	rm -rf .test
	rm -f ipm-autobuild-*.log
	cd bin; make clean
	cd src; make clean
	rm -rf lib
	rm -f include/config.h
	rm -f include/ipm_calls.h
	rm -f include/ipm_fproto.h
	cd examples; make clean
	cd utest; make clean
	cd db ; make clean 
	rm -f lib/*.a lib/*.so */core* .ipm_FAKECOOKIE core
	rm -rf dskinner.* 
	rm -rf config.log
	rm -rf aix
	rm -rf /var/log/ipm/*
	rm -rf bas src/bas include/bas doc/bas 

distclean: clean
	rm -f include/ipm_fproto.h include/config.h
	@rm -f make_config
	@echo "#this file is generated by ./configure" > make_config
	@echo "IPM_VERSION="`cat VERSION` >> make_config


