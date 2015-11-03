/***************************************/
/* IPM library HPM includes            */
/***************************************/

#ifndef IPM_HPM_H_INCLUDED
#define IPM_HPM_H_INCLUDED

#include <sys/time.h>
#include <sys/resource.h>

#include "ipm_sizes.h"

#ifdef HPM_DISABLED
#define MAXSIZE_HPMCOUNTERS 1
#define MAXSIZE_NEVENTSETS 1
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = { {0} };
static char                     *ipm_hpm_ename[1][1] = {{"NULL"}};
#define IPM_HPM_CALC_FLOPS(x) (-1)
#endif

#ifdef HPM_PAPI
#include <papi.h>
#endif
                                                                                
#ifdef HPM_PMAPI
#include <pmapi.h>
#endif


/***************************************/
/* IPM library HPM API                 */
/***************************************/

static void ipm_hpm_start();
static void ipm_hpm_stop();
static void ipm_hpm_read(long long int *count);

/*******************************************/
/* IPM library pre-selected HPM events     */
/*******************************************/
/* Last entry is blank - this is for user defined events
entries MUST BE -1 by default
Which are selected with eg.setenv IPM_HPM PAPI_L1_DCA,PAPI_FML_INS
To add another group make sure you add it to the last but one
entry in the papi_event and ipm_hpm_ename arrays.
*/

#ifdef HPM_PAPI
/* PAPI { */

#ifdef IPM_MONITOR_PTHREADS
static int papi_eventset[MAXSIZE_NTHREADS];
#else
static int papi_eventset[1] = {PAPI_NULL};
#endif

static const PAPI_hw_info_t *hwinfo = NULL;

#ifdef CPU_PPC450D

#define MAXSIZE_HPMCOUNTERS 16
#define MAXSIZE_NEVENTSETS 2
static int papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =
  {
   { PAPI_FP_OPS,
     PAPI_FP_INS,
     PAPI_FMA_INS,
     PAPI_LD_INS,
     PAPI_SR_INS,
     PAPI_TOT_CYC,
      PAPI_L1_DCH,
      PAPI_L1_DCM,
      PAPI_L1_DCA,
      PAPI_L1_TCM,
      PAPI_L1_TCW,
      PAPI_L2_DCM,
      PAPI_L2_DCH,
      PAPI_L3_LDM,
      PAPI_L3_TCR,
      PAPI_L3_TCW
     },
    {-1,-1,-1,-1,
    -1,-1,-1,-1,
    -1,-1,-1,-1}
  };

static int ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =
  {{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}};

static char *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =            
  {{ "PAPI_FP_OPS",
     "PAPI_FP_INS",
     "PAPI_FMA_INS",
     "PAPI_LD_INS",
     "PAPI_SR_INS",
     "PAPI_TOT_CYC",
      "PAPI_L1_DCH",
      "PAPI_L1_DCM",
      "PAPI_L1_DCA",
      "PAPI_L1_TCM",
      "PAPI_L1_TCW",
      "PAPI_L2_DCM",
      "PAPI_L2_DCH",
      "PAPI_L3_LDM",
      "PAPI_L3_TCR",
      "PAPI_L3_TCW"
     },
   {"            ", "            ",
   "            ", "            ",
   "            ", "            ",
   "            ", "            ",
   "            ", "            ",
   "            ", "            ",
   "            ", "            ",
    "            ", "            "}
  };
#define IPM_HPM_CALC_FLOPS(x)   ((eventset_current==0)?(x[0]):(-1))
#endif

/***************************************************/
#ifdef CPU_OPTERON
#define MAXSIZE_HPMCOUNTERS 4
#define MAXSIZE_NEVENTSETS 22
static int                      papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] ;
// !!!
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {{0,1,2,3},
{ 0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},
{ 0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},
{ 0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3}
};
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
/* counter groups taken from craypat for quad core opteron */
/*	Group 0: Summary with instructions metrics*/
{"PAPI_TOT_INS",
 "PAPI_FP_OPS",
 "PAPI_L1_DCA",
 "PAPI_L1_DCM"},
