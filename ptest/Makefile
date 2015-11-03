
include ../make_config

TEST_CFLAGS = -I../include $(CFLAGS) -DIPM
TEST_LFLAGS = -l ipm $(LDFLAGS) 

TESTS = xmpi_hello \
	xmpi_allgather xmpi_allgatherv xmpi_allreduce \
	xmpi_alltoall xmpi_alltoallv xmpi_bcast \
	xmpi_gather xmpi_gatherv \
	xmpi_reduce xmpi_reduce_scatter \
	xmpi_scan xmpi_scatter xmpi_scatterv \
	xmpi_send-recv xmpi_sendrecv xmpi_sendrecv_replace 

all: $(TESTS)

xmpi_hello : xmpi_hello.o
	$(MPIF77) -o $@ $+ $(TEST_LFLAGS) 

xmpi_% : xmpi_%.o
	$(MPIF77) -o $@ $+ $(TEST_LFLAGS) 

%.o : %.c 
	$(MPICC) -c $(TEST_CFLAGS) -o $@ $+ 


clean:
	rm -rf *.o *~ *.out *.err $(TESTS)





