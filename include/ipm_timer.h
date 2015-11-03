/***************************************/
/* IPM timer Definitions               */
/***************************************/

#ifndef IPM_TIMER_H_INCLUDED 
#define IPM_TIMER_H_INCLUDED 

/*
 *
 * This is a C macro API for low overhead counters
 * it provides a platform independent abstraction for
 * timing code in libipm.a. It's also used in some
 * of the examples programs although the timers
 * are not part of the IPM API.
 *
 * IPM_STIME_SOURCE -> RDTSC, RTC, READREALTIME, GTOD
 *                    required, defines underlying sources for timer
 *
 * IPM_GTIME_SOURCE -> MPI_WTIME, RDTSC, RTC, READREALTIME, GTOD
 *                    required, defines underlying sources for timer
 *
 * IPM_TIME_INIT   -> to be called once for setup, may be noop
 * IPM_TIME_GET    -> fast counting of clock ticks
 * IPM_TIME_SEC    -> convert ticks to seconds
 * IPM_TIME_GTOD   -> get microseond unix time (slow)
 *
 * a typical use of this API is to, after initializing, 
 * grab two tick values via IPM_TIME_GET before and after
 * some event to be timed. After the event is done and 
 * both timers have been called. The conversion of units
 * and duration of the event can be calculated. 
 *
 * IPM_TIME_GTOD is meant to tell one what the good 
 * old date and time is via a unix timestamp and fractions of seconds
 *
 */


#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <limits.h>

#ifdef OS_AIX
#include <sys/systemcfg.h>
#include <sys/processor.h>
#include <sys/procfs.h>
#endif

#if defined (CPU_ITANIUM2) || defined (CPU_ia64)
#include <ia64intrin.h>
#endif

static struct timeval ipm_tod_tv;
#define IPM_TIME_GTOD(tod)  {gettimeofday(&ipm_tod_tv,NULL); tod=ipm_tod_tv.tv_sec+1.0e-6*ipm_tod_tv.tv_usec;}

extern double ipm_seconds_per_tick; 

/* 
 * STIME source
 */ 

#ifdef IPM_STIME_SOURCE_IS_RDTSC
#define IPM_STIME_SOURCE_IS_DEFINED
static inline long long ipm_rdtsc()
{
        unsigned int low,high;
        __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
        return low + (((long long)high)<<32);
}
#define IPM_TICK_TYPE    unsigned long long int
#define IPM_TIME_GET(t) t=ipm_rdtsc();
#define IPM_TIME_TICK(t) t
#define IPM_TIME_SEC(t) (double)(t*ipm_seconds_per_tick)
#define IPM_TIME_INIT {\
    double speed = 0.0;\
    char sbuffer[1024];\
    sprintf(sbuffer,"/proc/cpuinfo");\
    FILE* fp = fopen(sbuffer,"r");\
    if(fp){\
        while(fgets(sbuffer,1024,fp)){\
            if(!strncmp(sbuffer,"cpu MHz",7)){\
                char* p = strchr(sbuffer,':');\
                if(p){\
                    sscanf(++p,"%lf",&speed);\
                }\
                break;\
            }\
        }\
        fclose(fp);\
    }\
      ipm_seconds_per_tick= (1.0 / (speed * 1.0e6));\
    }
#endif

#ifdef IPM_STIME_SOURCE_IS_READREALTIME
#define IPM_STIME_SOURCE_IS_DEFINED
static double tconvert1,tconvert2;
#define IPM_TICK_TYPE    struct timebasestruct
#define IPM_TIME_GET(t) read_real_time(&(t), TIMEBASE_SZ)
#define IPM_TIME_TICK(t)  ((t).tb_high*1000000000 + (t).tb_low)
#define IPM_TIME_SEC(t) (tconvert1*(tconvert2*((double) (t).tb_high) + 1.0e-9*((double) (t).tb_low)))
#define IPM_TIME_INIT {\
         tconvert1 = ((double) _system_configuration.Xint)/((double) _system_configuration.Xfrac);\
         tconvert2 = 4.294967296;\
        }
#define IPM_TIME_OVERFLOW_CHECK(t1,t2) {\
 if(t2.tb_low < t1.tb_low) {\
  t2.tb_high--; t1.tb_low -= 1000000000;\
 }\
}
#define IPM_TIME_RESOLUTION_CHECK(t1,t2) if (t2 <= t1) t2=t1
#endif

#ifdef IPM_STIME_SOURCE_IS_RTC
#define IPM_STIME_SOURCE_IS_DEFINED
#include <intrinsics.h>
static double ipm_rtc_rate=-1.0;
#define IPM_RUSAGE_TYPE struct rusage
#define IPM_TICK_TYPE    long long
#define IPM_TIME_RATE ipm_rtc_rate
#define IPM_TIME_GET(t)  t=_rtc()
#define IPM_TIME_TICK(t)  t
#define IPM_TIME_SEC(t) (double)(t*IPM_TIME_RATE)
#define IPM_TIME_INIT {\
 ipm_rtc_rate=1.0/irtc_rate_();\
}
#endif 