/*	Group 1: Summary with TLB metrics*/
  {"PAPI_FP_OPS",
  "PAPI_L1_DCA",
  "PAPI_L1_DCM",
  "PAPI_TLB_DM"},
/*	Group 2: L1 and L2 Metrics*/
  {"PAPI_L1_DCA",
  "DATA_CACHE_REFILLS:L2_MODIFIED:L2_OWNED:L2_EXCLUSIVE:L2_SHARED",
  "DATA_CACHE_REFILLS_FROM_SYSTEM:ALL",
  "REQUESTS_TO_L2:DATA"},
/*	Group 3: Bandwidth information*/
  {"DATA_CACHE_REFILLS:L2_MODIFIED:L2_OWNED:L2_EXCLUSIVE:L2_SHARED",
  "DATA_CACHE_REFILLS_FROM_SYSTEM:ALL",
  "DATA_CACHE_LINES_EVICTED:ALL",
  "QUADWORDS_WRITTEN_TO_SYSTEM:ALL"},
/*	Group 4: <Unused>*/
/*	Group 5: Floating point mix*/
  {"PAPI_FAD_INS",
  "PAPI_FML_INS",
  "PAPI_FDV_INS",
  "RETIRED_MMX_AND_FP_INSTRUCTIONS:PACKED_SSE_AND_SSE2"},
/*	Group 6: Cycles stalled", resources idle*/
  {"PAPI_RES_STL", 
  "PAPI_FPU_IDL",
  "PAPI_STL_ICY",
  "INSTRUCTION_FETCH_STALL"},
/*	Group 7: Cycles stalled, resources full*/
  {"DISPATCH_STALLS", 
  "DISPATCH_STALL_FOR_FPU_FULL",
  "DISPATCH_STALL_FOR_LS_FULL",
  "DECODER_EMPTY"},
/*	Group 8: Instructions and branches*/
  {"PAPI_TOT_INS",
  "INSTRUCTION_CACHE_MISSES",
  "PAPI_BR_TKN",
  "PAPI_BR_MSP"},
/*	Group 9: Instruction cache*/
  {"PAPI_L1_ICA",
  "INSTRUCTION_CACHE_MISSES",
  "PAPI_L2_ICM",
  "INSTRUCTION_CACHE_REFILLS_FROM_L2"},
/*	Group 10: Cache Hierarchy*/
   {"PAPI_L1_DCA",
   "PAPI_L2_DCA",
   "PAPI_L2_DCM",
   "PAPI_L3_TCM"},
/*	Group 11: Floating point operations mix (2)*/
   {"RETIRED_SSE_OPERATIONS:SINGLE_ADD_SUB_OPS:SINGLE_MUL_OPS:OP_TYPE",
   "RETIRED_SSE_OPERATIONS:DOUBLE_ADD_SUB_OPS:DOUBLE_MUL_OPS:OP_TYPE",
   "RETIRED_SSE_OPERATIONS:SINGLE_DIV_OPS:OP_TYPE",
   "RETIRED_SSE_OPERATIONS:DOUBLE_DIV_OPS:OP_TYPE"},
/*	Group 12: Floating point operations mix (vectorization)*/
   {"RETIRED_SSE_OPERATIONS:SINGLE_ADD_SUB_OPS:SINGLE_MUL_OPS",
   "RETIRED_SSE_OPERATIONS:DOUBLE_ADD_SUB_OPS:DOUBLE_MUL_OPS",
   "RETIRED_SSE_OPERATIONS:SINGLE_ADD_SUB_OPS:SINGLE_MUL_OPS:OP_TYPE",
   "RETIRED_SSE_OPERATIONS:DOUBLE_ADD_SUB_OPS:DOUBLE_MUL_OPS:OP_TYPE"},
