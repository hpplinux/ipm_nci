#!/bin/bash

if [ "$MPIRUN" == "" ]
then
    MPIRUN="mpirun -np 4"
fi


function runtest {
    IPM_TEST=$1     # name of the test
    EXECUTABLE=$2   # binary to invoke

    echo "Testing $IPM_TEST..."
    export IPM_LOGDIR="./${IPM_TEST}/"
    rm -rf $IPM_LOGDIR
    mkdir -p $IPM_LOGDIR
    echo "Invoking '${MPIRUN} ${EXECUTABLE} > $IPM_TEST.out 2> $IPM_TEST.err'"
    $MPIRUN ${EXECUTABLE} > $IPM_TEST.out 2> $IPM_TEST.err

    if [ "$?" = 0 ]; then
	echo -n "Valididating result:  "
    	find $IPM_LOGDIR  -type f -exec sh -c "ipm_parse -mpi {} | ./ptest_validate.pl $IPM_TEST" \;
    else
	echo "Valididating result:  FAIL ($IPM_TEST)"
    fi

    rm -rf $IPM_LOGDIR
    echo ""    
}

for test in mpi_hello mpi_allgather mpi_allgatherv mpi_allreduce mpi_alltoall mpi_alltoallv mpi_bcast mpi_gather mpi_gatherv mpi_reduce mpi_reduce_scatter mpi_scan mpi_scatter mpi_scatterv mpi_sendrecv mpi_sendrecv_replace
do
    runtest $test ./x$test
done    

# P2P send-recv combinations
for sendop  in  SEND ISEND BSEND RSEND SSEND IBSEND IRSEND ISSEND
do
    for recvop in RECV IRECV
    do
	runtest mpi_$sendop-$recvop "./xmpi_send-recv $sendop $recvop"
    done
done




