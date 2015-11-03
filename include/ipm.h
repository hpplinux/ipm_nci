#include "config.h"

#ifndef IPM_H_INCLUDED
#define IPM_H_INCLUDED

/* Jie's addition for rounded and checkpoints and mmap sharing*/
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int IPM_ROUNDED = 0;
int IPM_CHECKPOINT = 0;
double CHKPT_INTERVAL = 0;
char IPM_MMAP_FILE[1024];
double SHARE_INTERVAL = 0;
/* for OpenMP only... */
int MPI_LOCAL_RANK=0, MPI_LOCAL_SIZE=0;
struct ipm_stats {
    pid_t pid;    
    unsigned long mtime;
} *share_map = NULL;

int map_fd;
/* end of Jie's addition */

/*
 *   key { callsite call rank size region hpmdelta lastkey }
 *   value { n t_tot t_max t_min }
 */

/***************************************/
/* IPM types                           */
/***************************************/

/*
 * key(2^64) = null(2^4) region(2^4) call(2^8) to_rank(2^16) size(2^32)
 *
 * count_max = 2^64  = 18446744073709551616/(1024*1024*1024*65536*3600)
 *                     72 hours 65536 way one MPI call per cycle
 *
 */

#define IPM_COUNT_TYPE unsigned long long int
#define IPM_MPICOUNT_TYPE MPI_UNSIGNED_LONG_LONG
#define IPM_KEY_TYPE unsigned long long int
#define IPM_MPIKEY_TYPE MPI_UNSIGNED_LONG_LONG
#define IPM_RUSAGE_TYPE struct rusage
#define IPM_ADDR_TYPE unsigned long long int

/***************************************/
/* IPM library [MPI_P|mpi_p]control]   */
/***************************************/

/*
 * 
 * MPI_Pcontrol(0,char *cmd, void *data)
 *
 * cmd is of the form string1(arg1[,arg2]...)
 *
 * data is a typeless address through which data may be returned
 * this memory must be allocated in user code
 *
 * The pcontrol vocabulary

  MPI_Pcontrol("enter_region(region_name)",NULL);
  MPI_Pcontrol("exit_region(region_name)",NULL);
  MPI_Pcontrol("get_region()",(void *)cp);
  MPI_Pcontrol("get_memusage(,ALL)",(void *)dblp);
  MPI_Pcontrol("get_pmap(,ALL)",(void *)dblp);
  MPI_Pcontrol("get_mtime()",(void *)dblp);
  MPI_Pcontrol("get_stime()",(void *)dblp);
  MPI_Pcontrol("get_wtime()",(void *)dblp);
  MPI_Pcontrol("get_wremain()",(void *)dblp);
  MPI_Pcontrol("get_trcbuf()",(void *)ipm_trc_entp);
  MPI_Pcontrol("get_task()",(void *)ipm_task_struct); 
  MPI_Pcontrol("imp_init(port,cred):", (void*)ip);
  MPI_Pcontrol("imp_trc_emit(port,cred):", (void *)ip);
  MPI_Pcontrol("imp_color(r,g,b):", (void *)ip);
  MPI_Pcontrol("imp_line(x1,y1,x2,y2):", (void *)ip);
  MPI_Pcontrol("imp_finalize():", (void*)ip);

 */

/***************************************/
/* IPM library exposed API in C        */
/***************************************/

/* called from MPI_Init or ipm.c, idempotent */
int  ipm_init(unsigned long long int iflags);
void ipm_finalize(void);                /* called from MPI_Finalize */
void ipm_region(int io, char *tag);     /* export ? */
void ipm_event(char *tag);              /* export ? */
void ipm_aggregate(char *tag);          /* needs MPI */
void ipm_report(char *tag);             /* needs MPI */
void ipm_trace(IPM_KEY_TYPE key, double T); /* export ? */


/***************************************/
/* IPM library Misc API                */
/***************************************/

static int ipm_get_env(void);
static int ipm_get_cmdline(char *cmd, char *rpath);
static int ipm_get_procmem(double *bytes);
static int ipm_get_machtopo(void);
static int ipm_get_jobdata(char *jid, char *jun, char *jgn);
static int ipm_get_switchname(char *sname);
static int ipm_get_switchbytes(double *bytes_tx, double *bytes_rx);

/***************************************/
/* static sizes                        */
/***************************************/
#include "ipm_sizes.h"