/*	Group 13: Floating point operations mix (SP)*/
   {"RETIRED_SSE_OPERATIONS:SINGLE_ADD_SUB_OPS",
   "RETIRED_SSE_OPERATIONS:SINGLE_MUL_OPS",
   "RETIRED_SSE_OPERATIONS:SINGLE_ADD_SUB_OPS:OP_TYPE",
   "RETIRED_SSE_OPERATIONS:SINGLE_MUL_OPS:OP_TYPE"},
/*	Group 14: Floating point operations mix (DP)*/
   {"RETIRED_SSE_OPERATIONS:DOUBLE_ADD_SUB_OPS",
   "RETIRED_SSE_OPERATIONS:DOUBLE_MUL_OPS",
   "RETIRED_SSE_OPERATIONS:DOUBLE_ADD_SUB_OPS:OP_TYPE",
   "RETIRED_SSE_OPERATIONS:DOUBLE_MUL_OPS:OP_TYPE"},
/*	Group 15: L3 (socket level)*/
   {"READ_REQUEST_TO_L3_CACHE:ALL",
   "L3_CACHE_MISSES:ALL",
   "L3_FILLS_CAUSED_BY_L2_EVICTIONS:ALL",
   "L3_EVICTIONS:ALL"},
/*	Group 16: L3 (core level reads)*/
   {"READ_REQUEST_TO_L3_CACHE:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_0_SELECT",
   "READ_REQUEST_TO_L3_CACHE:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_1_SELECT",
   "READ_REQUEST_TO_L3_CACHE:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_2_SELECT",
   "READ_REQUEST_TO_L3_CACHE:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_3_SELECT"},
/*	Group 17: L3 (core level misses)*/
   {"L3_CACHE_MISSES:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_0_SELECT",
   "L3_CACHE_MISSES:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_1_SELECT",
   "L3_CACHE_MISSES:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_2_SELECT",
   "L3_CACHE_MISSES:READ_BLOCK_MODIFY:READ_BLOCK_EXCLUSIVE:READ_BLOCK_SHARED:CORE_3_SELECT"},
/*	Group 18: L3 (core level fills caused by L2 evictions)*/
   {"L3_FILLS_CAUSED_BY_L2_EVICTIONS:MODIFIED:OWNED:EXCLUSIVE:SHARED:CORE_0_SELECT",
   "L3_FILLS_CAUSED_BY_L2_EVICTIONS:MODIFIED:OWNED:EXCLUSIVE:SHARED:CORE_1_SELECT",
   "L3_FILLS_CAUSED_BY_L2_EVICTIONS:MODIFIED:OWNED:EXCLUSIVE:SHARED:CORE_2_SELECT",
   "L3_FILLS_CAUSED_BY_L2_EVICTIONS:MODIFIED:OWNED:EXCLUSIVE:SHARED:CORE_3_SELECT"},
/*	Group 19: Prefetchs */
   {"DATA_PREFETCHES:CANCELLED",
   "DATA_PREFETCHES:ATTEMPTED",
   "PREFETCH_INSTRUCTIONS_DISPATCHED:LOAD",
   "PREFETCH_INSTRUCTIONS_DISPATCHED:STORE"},
                                {"            ", "            ",
                                 "            ", "            "}
                                };
#define IPM_HPM_CALC_FLOPS(x)   ((eventset_current==0)?(x[1]):(-1))
#endif
/***************************************************/
#ifdef CPU_NEHALEM

#define MAXSIZE_HPMCOUNTERS 7
#define MAXSIZE_NEVENTSETS 6
static int papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] ;
static int ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =
  {{ 0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6}};

static char *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =            
  {{ "PAPI_FP_OPS",
     "PAPI_FP_INS",
     "PAPI_DP_OPS",
     "PAPI_VEC_DP",
     "            ",
     "            ",
     "            "}, 
     { "PAPI_FP_OPS",
     "PAPI_FP_INS",
     "PAPI_SP_OPS",
     "PAPI_VEC_SP",
     "            ",
     "            ",
     "            "},
    { "PAPI_FP_OPS","PAPI_RES_STL","PAPI_TOT_CYC",
     "PAPI_TOT_INS","            ","            ","            "}, 
    { "PAPI_FP_OPS","PAPI_TLB_DM","PAPI_TLB_IM","            ","            ",
     "            ","            "}, 
   {"PAPI_L1_DCA","PAPI_L2_DCM","PAPI_L2_DCA",
    "            ","            ","            ",
    "            "},
   {"            ", "            ","            ", "            ",
    "            ", "            ", "            "}
  };
