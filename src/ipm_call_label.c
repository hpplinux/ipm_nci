/*
 * #####
 * # IPM MPI labels and ids
 * #####
 *
 * DO NOT EDIT: automatically generated at buildtime by make_wrappers
 */

#define MPI_INIT_ID               1  
#define MPI_FINALIZE_ID           2  
#define MPI_COMM_SIZE_ID          3  
#define MPI_COMM_RANK_ID          4  
#define MPI_SEND_ID               5  
#define MPI_SSEND_ID              6  
#define MPI_RSEND_ID              7  
#define MPI_BSEND_ID              8  
#define MPI_ISEND_ID              9  
#define MPI_ISSEND_ID             10 
#define MPI_IRSEND_ID             11 
#define MPI_IBSEND_ID             12 
#define MPI_SEND_INIT_ID          13 
#define MPI_SSEND_INIT_ID         14 
#define MPI_RSEND_INIT_ID         15 
#define MPI_BSEND_INIT_ID         16 
#define MPI_RECV_INIT_ID          17 
#define MPI_RECV_ID               18 
#define MPI_IRECV_ID              19 
#define MPI_SENDRECV_ID           20 
#define MPI_SENDRECV_REPLACE_ID   21 
#define MPI_BUFFER_ATTACH_ID      22 
#define MPI_BUFFER_DETACH_ID      23 
#define MPI_PROBE_ID              24 
#define MPI_IPROBE_ID             25 
#define MPI_TEST_ID               26 
#define MPI_TESTANY_ID            27 
#define MPI_TESTALL_ID            28 
#define MPI_TESTSOME_ID           29 
#define MPI_WAIT_ID               30 
#define MPI_WAITANY_ID            31 
#define MPI_WAITALL_ID            32 
#define MPI_WAITSOME_ID           33 
#define MPI_START_ID              34 
#define MPI_STARTALL_ID           35 
#define MPI_BCAST_ID              36 
#define MPI_BARRIER_ID            37 
#define MPI_GATHER_ID             38 
#define MPI_GATHERV_ID            39 
#define MPI_SCATTER_ID            40 
#define MPI_SCATTERV_ID           41 
#define MPI_SCAN_ID               42 
#define MPI_ALLGATHER_ID          43 
#define MPI_ALLGATHERV_ID         44 
#define MPI_REDUCE_ID             45 
#define MPI_ALLREDUCE_ID          46 
#define MPI_REDUCE_SCATTER_ID     47 
#define MPI_ALLTOALL_ID           48 
#define MPI_ALLTOALLV_ID          49 
int ipm_label_write(ipm_taskdata *taskp) {
 sprintf(taskp->call_label[1],"MPI_Init");
 sprintf(taskp->call_label[2],"MPI_Finalize");
 sprintf(taskp->call_label[3],"MPI_Comm_rank");
 sprintf(taskp->call_label[4],"MPI_Comm_size");
 sprintf(taskp->call_label[5],"MPI_Send");
 sprintf(taskp->call_label[6],"MPI_Ssend");
 sprintf(taskp->call_label[7],"MPI_Rsend");
 sprintf(taskp->call_label[8],"MPI_Bsend");
 sprintf(taskp->call_label[9],"MPI_Isend");
 sprintf(taskp->call_label[10],"MPI_Issend");
 sprintf(taskp->call_label[11],"MPI_Irsend");
 sprintf(taskp->call_label[12],"MPI_Ibsend");
 sprintf(taskp->call_label[13],"MPI_Send_init");
 sprintf(taskp->call_label[14],"MPI_Ssend_init");
 sprintf(taskp->call_label[15],"MPI_Rsend_init");
 sprintf(taskp->call_label[16],"MPI_Bsend_init");
 sprintf(taskp->call_label[17],"MPI_Recv_init");
 sprintf(taskp->call_label[18],"MPI_Recv");
 sprintf(taskp->call_label[19],"MPI_Irecv");
 sprintf(taskp->call_label[20],"MPI_Sendrecv");
 sprintf(taskp->call_label[21],"MPI_Sendrecv_replace");
 sprintf(taskp->call_label[22],"MPI_Buffer_attach");
 sprintf(taskp->call_label[23],"MPI_Buffer_detach");
 sprintf(taskp->call_label[24],"MPI_Probe");
 sprintf(taskp->call_label[25],"MPI_Iprobe");
 sprintf(taskp->call_label[26],"MPI_Test");
 sprintf(taskp->call_label[27],"MPI_Testany");
 sprintf(taskp->call_label[28],"MPI_Testall");
 sprintf(taskp->call_label[29],"MPI_Testsome");
 sprintf(taskp->call_label[30],"MPI_Wait");
 sprintf(taskp->call_label[31],"MPI_Waitany");
 sprintf(taskp->call_label[32],"MPI_Waitall");
 sprintf(taskp->call_label[33],"MPI_Waitsome");
 sprintf(taskp->call_label[34],"MPI_Start");
 sprintf(taskp->call_label[35],"MPI_Startall");
 sprintf(taskp->call_label[36],"MPI_Bcast");
 sprintf(taskp->call_label[37],"MPI_Barrier");
 sprintf(taskp->call_label[38],"MPI_Gather");
 sprintf(taskp->call_label[39],"MPI_Gatherv");
 sprintf(taskp->call_label[40],"MPI_Scatter");
 sprintf(taskp->call_label[41],"MPI_Scatterv");
 sprintf(taskp->call_label[42],"MPI_Scan");
 sprintf(taskp->call_label[43],"MPI_Allgather");
 sprintf(taskp->call_label[44],"MPI_Allgatherv");
 sprintf(taskp->call_label[45],"MPI_Reduce");
 sprintf(taskp->call_label[46],"MPI_Allreduce");
 sprintf(taskp->call_label[47],"MPI_Reduce_scatter");
 sprintf(taskp->call_label[48],"MPI_Alltoall");
 sprintf(taskp->call_label[49],"MPI_Alltoallv");
 return 0;
}