/***************************************/
/* statics consts                      */
/***************************************/
#include "ipm_flags.h"

/***************************************/
/* IPM library HPM API                 */
/***************************************/
#include "ipm_hpm.h"

#ifndef IPM_ENABLE_PROFLOW
/***************************************/
/* IPM library callsite API            */
/***************************************/
#include "ipm_callsite.h"
#endif

/***************************************/
/* IPM library timer API               */
/***************************************/
#include "ipm_timer.h"


#ifdef OS_AIX
#include <sys/systemcfg.h>
#include <sys/processor.h>
#include "/usr/lpp/ppe.poe/include/pm_util.h"
#endif


typedef struct ipm_hash_ent {
 IPM_KEY_TYPE key;
#ifdef IPM_ENABLE_PROFLOW
 IPM_KEY_TYPE key_last;
 IPM_ADDR_TYPE callsite; 
#endif
 IPM_COUNT_TYPE count;
 double t_tot, t_min, t_max;
#ifdef IPM_ENABLE_POHEAD
 double t_ipm;
#endif
#ifdef IPM_ENABLE_HPMKEY
 IPM_COUNT_TYPE hpm_delta_max[MAXSIZE_HPMCOUNTERS];
 IPM_COUNT_TYPE hpm_delta_min[MAXSIZE_HPMCOUNTERS];
 IPM_COUNT_TYPE hpm_delta_tot[MAXSIZE_HPMCOUNTERS];
#endif
} ipm_hash_ent;


typedef struct ipm_trc_ent {
 IPM_KEY_TYPE key;
#ifdef IPM_ENABLE_PROFLOW
 IPM_ADDR_TYPE callsite;
#endif
 double t;
 long long int          hpm[MAXSIZE_HPMCOUNTERS];
} ipm_trc_ent;

#ifndef IPM_ENABLE_PROFLOW
#define IPM_CLEAR_TRCENT(trc) { \
 trc.key = 0;\
 trc.t = 0.0;\
 for(ii=0;ii<MAXSIZE_HPMCOUNTERS;ii++) trc.hpm[ii] = 0;\
}
#else
#define IPM_CLEAR_TRCENT(trc) { \
 trc.key = 0;\
 trc.callsite = 0;\
 trc.t = 0.0;\
 for(ii=0;ii<MAXSIZE_HPMCOUNTERS;ii++) trc.hpm[ii] = 0;\
}
#endif

struct ipm_jobdata {
 int                    nhosts,ntasks,nexecutables;
 time_t                 start_ts, final_ts;
 char                   ipm_version[MAXSIZE_LABEL];
 char                   username[MAXSIZE_USERNAME];
 char                   groupname[MAXSIZE_USERNAME];
 char                   codename[MAXSIZE_TXTLINE];
 char                   hpcname[MAXSIZE_TXTLINE];
 char                   id[MAXSIZE_TXTLINE];
 char                   cookie[MAXSIZE_TXTLINE];
 char                   compiler[MAXSIZE_TXTLINE];
 char                   arch[MAXSIZE_TXTLINE];
 char                   interconnect[MAXSIZE_TXTLINE];
 char                   os[MAXSIZE_TXTLINE];
 char                   cpu[MAXSIZE_TXTLINE];
 char                   cmdline[MAXSIZE_CMDLINE];
 char                   log_fname[MAXSIZE_FILENAME];
 char                   log_dir[MAXSIZE_FILENAME];
 double                 wtime, utime, stime, mtime, mtime_lost, iotime;
 double			bal_io, bal_comm, bal_comp;
 double                 bytes,flops;
 double                 switch_bytes_tx, switch_bytes_rx;
};

#define IPM_CLEAR_JOBDATA(j) {\
 j.ntasks = j.nhosts = -1;\
 j.start_ts = j.final_ts = 0;\
 sprintf(j.ipm_version,IPM_VERSION);\
 sprintf(j.username,"unknown");\
 sprintf(j.codename,"unknown");\
}

#define IPM_PRINT_JOBDATA(j) {\
 printf("j.start_ts %d\n", j.start_ts);\
 printf("j.final_ts %d\n", j.final_ts);\
}