#define IPM_HPM_CALC_FLOPS(x)   ((eventset_current==0|| eventset_current==1 || eventset_current==2)?(x[0]):(-1))
#endif


/***************************************************/
#ifdef CPU_CORE2DUO

#define MAXSIZE_HPMCOUNTERS 4
#define MAXSIZE_NEVENTSETS 3
static int papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =
  {
    { PAPI_FP_OPS,
      PAPI_TOT_CYC,
      PAPI_VEC_INS,
      PAPI_TOT_INS  },
    { PAPI_L1_DCM,
      PAPI_L1_DCA,
      -1,
      -1 },
    {-1,-1,-1,-1}
  };

static char *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =
  {{ "PAPI_FP_OPS",
     "PAPI_TOT_CYC",
     "PAPI_VEC_INS",
     "PAPI_TOT_INS" },
   {"PAPI_L1_DCM",
    "PAPI_L1_DCA",
    "            ",
    "            "},
   {"            ", "            ",
    "            ", "            "}
  };

static int ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = 
  {{ 0,1,2,3},{0,1,2,3},{0,1,2,3}};

#define IPM_HPM_CALC_FLOPS(x)   ((eventset_current==0)?(x[0]):(-1))
#endif

/************* Jie's Coustomized eventsets *********************/
#ifdef CPU_NCINF

#define MAXSIZE_HPMCOUNTERS 7 
#define MAXSIZE_NEVENTSETS 15
static int papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] ;
static int ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =
{{ 0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},
 { 0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6},
 { 0,1,2,3,4,5,6},{0,1,2,3,4,5,6},{0,1,2,3,4,5,6}};

static char *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] =            
{
    // default for both XE and VAYU
    { "PAPI_FP_OPS",  // eventset_current = 0
      "PAPI_TOT_INS",
      "PAPI_TOT_CYC", 
      "PAPI_L2_TCA",
      "            ",       
      "            ", 
      "            "},
    // XE only
    { "PAPI_TOT_INS", // eventset_current = 1
      "PAPI_TOT_CYC", 
      "PAPI_L2_TCA",
      "PAPI_L2_TCM", 
      "            ", 
      "            ",
      "            "},
    { "PAPI_TOT_INS", // eventset_current = 2
      "PAPI_TOT_CYC", 
      "PAPI_L2_DCA",
      "            ", 
      "            ", 
      "            ",
      "            "},
    // original for CORE2 from IPM (for xe)
    { "PAPI_FP_OPS", // eventset_current = 3
      "PAPI_TOT_CYC",
      "PAPI_VEC_INS",
      "PAPI_TOT_INS",
      "            ",
      "            "},
    {"PAPI_L1_DCM", // eventset_current = 4
     "PAPI_L1_DCA",
     "            ",
     "            ",
     "            ",
     "            ", 
     "            "},
    // VAYU only
    { "PAPI_FP_OPS", // eventset_current = 5
      "PAPI_TOT_INS",
      "PAPI_TOT_CYC", 
      "PAPI_L2_TCA",
      "PAPI_L2_TCM", 
      "            ", 
      "            "},
    { "PAPI_TOT_INS",// eventset_current = 6
      "PAPI_TOT_CYC",
      "PAPI_L2_STM",
      "PAPI_L2_LDM",
      "PAPI_L2_TCM",
//      "PAPI_L2_DCA", (this gives only 0 counts on vayu! under investigation)
      "PAPI_L2_DCM",
      "            "},
    { "PAPI_TOT_CYC",// eventset_current = 7
      "PAPI_TOT_INS",
      "PAPI_L3_TCM",
      "PAPI_L3_LDM",
      "PAPI_SR_INS",
      "PAPI_LD_INS",
      "            "},    
    { "MEM_LOAD_RETIRED:OTHER_CORE_L2_HIT_HITM", // eventset_current = 8
      "MEM_UNCORE_RETIRED:OTHER_CORE_L2_HITM",
      "            ",
      "            ",
      "            ",
      "            ",
      "            "},
    // original for Nehalem from IPM (on vayu)
    { "PAPI_FP_OPS", // eventset_current = 9
      "PAPI_FP_INS",
      "PAPI_DP_OPS",
      "PAPI_VEC_DP",
      "            ",
      "            ", 
      "            "}, 
    { "PAPI_FP_OPS", // eventset_current = 10
      "PAPI_FP_INS",
      "PAPI_SP_OPS",
      "PAPI_VEC_SP",
      "            ",
      "            ", 
      "            "},
    { "PAPI_FP_OPS", // eventset_current = 11
      "PAPI_RES_STL",
      "PAPI_TOT_CYC",
      "PAPI_TOT_INS",
      "            ",
      "            ", 
      "            "}, 
    { "PAPI_FP_OPS", // eventset_current = 12
      "PAPI_TLB_DM",
      "PAPI_TLB_IM",
      "            ",
      "            ",
      "            ", 
      "            "}, 
    { "PAPI_L1_DCA", // eventset_current = 13
      "PAPI_L2_DCM",
      "PAPI_L2_DCA",
      "            ",
      "            ",
      "            ", 
      "            "},
    { "            ", // eventset_current = 14
      "            ",
      "            ",
      "            ",
      "            ",
      "            ", 
      "            "}
  };
