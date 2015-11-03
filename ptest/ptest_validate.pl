#!/usr/bin/perl

%vstring = (
    "mpi_hello"              =>  [ "MPI_Comm_size", "MPI_Comm_rank" ],
    "mpi_allgather"          =>  [ "MPI_Allgather" ],
    "mpi_allgatherv"         =>  [ "MPI_Allgatherv" ],
    "mpi_allreduce"          =>  [ "MPI_Allreduce" ],
    "mpi_alltoall"           =>  [ "MPI_Alltoall" ],
    "mpi_alltoallv"          =>  [ "MPI_Alltoallv" ],
    "mpi_bcast"              =>  [ "MPI_Bcast" ],
    "mpi_gather"             =>  [ "MPI_Gather" ],
    "mpi_gatherv"            =>  [ "MPI_Gatherv" ],
    "mpi_reduce"             =>  [ "MPI_Reduce" ],
    "mpi_reduce_scatter"     =>  [ "MPI_Reduce_scatter" ],
    "mpi_scan"               =>  [ "MPI_Scan" ],
    "mpi_scatter"            =>  [ "MPI_Scatter" ],
    "mpi_scatterv"           =>  [ "MPI_Scatterv" ],
    "mpi_sendrecv"           =>  [ "MPI_Sendrecv" ],
    "mpi_sendrecv_replace"   =>  [ "MPI_Sendrecv_replace" ],
    "mpi_SEND-RECV"          =>  [ "MPI_Send", "MPI_Recv" ],
    "mpi_SEND-IRECV"         =>  [ "MPI_Send", "MPI_Irecv" ],
    "mpi_ISEND-RECV"         =>  [ "MPI_Isend", "MPI_Recv" ],
    "mpi_ISEND-IRECV"        =>  [ "MPI_Isend", "MPI_Irecv" ],
    "mpi_RSEND-RECV"         =>  [ "MPI_Rsend", "MPI_Recv" ],
    "mpi_RSEND-IRECV"        =>  [ "MPI_Rsend", "MPI_Irecv" ],
    "mpi_BSEND-RECV"         =>  [ "MPI_Bsend", "MPI_Recv" ],
    "mpi_BSEND-IRECV"        =>  [ "MPI_Bsend", "MPI_Irecv" ],
    "mpi_SSEND-RECV"         =>  [ "MPI_Ssend", "MPI_Recv" ],
    "mpi_SSEND-IRECV"        =>  [ "MPI_Ssend", "MPI_Irecv" ],
    "mpi_IBSEND-RECV"        =>  [ "MPI_Ibsend", "MPI_Recv" ],
    "mpi_IBSEND-IRECV"       =>  [ "MPI_Ibsend", "MPI_Irecv" ],
    "mpi_IRSEND-RECV"        =>  [ "MPI_Irsend", "MPI_Recv" ],
    "mpi_IRSEND-IRECV"       =>  [ "MPI_Irsend", "MPI_Irecv" ],
    "mpi_ISSEND-RECV"        =>  [ "MPI_Issend", "MPI_Recv" ],
    "mpi_ISSEND-IRECV"       =>  [ "MPI_Issend", "MPI_Irecv" ],
    );

sub usage {
    print "Usage: ptest_validate <test>\n";
}

if(@ARGV == 0 || $ARGV[0] =~ /^-h$/ || $ARGV[0] =~ /^-help$/ || $vstring{$ARGV[0]} eq "" )
{
    usage();
    exit(1);
}

%valid = ();
foreach $str (@{$vstring{$ARGV[0]}})
{
    $valid{$str}=0;
}

while(<STDIN>)
{
    foreach $str (@{$vstring{$ARGV[0]}})
    {
	if( $_=~/$str/ ) {
	    $valid{$str}=1;
	}
    }
}


$pass=1;
foreach $str (@{$vstring{$ARGV[0]}})
{
    $pass*=$valid{$str};
}


if( $pass ) {
    print "PASS ($ARGV[0])\n";
}
else {
    print "FAIL ($ARGV[0])\n";
}
