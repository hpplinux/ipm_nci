/***************************************/
/* IPM MPI internal utility macros     */
/***************************************/

/* variables that may be in each wrapper
   int i,ntypes,bytes,rank,size;
   IPM_TICK_TYPE T1,T2;
*/
                                                                                
#define IPM_MPI_BYTES_NONE_C 
#define IPM_MPI_BYTES_NONE_F 
#define IPM_MPI_BYTES_ZERO_C 
#define IPM_MPI_BYTES_ZERO_F 
#define IPM_MPI_BYTES_SCOUNT_C \
   PMPI_Type_size(stype, &ibytes); \
   ibytes = (scount) * ibytes;
#define IPM_MPI_BYTES_SCOUNT_F \
   PMPI_Type_size(*stype, &ibytes); \
   ibytes = (*scount) * ibytes;
#define IPM_MPI_BYTES_RCOUNT_C \
   PMPI_Type_size(rtype, &ibytes); \
   ibytes = (rcount) * ibytes;
#define IPM_MPI_BYTES_RCOUNT_F \
   PMPI_Type_size(*rtype, &ibytes); \
   ibytes = (*rcount) * ibytes;
#define IPM_MPI_BYTES_SCOUNTI_C \
   PMPI_Comm_rank(comm, &irank); \
   PMPI_Type_size(stype, &ibytes); \
   ibytes = scounts[irank] * ibytes;
#define IPM_MPI_BYTES_SCOUNTI_F \
   PMPI_Comm_rank(*comm, &irank); \
   PMPI_Type_size(*stype, &ibytes); \
   ibytes = scounts[irank] * ibytes;
#define IPM_MPI_BYTES_RCOUNTI_C \
   PMPI_Comm_rank(comm, &irank); \
   PMPI_Type_size(rtype, &ibytes); \
   ibytes = rcounts[irank] * ibytes;
#define IPM_MPI_BYTES_RCOUNTI_F  \
   PMPI_Comm_rank(*comm, &irank); \
   PMPI_Type_size(*rtype, &ibytes); \
   ibytes = rcounts[irank] * ibytes;
#define IPM_MPI_BYTES_SCOUNTS_C \
   PMPI_Comm_size(comm, &isize); \
   PMPI_Type_size(stype, &ibytes); \
   for (irank=0,ntypes=0; irank<isize; irank++) ntypes += scounts[irank]; \
   ibytes = ntypes * ibytes;
#define IPM_MPI_BYTES_SCOUNTS_F \
   PMPI_Comm_size(*comm, &isize); \
   PMPI_Type_size(*stype, &ibytes); \
   for (irank=0,ntypes=0; irank<isize; irank++) ntypes += scounts[irank]; \
   ibytes = ntypes * ibytes;
#define IPM_MPI_BYTES_RCOUNTS_C \
   PMPI_Comm_size(comm, &isize); \
   PMPI_Type_size(rtype, &ibytes); \
   for (irank=0,ntypes=0; irank<isize; irank++) ntypes += rcounts[irank]; \
   ibytes = ntypes * ibytes;
#define IPM_MPI_BYTES_RCOUNTS_F \
   PMPI_Comm_size(*comm, &isize); \
   PMPI_Type_size(*rtype, &ibytes); \
   for (irank=0,ntypes=0; irank<isize; irank++) ntypes += rcounts[irank]; \
   ibytes = ntypes * ibytes;

/*

  these work for everything but Waits where rtype does not exist

#define IPM_MPI_BYTES_STATUS_C \
   PMPI_Get_count(status, rtype, &ntypes); \
   PMPI_Type_size(rtype, &ibytes); \
   ibytes = ntypes * ibytes;
#define IPM_MPI_BYTES_STATUS_F \
   PMPI_Get_count(status, *rtype, &ntypes); \
   PMPI_Type_size(*rtype, &ibytes); \
   ibytes = ntypes * ibytes;

*/


#define IPM_MPI_BYTES_STATUS_C if(status != MPI_STATUS_IGNORE) {ibytes = status->MPI_STATUS_COUNT;}
#define IPM_MPI_BYTES_STATUS_F if(status != MPI_STATUS_IGNORE) {ibytes = status->MPI_STATUS_COUNT;}
#define IPM_MPI_BYTES_STATUSI_C if(status != MPI_STATUS_IGNORE) {ibytes = status[*indx].MPI_STATUS_COUNT;}
#define IPM_MPI_BYTES_STATUSI_F if (status  != MPI_STATUS_IGNORE) {if(status+*indx) {ibytes = status[*indx].MPI_STATUS_COUNT;}}
#define IPM_MPI_BYTES_STATUSES_C \
   if(status != MPI_STATUS_IGNORE) {\
   for (irank=0,ntypes=0; irank<num; irank++) if(status+irank) {ntypes += status[irank].MPI_STATUS_COUNT;} \
   ibytes = ntypes;}