/* #define IPM_HPM_CALC_FLOPS(x)   ((task.hpm_eventset==0|| task.hpm_eventset==3 || task.hpm_eventset==5 \ */
/* 				  || task.hpm_eventset==9|| task.hpm_eventset==10 || task.hpm_eventset==11 \ */
/* 				  || task.hpm_eventset==12 )?(x[0]):(-1)) */

#define IPM_HPM_CALC_FLOPS(x) ((!strcmp(ipm_hpm_ename[task.hpm_eventset][0], \
				       "PAPI_FP_OPS")) ? (x[0]):(-1))

#endif
/**************** end of Jie's defines ******************/

/***************************************************/
#if defined (CPU_PENTIUM_M) || defined(CPU_WOODCREST)
#define MAXSIZE_HPMCOUNTERS 2
#define MAXSIZE_NEVENTSETS 5
static int                      papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{ PAPI_FP_OPS, PAPI_TOT_CYC }, 
                                { PAPI_TOT_IIS, PAPI_TOT_INS},
                                { PAPI_L1_TCM, PAPI_L2_TCM },
                                { PAPI_FML_INS, PAPI_FDV_INS},
                                { -1,-1},
                                };
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{0,1},
				{0,1},
				{0,1},
				{0,1},
				{0,1}
                                };
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{ "PAPI_FP_OPS", "PAPI_TOT_CYC" }, 
                                { "PAPI_TOT_IIS", "PAPI_TOT_INS"},
                                { "PAPI_L1_TCM", "PAPI_L2_TCM"},
                                { "PAPI_FML_INS", "PAPI_FDV_INS"},
                                { "            ", "            "}
                                };
#define IPM_HPM_CALC_FLOPS(x)   ((eventset_current==0)?(x[0]):(-1))
#endif
/***************************************************/

/***************************************************/
#ifdef CPU_PENTIUM_4
#endif
/***************************************************/
                                                                                
