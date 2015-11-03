all:

	rm -f ipm.exp
	rm -rf 32 64
	rm -f *.o *.a 

	export USE_PMAPI=-lpmapi; export OBJECT_MODE=32; ./build_profiled_lib
	export USE_PMAPI=-lpmapi; export OBJECT_MODE=64; ./build_profiled_lib_64

	../mklib96 32/libmpi_r.a 64/libmpi_r.a libmpi_r.a
	../mklib96 32/libmpi_X.a 64/libmpi_X.a libmpi_X.a
	../mklib96 32/libvtd_X.a 64/libvtd_X.a libvtd_X.a

	echo "now do --->  cp *.a /usr/common/usg/ipm/0.7/lib/"
	rm *.exp 