typedef struct ipm_taskdata {
 pid_t                  pid;
 char                   hostname[MAXSIZE_HOSTNAME];
 char                   mach_info[2*MAXSIZE_TXTLINE];
 char                   mpi_hname_root[MAXSIZE_HOSTNAME];
 char                   mpi_cmdline[MAXSIZE_CMDLINE];
 char                   mpi_execmd5sum[MAXSIZE_CMDLINE];
 char                   mpi_realpath[MAXSIZE_CMDLINE];
 int                    mpi_argc;
 int                    mpi_rank,ipm_rank;
 int                    mpi_size,ipm_size;
 int                    ipm_port,signal;
 int                    inter_left, inter_right;
 int                    intra_size, intra_root, intra_left, intra_right;
 double                 wtime, utime, stime, iotime, mtime, mtime_lost;
 double                 bytes,flops,switch_bytes_tx, switch_bytes_rx;
 double                 stamp_start, stamp_init,stamp_current,stamp_final;
 double                 stamp_init_global, stamp_final_mpi;
 time_t                 start_ts, init_ts, curent_ts, final_ts;
 time_t                 final_ts_mpi;
 IPM_RUSAGE_TYPE        ru_SELF_start,ru_CHILD_start;
 IPM_RUSAGE_TYPE        ru_SELF_init,ru_CHILD_init;
 IPM_RUSAGE_TYPE        ru_SELF_curr, ru_CHILD_curr;
 IPM_RUSAGE_TYPE        ru_SELF_final, ru_CHILD_final;
 long long int          sbrk_s, sbrk_e;
 int                    region_current,region_nregion;
 unsigned long long int region_count[MAXSIZE_REGION];
 unsigned long long int hash_nlog, hash_nkey;
 unsigned long long int call_mask[MAXSIZE_CALLS+1];
 char                   mpi_lang[MAXSIZE_LABEL];
 char                   call_label[MAXSIZE_CALLS+1][MAXSIZE_LABEL];
 char                   region_label[MAXSIZE_REGION][MAXSIZE_LABEL];
 double                 region_wtime[MAXSIZE_REGION];
 double                 region_utime[MAXSIZE_REGION];
 double                 region_stime[MAXSIZE_REGION];
 double                 region_mtime[MAXSIZE_REGION];
 double                 call_mtime[MAXSIZE_REGION][MAXSIZE_CALLS+1];
 unsigned long long int call_mcount[MAXSIZE_REGION][MAXSIZE_CALLS+1];
 ipm_hash_ent           hash[MAXSIZE_HASH+1];
 IPM_TICK_TYPE            ipm_trc_tick_init;
 double                   ipm_trc_time_init;
 int                      ipm_trc_nregion;
 int                      ipm_trc_mask_region_iti[MAXSIZE_REGION];
 int                      ipm_trc_mask_region_itf[MAXSIZE_REGION];
 char                     ipm_trc_mask_region[MAXSIZE_REGION][MAXSIZE_LABEL];
 double                   ipm_trc_mask_time_init;
 double                   ipm_trc_mask_time_final;
 IPM_COUNT_TYPE           ipm_trc_count;
 IPM_COUNT_TYPE           ipm_trc_count_max;
 ipm_trc_ent              *ipm_trc_buf;
 int                    hpm_eventset;
 int                    hpm_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS];
 long long int          hpm_count[MAXSIZE_REGION][MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS];
 long long int            ipm_trc_hpm_region_init[MAXSIZE_HPMCOUNTERS];
 long long int            ipm_trc_hpm_region_final[MAXSIZE_HPMCOUNTERS];
 char                   switch_name[MAXSIZE_LABEL];
 unsigned long long int flags;
} ipm_taskdata;


typedef struct ipm_pthreaddata {
  long long papi_ctrs[MAXSIZE_HPMCOUNTERS];
  long long papi_enter[MAXSIZE_HPMCOUNTERS];
  char papi_running;
  char active;
} ipm_pthreadata;

typedef struct ipm_callsite {
  int id;
  char fname[MAXSIZE_TXTLINE];
} ipm_callsite;


typedef struct ipm_pthread_data {
  long long papi_ctrs[MAXSIZE_HPMCOUNTERS];
  long long papi_enter[MAXSIZE_HPMCOUNTERS];
  char papi_running;
  char active;
} ipm_pthread_data;