/***************************************************/
#ifdef LINUX_ALTIX
/***************************************************/
#if defined (CPU_ITANIUM2) || defined (CPU_ia64)
#define MAXSIZE_HPMCOUNTERS 4
#define MAXSIZE_NEVENTSETS 3
static int                      papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{ PAPI_FP_OPS,
                                  PAPI_TOT_CYC,
				  PAPI_FP_STAL,
                                  PAPI_TLB_DM},
				{ PAPI_FP_OPS,
				  PAPI_FP_STAL,
				  PAPI_BR_MSP,
                                  -1},
                                 {-1,-1,-1,-1}
                                };
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{ 0,1,2,3},
				{ 0,1,2,3},
				{ 0,1,2,3}
	};
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{ "PAPI_FP_OPS",
                                  "PAPI_TOT_CYC",
				  "PAPI_FP_STAL",
                                  "PAPI_TLB_DM"},
				{ "PAPI_FP_OPS",
				  "PAPI_FP_STAL",
                                  "PAPI_BR_MSP",
                                  "IPM_HPM_IGNORE"},
                                {"            ", "            ",
                                 "            ", "            "}
                                };
#define IPM_HPM_CALC_FLOPS(x)   ((eventset_current==0 || eventset_current==1)?(x[0]):(-1))

#endif
/* ITANIUMM2 */
#endif 
/* ALTIX */

/***************************************************/
#ifdef OS_AIX
/***************************************************/
#ifdef CPU_POWER3
#define MAXSIZE_HPMCOUNTERS 7
#define MAXSIZE_NEVENTSETS 1
static int                      papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{ PAPI_LD_INS,
                                  PAPI_SR_INS,
                                  PAPI_FP_OPS,
                                  PAPI_FMA_INS,
                                  PAPI_TLB_TL,
                                  PAPI_TOT_INS,
                                  PAPI_TOT_CYC }
        };
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
	
				 {2,3,0,1,4,5,6};
	}
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				 { "PAPI_LD_INS",
                                   "PAPI_SR_INS",
                                   "PAPI_FP_OPS",
                                   "PAPI_FMA_INS",
                                   "PAPI_TLB_TL",
                                   "PAPI_TOT_INS",
                                   "PAPI_TOT_CYC" }
                                };
#define IPM_HPM_CALC_FLOPS(x)   ((task.hpm_eventset==1)?(x[2]+x[3]):(-1))
#endif
/***************************************************/
#ifdef CPU_POWER4
#define MAXSIZE_HPMCOUNTERS 7
#define MAXSIZE_NEVENTSETS 1
bogus
static int                      papi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				{ PAPI_LD_INS,
                                  PAPI_SR_INS,
                                  PAPI_FP_OPS,
                                  PAPI_FMA_INS,
                                  PAPI_TLB_TL,
                                  PAPI_TOT_INS,
                                  PAPI_TOT_CYC }
        };
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
	
				 {2,3,0,1,4,5,6};
	}
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
				 { "PAPI_LD_INS",
                                   "PAPI_SR_INS",
                                   "PAPI_FP_OPS",
                                   "PAPI_FMA_INS",
                                   "PAPI_TLB_TL",
                                   "PAPI_TOT_INS",
                                   "PAPI_TOT_CYC" }
                                };
#define IPM_HPM_CALC_FLOPS(x)   ((task.hpm_eventset==1)?(x[2]+x[3]):(-1))
#endif
/***************************************************/
#endif
                                                                                
#endif
/* } PAPI */

#ifdef HPM_PMAPI
/* { PMAPI */
#ifdef OS_AIX
/* { AIX */
                                                                                
static pm_data_t                pmapi_data;
static pm_prog_t pmapi_prog;
#ifdef AIX51
static pm_info_t pmapi_info;
#else
static pm_info2_t pmapi_info;
#endif
static pm_groups_info_t pmapi_groups_info;
                                                                                