#ifdef IPM_STIME_SOURCE_IS_ITC
#define IPM_STIME_SOURCE_IS_DEFINED
#define IPM_REQUIRE_SECONDSPERTICK
#define IPM_TICK_TYPE unsigned long
#define IPM_TIME_GET(t) t=__getReg(_IA64_REG_AR_ITC)
#define IPM_TIME_TICK(t) t
#define IPM_TIME_SEC(t) (double)(t*ipm_seconds_per_tick)
#define IPM_TIME_INIT {\
    double speed = 0.0;\
    char sbuffer[1024];\
    sprintf(sbuffer,"/proc/cpuinfo");\
    FILE* fp = fopen(sbuffer,"r");\
    if(fp){\
        while(fgets(sbuffer,1024,fp)){\
            if(!strncmp(sbuffer,"itc MHz",7)){\
                char* p = strchr(sbuffer,':');\
                if(p){\
                    sscanf(++p,"%lf",&speed);\
                }\
                break;\
            }\
        }\
        fclose(fp);\
    }\
      ipm_seconds_per_tick= (1.0 / (speed * 1.0e6));\
    }
#endif


#ifdef IPM_STIME_SOURCE_IS_RTS
#define IPM_STIME_SOURCE_IS_DEFINED
#define IPM_REQUIRE_SECONDSPERTICK
static double tconvert1,tconvert2;
#define IPM_TICK_TYPE    unsigned long long
#define IPM_TIME_GET(t) t=rts_get_timebase();
#define IPM_TIME_TICK(t)  t
#define IPM_TIME_SEC(t) IPM_SECONDSPERTICK*t
#define IPM_TIME_INIT
#endif

#ifdef IPM_STIME_SOURCE_IS_GTOD
#define IPM_STIME_SOURCE_IS_DEFINED
#define IPM_TICK_TYPE    double
#define IPM_TIME_GET(t) IPM_TIME_GTOD(t)
#define IPM_TIME_TICK(t)  t
#define IPM_TIME_SEC(t) t
#define IPM_TIME_INIT
#endif

#ifdef IPM_STIME_SOURCE_IS_NULL
#define IPM_STIME_SOURCE_IS_DEFINED
#define IPM_TICK_TYPE    double
#define IPM_TIME_GET(t) t=0;
#define IPM_TIME_TICK(t)  t
#define IPM_TIME_INIT
#warning Using NULL timers. Timings will be invalid.
#endif

#ifndef IPM_STIME_SOURCE_IS_DEFINED
#error IPM_STIME_SOURCE undefined in ipm_timer.h
#endif

#ifndef IPM_TIME_OVERFLOW_CHECK
#define IPM_TIME_OVERFLOW_CHECK(t1,t2)
#endif

#ifndef IPM_TIME_RESOLUTION_CHECK
#define IPM_TIME_RESOLUTION_CHECK(t1,t2)
#endif

#ifdef IPM_REQUIRE_SECONDSPERTICK
#ifndef IPM_SECONDSPERTICK
#error IPM_SECONDSPERTICK_UNDEFINED_IN_IPM_TIMER_H
#endif
#endif

/********************************************************/
/* platform independent (hopefully) higher level macros */
/********************************************************/

#define IPM_TIME_BRACKET(code) {\
        IPM_TIME_GET(T1);\
        code;\
        IPM_TIME_GET(T2);\
        }

#define IPM_GETRUSAGE_SELF(u) getrusage(RUSAGE_SELF,&u)
#define IPM_GETRUSAGE_CHILD(u) getrusage(RUSAGE_CHILDREN,&u)
#define IPM_UTIME_GET(u) (1.0*u.ru_utime.tv_sec+(1.0e-6)*u.ru_utime.tv_usec)
#define IPM_STIME_GET(u) (1.0*u.ru_stime.tv_sec+(1.0e-6)*u.ru_stime.tv_usec)

#define IPM_GETRUSAGE_PRINT(u) { \
   printf("ru_utime.:  %ld.%6.6ld\\n", u.ru_utime.tv_sec, u.ru_utime.tv_usec);\
   printf("ru_stime.:  %ld.%6.6ld\\n", u.ru_stime.tv_sec, u.ru_stime.tv_usec);\
   printf("ru_maxrss:  %ld\\n",u.ru_maxrss);\
   printf("ru_ixrss:   %ld\\n",u.ru_ixrss);\
   printf("ru_idrss:   %ld\\n",u.ru_idrss);\
   printf("ru_isrss:   %ld\\n",u.ru_isrss);\
   printf("ru_minflt:  %ld\\n",u.ru_minflt);\
   printf("ru_majflt:  %ld\\n",u.ru_majflt);\
   printf("ru_inblock: %ld\\n",u.ru_inblock);\
   printf("ru_oublock: %ld\\n",u.ru_oublock);\
   printf("ru_msgsnd:  %ld\\n",u.ru_msgsnd);\
   printf("ru_msgrcv:  %ld\\n",u.ru_msgrcv);\
   printf("ru_nvcsw:   %ld\\n",u.ru_nvcsw);\
   printf("ru_nivcsw:  %ld\\n",u.ru_nivcsw);\
}


#endif /* IPM_TIMER_H_INCLUDED */