#define IPM_PRINT_TASKDATA(t) {\
  printf("task.eventset = %d \n", t.hpm_eventset);\
  for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {\
    ii = ipm_hpm_eorder[t.hpm_eventset][i];\
    printf("ipm_hpm_ename[%d][%d] = %s\n",\
         t.hpm_eventset, ii,\
         ipm_hpm_ename[t.hpm_eventset][ii]);\
  }\
  for(ireg=0;ireg<MAXSIZE_REGION;ireg++) {\
   if(t.region_count[ireg]) {\
   printf("task.region_label[%d] = \"%s\"\n", ireg, t.region_label[ireg]);\
   printf("task.region_count[%d] = %d\n", ireg, t.region_count[ireg]);\
   printf("task.region_wtime[%d] = %.6e\n", ireg, t.region_wtime[ireg]);\
   printf("task.region_utime[%d] = %.6e\n", ireg, t.region_utime[ireg]);\
   printf("task.region_stime[%d] = %.6e\n", ireg, t.region_stime[ireg]);\
   printf("task.region_mtime[%d] = %.6e\n", ireg, t.region_mtime[ireg]);\
   for(i=0;i<MAXSIZE_HPMCOUNTERS;i++) {\
    ii = ipm_hpm_eorder[t.hpm_eventset][i];\
    printf("task.hpm_count[%d][%d][%d] = %lld\n",\
        ireg, t.hpm_eventset, ii,\
        task.hpm_count[ireg][t.hpm_eventset][ii]);\
   }\
  }\
 }\
}



#include "ipm_calls.h"

typedef struct ipm_reportdata {
  double wtime[3], utime[3], stime[3], mtime[3], pmpi[3], flops[3], bytes[3];
  double call_time[MAXSIZE_CALLS+1];
  IPM_COUNT_TYPE call_count[MAXSIZE_CALLS+1];
  long long int hpm_count[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS];
  int pop,pop_flops,pop_event_set[MAXSIZE_NEVENTSETS];
  char label[MAXSIZE_LABEL];
  struct ipm_reportdata *next;
 } reportdata;

#define IPM_CLEAR_REPORTDATA (rep) {\
 rep.wtime[0] = rep.wtime[1] = rep.wtime[2] = 0.0; \
 rep.utime[0] = rep.utime[1] = rep.utime[2] = 0.0; \
 rep.stime[0] = rep.stime[1] = rep.stime[2] = 0.0; \
 rep.mtime[0] = rep.mtime[1] = rep.mtime[2] = 0.0; \
 rep.pmpi[0] = rep.pmpi[1] = rep.pmpi[2] = 0.0; \
 rep.flops[0] = rep.flops[1] = rep.flops[2] = 0.0; \
 rep.bytes[0] = rep.bytes[1] = rep.bytes[2] = 0.0; \
 for(i=0;i<=MAXSIZE_CALLS;i++) { \
  rep.call_time[i] = 0.0; \
  rep.call_count[i] = 0.0; \
 }  \
 rep.pop = rep.pop_flops =0; \
 for(i=0;i<=MAXSIZE_NEVENTSETS;i++) { \
  for(j=0;j<=MAXSIZE_HPMCOUNTERS;j++) { \
   rep.hpm_count[i][j] = 0; \
  } \
 } \
 rep.label[0] = 0; \
 rep.next = NULL; \
}

#define KEY_MASK_BYTE       (0x000000007FFFFFFFULL)
#define KEY_MASK_RANK       (0x0000FFFF00000000ULL)
#define KEY_MASK_NORANK     (0x0000000080000000ULL)
#define KEY_MASK_CALL       (0x00FF000000000000ULL)
#define KEY_MASK_REGION     (0xFF00000000000000ULL)
#define KEY_MASK_IOBYTE     (0x0000FFFFFFFFFFFFULL)

#define KEY_TYPE_MPI   0 
#define KEY_TYPE_MPIIO 1 
#define KEY_TYPE_LIBC  2 

#define KEY_TYPE(k) 
#ifndef IPM_ENABLE_HASHCOMPRESS
#define KEY_BYTE(k)    (int)((k & KEY_MASK_BYTE))
#else 
#define KEY_BYTE(k)    (int)((k & task.call_mask[(int)((k & KEY_MASK_CALL) >> 48)]&CALL_MASK_BPREC)&KEY_MASK_BYTE)
#endif
#define KEY_RANK(k)    ((k&KEY_MASK_NORANK)?(-(int)((k & KEY_MASK_RANK) >> 32)):((int)((k & KEY_MASK_RANK) >> 32)))
#define KEY_CALL(k)    (int)((k & KEY_MASK_CALL) >> 48)
#define KEY_REGION(k)  (int)((k & KEY_MASK_REGION) >> 56)