/***************************************************/
/* POWER3 {                                        */
/***************************************************/
#ifdef CPU_POWER3 
#define MAXSIZE_HPMCOUNTERS 8
#define MAXSIZE_NEVENTSETS 4
static int                      pmapi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
			 {  4,35, 2, 9, 5,12, 9, 0}, 
                         { 19, 9, 5, 2, 6,12,10,10}, 
                         {  6,21, 5,14, 6,12,10,10},
                         {  5, 3, 2, 1, 2, 2, 1, 4}
	};
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
			{ 4,1,6,0,3,7,2,5 },
			{ 2,1,7,6,0,4,3,5 },
			{ 2,0,1,3,7,6,4,5 },
			{ 0,6,7,5,1,2,4,3 }
	};
	
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
	
   {"PM_LD_CMPL", "PM_FPU1_CMPL", "PM_INST_CMPL", "PM_ST_CMPL",
   "PM_FPU0_CMPL", "PM_CYC", "PM_FPU_FMA", "PM_TLB_MISS"}, 
                                                                                
   {"PM_TLB_MISS", "PM_ST_MISS_L1", "PM_LD_MISS_L1", "PM_INST_CMPL",
   "PM_LSU_IDLE", "PM_CYC", "PM_ST_DISP", "PM_LD_DISP"},
                                                                                
   {"PM_LD_MISS_L2HIT","PM_BIU_LD_NORTRY","PM_LD_MISS_L1","PM_BIU_ST_NORTRY",
   "PM_LSU_IDLE", "PM_CYC", "PM_ST_DISP", "PM_LD_DISP"},
                                                                                
   {"PM_IC_MISS", "PM_INST_DISP", "PM_INST_CMPL", "PM_CYC",
   "PM_0INST_CMPL", "PM_FXU2_PROD_RESULT", "PM_FXU0_PROD_RESULT", "PM_FXU1_PROD_RESULT"}

	};
                                                                                
#define IPM_HPM_CALC_FLOPS(x) ((task.hpm_eventset==0)?((x[1]+x[4]+x[6])):(-1))
#endif
/***************************************************/
/* POWER3 }                                        */
/***************************************************/

/***************************************************/
/* POWER4 {                                        */
/***************************************************/
#ifdef CPU_POWER4
#define MAXSIZE_HPMCOUNTERS 8
#define MAXSIZE_NEVENTSETS 5 
/*
The default group is 60. Groups considered interesting for application performance analysis are:

    * 60, counts of cycles, instructions, and FP operations (including divides, FMA, loads, and stores).
    * 59, counts of cycles, instructions, TLB misses, loads, stores, and L1 misses.
    * 5, counts of loads from L2, L3, and memory.
    * 58, counts of cycles, instructions, loads from L3, and loads from memory.
    * 53, counts of cycles, instructions, fixed-point operations, and FP operations (includes divides, SQRT, FMA, and FMOV or FEST).

*/

/* 


#60,84,83,22,27,82,84,78,78,pm_hpmcount2,Hpmcount group for computation intensity analysis
##8000,8000,0013,0017,800F,8020,8001,8930
00000810,01002028,9DCE84A0,00002000
Hpmcount group for computation intensity analysis

#59,6,88,79,70,82,86,84,82,pm_hpmcount1,Hpmcount group for L1 and TLB behavior analysis
##0904,8002,8C10,0C13,800F,8001,8C10,8C10
00001414,010B0000,44CE9420,00002000
Hpmcount group for L1 and TLB behavior analysis

#5,82,82,74,74,83,82,74,75,pm_lsource,Information on data source
##8C66,8C66,8C66,8C66,8C66,8C66,8C66,8C66
00000E1C,0010C000,739CE738,00002000
Information on data source

#58,82,82,74,74,83,81,78,75,pm_pe_bench6,PE Benchmarker group for L3 analysis
##8C66,8C66,8C66,8C66,8C66,800F,8001,8C66
00000E1C,0010C000,739C74B8,00002000
PE Benchmarker group for L3 analysis

#53,84,83,77,75,82,83,78,77,pm_pe_bench1,PE Benchmarker group for FP analysis
##8000,8000,8630,8010,800F,8000,8001,8010
00000810,10001002,420E84A0,00002000
PE Benchmarker group for FP analysis

*/