#define IPM_MPI_BYTES_STATUSES_F \
   if(status != MPI_STATUS_IGNORE) {\
   for (irank=0,ntypes=0; irank<*num; irank++) if(status+irank) {ntypes += status[irank].MPI_STATUS_COUNT;}; \
   ibytes = ntypes;}
#define IPM_MPI_BYTES_STATUSESP_C \
   if(status != MPI_STATUS_IGNORE) {\
   for (irank=0,ntypes=0; irank<*num; irank++) if(status+irank) {ntypes += status[irank].MPI_STATUS_COUNT;}; \
   ibytes = ntypes;}
#define IPM_MPI_BYTES_STATUSESP_F \
   if(status != MPI_STATUS_IGNORE) {\
   for (irank=0,ntypes=0; irank<*num; irank++) if(status+irank) {ntypes += status[irank].MPI_STATUS_COUNT;}; \
   ibytes = ntypes;}
#define IPM_MPI_BYTES_STATUSESPI_C \
   if(status != MPI_STATUS_IGNORE) {\
   for (irank=0,ntypes=0; irank<*num; irank++) if(status+ind[irank]) {ntypes += status[ind[irank]].MPI_STATUS_COUNT;}; \
   ibytes = ntypes;}
#define IPM_MPI_BYTES_STATUSESPI_F \
   if(status != MPI_STATUS_IGNORE) {\
   for (irank=0,ntypes=0; irank<*num; irank++) if(status+ind[irank]) ntypes += status[ind[irank]].MPI_STATUS_COUNT;};}; \
   ibytes = ntypes;}


#ifdef LINUX_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif

#define IPM_MPI_RANK_ALLRANKS 0
#define IPM_MPI_RANK_NORANK 0
#define IPM_MPI_RANK_NONE_C irank=IPM_MPI_RANK_NORANK
#define IPM_MPI_RANK_NONE_F irank=IPM_MPI_RANK_NORANK
#define IPM_MPI_RANK_ALL_C  irank=IPM_MPI_RANK_ALLRANKS
#define IPM_MPI_RANK_ALL_F irank=IPM_MPI_RANK_ALLRANKS
#define IPM_MPI_RANK_DEST_C irank=dest
#define IPM_MPI_RANK_DEST_F irank=*dest
#define IPM_MPI_RANK_SRC_C irank=src
#define IPM_MPI_RANK_SRC_F irank=*src
#define IPM_MPI_RANK_ROOT_C irank=root
#define IPM_MPI_RANK_ROOT_F irank=*root
#define IPM_MPI_RANK_STATUS_C if(status != MPI_STATUS_IGNORE) {irank=status->MPI_STATUS_SOURCE;}
#define IPM_MPI_RANK_STATUS_F if(status != MPI_STATUS_IGNORE) {irank=status->MPI_STATUS_SOURCE;}
#define IPM_MPI_RANK_STATUSI_C if(status != MPI_STATUS_IGNORE) {irank=status[*indx].MPI_STATUS_SOURCE;}
#define IPM_MPI_RANK_STATUSI_F if(status != MPI_STATUS_IGNORE) {irank=status[*indx].MPI_STATUS_SOURCE;}


/* this translates the above rank to MPI_COMM_WORLD if need be */

#define IPM_MPI_GETGLOBALRANK_C {\
 PMPI_Comm_compare(comm, MPI_COMM_WORLD, &ntypes); \
 if(ntypes == MPI_IDENT || ntypes == MPI_CONGRUENT) { \
  iirank = irank;\
 } else { \
  if(irank >= 0) { \
   iirank = irank;\
   PMPI_Comm_group(comm,&igroup); \
   PMPI_Group_translate_ranks(igroup, 1, &iirank, ipm_world_group, &irank); \
  /*  printf("global %d %d\n", irank, iirank); */\
 }\
 }\
}

#define IPM_MPI_GETGLOBALRANK_F {\
 PMPI_Comm_compare(*comm, MPI_COMM_WORLD, &ntypes); \
 if(ntypes == MPI_IDENT || ntypes == MPI_CONGRUENT) { \
  iirank = irank;\
 } else { \
  if(irank >= 0) { \
   iirank = irank;\
   PMPI_Comm_group(*comm,&igroup); \
   PMPI_Group_translate_ranks(igroup, 1, &iirank, ipm_world_group, &irank); \
  /*  printf("global %d %d\n", irank, iirank); */\
 }\
 }\
}