#define CALL_MASK_STATE     		(0xFF00000000000000ULL)
#define CALL_MASK_OPT       		(0x00FF000000000000ULL)
#define CALL_MASK_BPREC     		(0x0000FFFFFFFFFFFFULL)
#define CALL_OPT_TRACE  		(0x0000000000000001ULL<<48)
#define CALL_OPT_PASS   		(0x0000000000000001ULL<<49)
#define CALL_OPT_NOOP   		(0x0000000000000001ULL<<50)
#define CALL_OPT_WAITB  		(0x0000000000000001ULL<<51)
#define CALL_OPT_WAITA  		(0x0000000000000001ULL<<52)
#define CALL_SEM_RANK_NONE 		(0x0000000000000001ULL<<63)
#define CALL_SEM_DATA_NONE 		(0x0000000000000001ULL<<62)
#define CALL_SEM_DATA_TX 		(0x0000000000000001ULL<<61)
#define CALL_SEM_DATA_RX 		(0x0000000000000001ULL<<60)
#define CALL_SEM_DATA_TXRX 		(0x0000000000000001ULL<<59)
#define CALL_SEM_DATA_COLLECTIVE 	(0x0000000000000001ULL<<58)
#define CALL_SEM_CS_ASYNC 		(0x0000000000000001ULL<<57)
#define CALL_SEM_CS_FSYNC 		(0x0000000000000001ULL<<56)
#define CALL_SEM_CS_PSYNC 		(0x0000000000000001ULL<<55)
#define CALL_SEM_CS_UNUSED 		(0x0000000000000001ULL<<54)
#define CALL_BUF_PRECISION(i)		(task.call_mask[i]&CALL_MASK_BPREC)
#define CALL_BUF_PRECISION_UP(i)        {\
   task.call_mask[i] =\
   ((task.call_mask[i] & (CALL_MASK_STATE | CALL_MASK_OPT))) | \
   (((task.call_mask[i] & CALL_MASK_BPREC) >> 1) & CALL_MASK_BPREC);\
}
#define CALL_BUF_PRECISION_DOWN(i)        {\
   task.call_mask[i] =\
   ((task.call_mask[i] & (CALL_MASK_STATE | CALL_MASK_OPT))) | \
   (((task.call_mask[i] & CALL_MASK_BPREC) << 1) & CALL_MASK_BPREC);\
}

/* IPM_MPI_KEY = calculate key from region, call, rank, size (32 bits) */
/* IPM_IO_KEY = calculate key from region, call, size (40 bits) */
/* IPM_HASH_HKEY = calculate hash key (1<MAXSIZE_HASH) from key */
/* KEY_COUNT = number of unique keys in this hash */
/* KEY_LOG   = number of events logged */

/* MPI buffer sizes are 32 bit ints. I/O calls may express buffer sizes
   > 32 bits. We allow 40. This is 2^40 bytes == 1TB, which while smaller
   that what is technically allowed is suffcient for most I/O calls where
   the user memory buffer would be at most ~GB. It's possible that mmap
   or a similar call would be the first to show this shortcoming.   */

/* Bitwise storage in IPM keys - first 64 bits
 *
 * 6               4               3               1
 * 4               8               2               6              0
 * 0000000000000000000000000000000000000000000000000000000011111111 regi
 * 0000000000000000000000000000000000000000000000001111111100000000 call
 * 0000000000000000000000000000000011111111111111110000000000000000 rank
 * 1111111111111111111111111111111000000000000000000000000000000000 size
 *
 * 1111111111111111111111111111111011111111111111111111111111111111 bound
 * size = 2147483647             (^ plus one MPI unused sign bit)
 * rank = 65535
 * call = 255
 * region = 255
 */

/* old way - problem needs to be fixed when rank < 0 & mask regions overlap
#define IPM_MPI_KEY(key,region,call,rank,size) {  \
 
 key = region; key = key << 8; \
 key |= call;  key = key << 16; \
 key |= rank;  key = key << 32; \
 key |= size;  \
}
*/

#define IPM_MPI_KEY(key,region,call,rank,size) {  \
 key = region; key = key << 8; \
 key |= call;  key = key << 16; \
 if(rank<0) {\
  key |= -rank; key = key << 32;  key |= KEY_MASK_NORANK;\
 } else {\
  key |= rank;  key = key << 32; \
 }\
 key |= (size & CALL_BUF_PRECISION(call)) ;  \
}