static int                      pmapi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
			 {  84,83,22,27,82,84,78,78}, 
                         {   6,88,79,70,82,86,84,82}, 
                         {  82,82,74,74,83,82,74,75},
			 {  82,82,74,74,83,81,78,75},
			 {  84,83,77,75,82,83,78,77}
	};
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
			{ 4,1,6,0,3,7,2,5 },
			{ 2,1,7,6,0,4,3,5 },
			{ 2,0,1,3,7,6,4,5 },
			{ 0,6,7,5,1,2,4,3 },
			{ 0,6,7,5,1,2,4,3 }
	};
	
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
	
{"PM_FPU_FDIV", "PM_FPU_FMA", "PM_FPU0_FIN", "PM_FPU1_FIN",
 "PM_CYC", "PM_FPU_STF", "PM_INST_CMPL", "PM_LSU_LDF"}, 
                                                                                
{"PM_DTLB_MISS", "PM_LSU_LMQ_SRQ_EMPTY_CYC", "PM_LD_MISS_L1", "PM_ST_MISS_L1",
 "PM_CYC", "PM_INST_CMPL", "PM_ST_REF_L1", "PM_LD_REF_L1"}, 
                                                                                
{"PM_DATA_FROM_L3", "PM_DATA_FROM_MEM", "PM_DATA_FROM_L35", "PM_DATA_FROM_L2",
 "PM_DATA_FROM_L25_SHR", "PM_DATA_FROM_L275_SHR", "PM_DATA_FROM_L275_MOD", "PM_DATA_FROM_L25_MOD"}, 
                                                                                
{"PM_DATA_FROM_L3", "PM_DATA_FROM_MEM", "PM_DATA_FROM_L35", "PM_DATA_FROM_L2",
 "PM_DATA_FROM_L25_SHR", "PM_CYC", "PM_INST_CMPL", "PM_DATA_FROM_L25_MOD"}, 
                                                                               
{"PM_FPU_FDIV", "PM_FPU_FMA", "PM_FXU_FIN", "PM_FPU_FIN",
 "PM_CYC", "PM_FPU_FSQRT", "PM_INST_CMPL", "PM_FPU_FMOV_FEST"}
                                                                                
	};
                                                                                
#define IPM_HPM_CALC_FLOPS(x) ((task.hpm_eventset==0)?((x[1]+x[2]+x[3]-x[5])):(-1))
#endif
/***************************************************/
/* } POWER4                                        */
/***************************************************/

/***************************************************/
/* POWER5 {                                        */
/***************************************************/
#ifdef CPU_POWER5
#define MAXSIZE_HPMCOUNTERS 6
#define MAXSIZE_NEVENTSETS 1 
static int                      pmapi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
	{105,110,432,253,156,414}
       };
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
			{ 0,1,2,3,4,5}};
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
{"PM_FPU_1FLOP","PM_FPU_FMA","PM_ST_REF_L1","PM_LD_REF_L1","PM_INST_CMPL","PM_RUN_CYC"}
};
#define IPM_HPM_CALC_FLOPS(x) ((task.hpm_eventset==0)?(x[0]+2*x[1]):(-1))

/* } POWER5                                        */
#endif
#ifdef CPU_POWER6
#define MAXSIZE_HPMCOUNTERS 6
#define MAXSIZE_NEVENTSETS 11
pm_events2_t *evp;
static int                      pmapi_event[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS];
static int                     ipm_hpm_eorder[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS] = {
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5},
                        { 0,1,2,3,4,5}
};
static char                     *ipm_hpm_ename[MAXSIZE_NEVENTSETS][MAXSIZE_HPMCOUNTERS]={
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
{"                                   ","                                   ",
 "                                   ","                                   ",
 "                                   ","                                   "},
};
#define IPM_HPM_CALC_FLOPS(x) ((task.hpm_eventset==0)?(x[0]+2*x[1]+4*x[2]):(-1))


/* } POWER6                                        */
#endif
/* } AIX                                           */
#endif
/* } PMAPI                                         */
#endif                                                                     
                                                                                


#endif 
