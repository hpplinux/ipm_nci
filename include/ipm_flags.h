/********************************************************/
/* IPM flags                                            */
/********************************************************/

#ifndef IPM_FLAGS_INCLUDED 
#define IPM_FLAGS_INCLUDED 

#define DEBUG                   (0x0000000000000001ULL <<  0)
#define VERBOSE                 (0x0000000000000001ULL <<  1)
#define REPORT_TERSE            (0x0000000000000001ULL <<  2)
#define REPORT_FULL             (0x0000000000000001ULL <<  3)
#define REPORT_NONE             (0x0000000000000001ULL <<  4)
#define REPORT_LABELIO          (0x0000000000000001ULL <<  5)
#define LOG_TERSE               (0x0000000000000001ULL <<  6)
#define LOG_FULL                (0x0000000000000001ULL <<  7)
#define LOG_NONE                (0x0000000000000001ULL <<  8)
#define IPM_PRELOAD             (0x0000000000000001ULL <<  9)
#define IPM_INITIALIZED         (0x0000000000000001ULL << 10)
#define IPM_PASSTHRU            (0x0000000000000001ULL << 11)
#define IPM_ABORTED             (0x0000000000000001ULL << 12)
#define IPM_INTERRUPTED         (0x0000000000000001ULL << 13)
#define IPM_FINALIZED           (0x0000000000000001ULL << 14)
#define IPM_REGIONALIZED        (0x0000000000000001ULL << 15)
#define IPM_MPI_INITIALIZED     (0x0000000000000001ULL << 16)
#define IPM_MPI_FINALIZING      (0x0000000000000001ULL << 17)
#define IPM_MPI_FINALIZED       (0x0000000000000001ULL << 18)
#define IPM_MPI_INSIDE          (0x0000000000000001ULL << 19)
#define IPM_MPI_ACTIVE          (0x0000000000000001ULL << 20)
#define IPM_MPI_CANCELED        (0x0000000000000001ULL << 21)
#define IPM_MPI_BREAKWAIT       (0x0000000000000001ULL << 22)
#define IPM_MPI_PREBARFSYNC     (0x0000000000000001ULL << 23)
#define IPM_HPM_ACTIVE          (0x0000000000000001ULL << 24)
#define IPM_HPM_CANCELED        (0x0000000000000001ULL << 25)
#define IPM_APP_RUNNING         (0x0000000000000001ULL << 26)
#define IPM_APP_COMPLETED       (0x0000000000000001ULL << 27)
#define IPM_APP_INTERRUPTED     (0x0000000000000001ULL << 28)
#define IPM_WROTELOG            (0x0000000000000001ULL << 29)
#define IPM_TRC_ACTIVE          (0x0000000000000001ULL << 31)
#define IPM_TRC_CANCELED        (0x0000000000000001ULL << 32)
#define PARALLEL_IO_LOG         (0x0000000000000001ULL << 33)

#define APP_STATE_RUNNING       "running"
#define APP_STATE_COMPLETED     "completed"
#define APP_STATE_INTERRUPTED   "interrupted"

/* Jie's change */
//#define DEBUG                   (0x0000000000000001ULL <<  0) || 1
//#define IPM_TRC_ACTIVE          (0x0000000000000001ULL << 31) || 1
//#define DEVEL
/* end of Jie's Change */
#endif /* IPM_FLAGS_INCLUDED  */