#define IPM_IO_KEY(key,region,call,size) {  \
 key = region; key = key << 8; \
 key |= call;  key = key << 40; \
 key |= (size & CALL_BUF_PRECISION(call));  \
}

/*
 printf("DOHASH %lld %d %d %d %d\n", key, region, call, rank, size);\
*/

/*
 printf("IPM: %d IPM_MPI_KEY(*,%d,%d,%d,%d) = (*,%d,%d,%d,%d)\n", \
        task.mpi_rank,\
        region,call,rank,size,\
        KEY_REGION(key),\
        KEY_CALL(key), \
        KEY_RANK(key), \
        KEY_BYTE(key));\
*/

#define IPM_MPI_SANITY(tag,key,region,call,rank,bytes) {\
 if(rank<0 || rank>= task.mpi_size) {\
  if(task.flags & DEBUG) printf("IPM: %d ERROR hashing rank lang=%s region=%d call=%d rank=%d bytes=%d\n", task.mpi_rank, tag, region,call,rank,bytes);\
  rank = 0; \
 } \
 if(call<1 || call> MAXSIZE_CALLS) {\
  if(task.flags & DEBUG) printf("IPM: %d ERROR hashing call lang=%s region=%d call=%d rank=%d bytes=%d\n", task.mpi_rank, tag, region,call,rank,bytes);\
 } \
 if(region<0 || region>= MAXSIZE_REGION) {\
  if(task.flags & DEBUG) printf("IPM: %d ERROR hashing region lang=%s region=%d call=%d rank=%d bytes=%d\n", task.mpi_rank, tag, region,call,rank,bytes);\
  region = 0; \
 } \
 if(bytes<0 || bytes> INT_MAX) {\
  if(task.flags & DEBUG) printf("IPM: %d ERROR hashing bytes lang=%s region=%d call=%d rank=%d bytes=%d\n", task.mpi_rank, tag, region,call,rank,bytes);\
  bytes=0; \
 } \
 /* FIXME -- this is wrong, but now used, if used, must fix */ \
 key = region; key = key << 8; \
 key |= call;  key = key << 16; \
 key |= rank;  key = key << 32; \
 key |= bytes;  \
 if(key == 0) {\
  if(task.flags & DEBUG) printf("IPM: %d ERROR hashing key lang=%s key=%lld region=%d call=%d rank=%d bytes=%d key=%lld\n", task.mpi_rank, tag, key, region,call,rank,bytes,key);\
  key = 0;\
 }\
}

static IPM_KEY_TYPE collisions;

#ifdef IPM_ENABLE_PROFLOW
 IPM_KEY_TYPE key_last=(IPM_KEY_TYPE);
#endif

/* made the first hkey computation faster by knowning collisions == 0 
 hkey = (fkey%MAXSIZE_HASH+collisions*(1+fkey%(MAXSIZE_HASH-2)))%MAXSIZE_HASH; \
*/

#ifdef IPM_ENABLE_PROFLOW
#define HASH_COLLIDE_CONDITIONS(h) || h.callsite != callsite || h.key_last != key_last
#define HASH_INSERT_ACTIONS(h)     h.key_last = key_last; h.callsite = callsite
#else 
#define HASH_COLLIDE_CONDITIONS(h)
#define HASH_INSERT_ACTIONS(h)
#endif

#define IPM_HASH_HKEY(hash,fkey,hkey) {  \
 collisions = 0;  \
 hkey = fkey%MAXSIZE_HASH; \
 /* spending time below should be avoided */  \
 while(hash[hkey].key != fkey HASH_COLLIDE_CONDITIONS(hash[hkey]) ) {  \
  if(hash[hkey].key == 0) { /* new key */  \
   hash[hkey].key = fkey;  \
   hash[hkey].t_tot = 0.0;  \
   hash[hkey].t_min = 1e15;  \
   hash[hkey].t_max = 0.0;  \
   hash[hkey].count = 0;  \
   HASH_INSERT_ACTIONS(hash[hkey]);\
   task.hash_nkey++;\
  } else { /* collision */  \
   if(task.hash_nkey >= MAXSIZE_HASHLIMIT) {  \
    if(task.flags & DEBUG) { \
     printf("IPM: %d ipm_hash_cull  %s\n", task.mpi_rank, "MAXSIZE_HASHLIMIT"); \
    }\
    ipm_hash_cull("MAXSIZE_HASHLIMIT");  \
   }  \
   /* try again */\
   collisions++;  \
   hkey = (fkey%MAXSIZE_HASH+collisions*(1+fkey%(MAXSIZE_HASH-2)))%MAXSIZE_HASH;  \
  }  \
 }  \
}

