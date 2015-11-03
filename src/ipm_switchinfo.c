#ifdef OS_AIX
/* switch reading mpc routines */
#include <pm_util.h>
#endif

static int ipm_get_switchname(char *sname) {
 char *cptr1, *cptr2;

#if defined(PROC_ETH1) || defined(PROC_ETH0)
 sprintf(sname, "ethernet");
#endif

#if defined(PROC_IB1) || defined(PROC_IB0)
 sprintf(sname, "infiniband");
#endif

#ifdef PE_COLONY
 cptr1 = getenv("MP_EUIDEVICE");
 cptr2 = getenv("MP_EUILIB");
 if(cptr1 != NULL  && cptr2 != NULL) {
  sprintf(sname, "colony_%s_%s", cptr1, cptr2);
 } else {
  sprintf(sname, "colony_?_?");
 }
#endif
#ifdef PE_FEDERATION
 cptr1 = getenv("MP_EUIDEVICE");
 cptr2 = getenv("MP_EUILIB");
 if(cptr1 != NULL  && cptr2 != NULL) {
  sprintf(sname, "federation_%s_%s", cptr1, cptr2);
 } else {
  sprintf(sname, "federation_?_?");
 }
#endif

 return 0;
}

static int ipm_get_switchbytes(double *bytes_tx, double *bytes_rx) {
 static int 	first_call = 1;
 int rv;
 FILE                   *fh=NULL, *fdh[2];
 int                     fdd[2];
 char                   txt_line[80];


  if(task.flags & DEBUG) {
   printf("IPM: %d get_switchbytes enter \n", task.mpi_rank);
  }

*bytes_rx =0;
*bytes_tx =0;


#ifdef OS_AIX
#ifndef AIX51
#if defined(PE_COLONY) || defined(PE_FEDERATION) 


  bw_stat_t bw_in;
  int rc;
  rc = mpc_bandwidth(NULL, MP_BW_MPI, &bw_in);
  *bytes_tx=(double)bw_in.switch_sent;
  *bytes_rx=(double)bw_in.switch_recv;

/*
LAPI: Tot_dup_pkt_cnt=0
LAPI: Tot_retrans_pkt_cnt=0
LAPI: Tot_gho_pkt_cnt=0
LAPI: Tot_pkt_sent_cnt=0
LAPI: Tot_pkt_recv_cnt=0
LAPI: Tot_data_sent=0
LAPI: Tot_data_recv=0
*/
#endif
#endif
#endif

 fh = NULL;
#if defined  (OS_LINUX)
#ifndef LINUX_XT3
#if defined (PROC_ETH0)
 fh = fopen("/sys/class/net/eth0/statistics/tx_bytes", "r");
#elif defined (PROC_ETH1)
 fh = fopen("/sys/class/net/eth1/statistics/tx_bytes", "r");
#elif defined (PROC_IB0)
 fh = fopen("/sys/class/net/ib0/statistics/tx_bytes", "r");
#elif defined (PROC_IB1)
 fh = fopen("/sys/class/net/ib1/statistics/tx_bytes", "r");
#endif
 if(fh) {
  rv = fscanf(fh, "%lf", bytes_tx);
  if(rv != 1) { *bytes_tx = -1.0; }
  fclose(fh);
 } else {
  *bytes_tx = -1.0;
 }

 fh = NULL;
#if defined (PROC_ETH0)
 fh = fopen("/sys/class/net/eth0/statistics/rx_bytes", "r");
#elif defined (PROC_ETH1)
 fh = fopen("/sys/class/net/eth1/statistics/rx_bytes", "r");
#elif defined (PROC_IB0)
 fh = fopen("/sys/class/net/ib0/statistics/rx_bytes", "r");
#elif defined (PROC_IB1)
 fh = fopen("/sys/class/net/ib1/statistics/rx_bytes", "r");
#endif
 if(fh) {
  rv = fscanf(fh, "%lf", bytes_rx);
  if(rv != 1) { *bytes_rx = -1.0; }
  fclose(fh);
 } else {
  *bytes_rx = -1.0;
 }
#endif 
#endif 

 if(task.flags & DEBUG) {
  printf("IPM: %d get_switchbytes leave tx %f rx %f  \n",
   task.mpi_rank, *bytes_tx, *bytes_rx);
 }

 return 0;
}