/* KEY_EQUIV(MASK) */

#ifdef IPM_ENABLE_PROFLOW
#define IPM_MPI_KEY_CLEAR(k) { \
  k.key = 0; \
  k.key_last = 0; \
  k.count = 0; \
  k.t_tot = k.t_min = k.t_max = 0.0; \
}
#else
#define IPM_MPI_KEY_CLEAR(k) { \
  k.key = 0; \
  k.count = 0; \
  k.t_tot = k.t_min = k.t_max = 0.0; \
}
#endif


#define IPM_KEY_PRINT(k) printf("key %d %s %20lld %2d %3d %6d %d\n", task.mpi_rank, task.call_label[KEY_CALL(k)], k,  KEY_REGION(k), KEY_CALL(k), KEY_RANK(k), KEY_BYTE(k));

#define IPM_HASH_PRINT(tag) {\
        printf("%s %d pointer %p\n", tag, task.mpi_rank, task.hash);\
        for(i=0;i<=MAXSIZE_HASH;i++) {\
         if((task.hash[i]).key != 0)\
          printf("%s %d %lld\n", tag, task.mpi_rank, (task.hash[i]).key);\
	 }\
}

#define IPM_HASH_PRINTV(tag) {\
        printf("%s %d pointer %p\n", tag,task.mpi_rank, task.hash);\
        for(i=0;i<=MAXSIZE_HASH;i++) {\
         if(task.hash[i].key != 0)\
          printf("%s %d %s %lld %d %lld %d %d %d %.3e %.3e %.3e\n",\
           tag,\
           task.mpi_rank,\
           task.call_label[KEY_CALL(task.hash[i].key)],\
           task.hash[i].key,\
           KEY_REGION(task.hash[i].key),\
           task.hash[i].count,\
           KEY_CALL(task.hash[i].key),\
           KEY_RANK(task.hash[i].key),\
           KEY_BYTE(task.hash[i].key),\
           task.hash[i].t_tot,\
           task.hash[i].t_min,\
           task.hash[i].t_max);\
         }\
}



#define IPM_HASH_CLEAR(hash) { \
 for(i=0;i<=MAXSIZE_HASH;i++) { \
  IPM_MPI_KEY_CLEAR(hash[i]);  \
 } \
}

/***************************************/
/* IPM Hash macros                     */
/***************************************/

#define IPM_LOG_MPIEVENT(region,call,rank,bytes,wall) { \
 task.hash_nlog++;\
 IPM_MPI_KEY(key,region,call,rank,bytes);\
 IPM_HASH_HKEY(task.hash,key,hkey);\
 task.hash[hkey].count ++;\
 task.hash[hkey].t_tot += (wall);\
 if(wall > task.hash[hkey].t_max) task.hash[hkey].t_max = (wall);\
 if(wall < task.hash[hkey].t_min) task.hash[hkey].t_min = (wall);\
}

#define IPM_LOG_IOEVENT(hash,call,region,bytes) { \
 task.hash_nlog++;\
 IPM_IO_KEY(key,region,call,bytes);\
 IPM_HASH_HKEY(hash,key,hkey);\
 hash[hkey].count ++;\
 hash[hkey].t_tot += wall;\
 if(wall > hash[hkey].t_max) hash[hkey].t_max = wall;\
 if(wall < hash[hkey].t_min) hash[hkey].t_min = wall;\
}

/* FIXME allow for non MPI keys ==> additional check below */
#define IPM_HASH_GET_COMMTIME(hash,t)  {\
 for(t=0.0,i=0;i<=MAXSIZE_HASH;i++) { \
  if(hash[i].key != 0)  { \
   t += hash[i].t_tot; \
  } \
 } \
}

/* FIXME allow for non MPI keys ==> additional check below */
#define IPM_HASH_GET_COMMTIME_REGION(hash,t,r)  {\
 for(t=0.0,i=0;i<=MAXSIZE_HASH;i++) { \
  if(hash[i].key != 0 && KEY_REGION(hash[i].key)==r)  { \
   t += hash[i].t_tot; \
  } \
 } \
}

#define IPM_SHOWCALL(k)  { \
 printf("opt ");\
 if(task.call_mask[k] & CALL_MASK_OPT & CALL_OPT_TRACE) printf("TRACE ");\
 if(task.call_mask[k] & CALL_MASK_OPT & CALL_OPT_PASS) printf("PASS ");\
 if(task.call_mask[k] & CALL_MASK_OPT & CALL_OPT_NOOP) printf("NOOP ");\
}

#define IPM_SHOWBITSI(k)  { \
 for(i=0;i<sizeof(int)*8;i++) { \
  if((k) & 1 <<i)) { printf("%d",1); } else { printf("%d",0); } \
 }\
}

#define IPM_SHOWBITSR(k)  { \
 for(ib=0;ib<sizeof(IPM_KEY_TYPE)*8;ib++) { \
  if((k) & ((IPM_KEY_TYPE)0x0000000000000001ULL <<ib)) { printf("%d",1); } else { printf("%d",0); } \
 }\
}

#define IPM_SHOWBITS(k)  { \
 for(ib=sizeof(IPM_KEY_TYPE)*8-1;ib>=0;ib--) { \
  if(k & ((IPM_KEY_TYPE)0x0000000000000001ULL <<ib)) { printf("%d",1); } else { printf("%d",0); } \
 }\
}




/***************************************/
/* ugly                                */
/***************************************/

static int ipm_report_internal(FILE *fh, int nregion, int jreg,
         char *region_label,
         unsigned long long int flags,
         struct ipm_taskdata *task,
         struct ipm_jobdata *job,
         double g_rbuf[3][MAXSIZE_AGGBUF],
         long long int g_cbuf[3][MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS],
         int g_cpop[MAXSIZE_NEVENTSETS],
         double g_mpi_time[MAXSIZE_CALLS+1],
         double g_mpi_count[MAXSIZE_CALLS+1]);

/***************************************/
/* trace and alarm callbacks           */
/***************************************/
#ifndef IPM_DISABLE_LOG
static void ipm_trc_mask_time_tron();
static void ipm_trc_mask_time_troff();
static void ipm_trc_dump();
#endif


/***************************************/
/* Environment and executable queried  */
/***************************************/


/***************************************/
/*  System wide log                    */
/***************************************/

#ifndef IPM_DISABLE_LOG
#include <sys/stat.h>
#include <sys/param.h>
#ifndef IPM_LOG_DIR
#define IPM_LOG_DIR "."
#endif
#endif


/***************************************/
/* File locking                        */
/***************************************/

#define IPM_FILE_LOCK(fname,fd)   { \
        fd = open(fname, O_CREAT|O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); \
        if(fd == -1) {\
         perror("IPM_FILE_LOCK : open"); \
        }\
        switch(fd) { \
         case -1: { \
          printf("IPM: %d can't open %s\n", task.mpi_rank, fname); \
          break; \
         } \
         default: { \
          break; \
         } \
        } \
        fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);\
        ipm_fl.l_type = F_WRLCK;\
        ipm_fl.l_whence = SEEK_SET;\
        ipm_fl.l_start = 0;\
        ipm_fl.l_len = 0;\
        ipm_fl.l_pid = getpid();\
        if (fcntl(fd, F_SETLKW, &ipm_fl) == -1) { \
         perror("IPM_FILE_LOCK : fcntl"); \
         printf("IPM: %d file_lock failed %s (%d)\n", task.mpi_rank, fname, fd); \
         return 0; \
        } \
        if(task.flags & DEBUG) { \
         printf("IPM: %d file_lock complete %s\n", task.mpi_rank, fname); \
        } \
       }


#define IPM_FILE_UNLOCK(fname, fd) { \
         ipm_fl.l_type = F_UNLCK; \
         ipm_fl.l_whence = SEEK_SET;\
         ipm_fl.l_start = 0;\
         ipm_fl.l_len = 0;\
         ipm_fl.l_pid = getpid();\
         if (fcntl(fd, F_SETLK, &ipm_fl) == -1) { \
          perror("fcntl"); \
          printf("IPM: %d file_unlock failed %s\n", task.mpi_rank, fname); \
         } \
         if(close(fd)) { \
          perror("close ipm_fd"); \
         } \
         if(task.flags & DEBUG) { \
          printf("IPM: %d file_unlock complete %s\n", task.mpi_rank, fname); \
         } \
        }


#endif
