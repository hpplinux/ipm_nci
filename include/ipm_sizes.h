/***************************************/
/* IPM library static sizes            */
/***************************************/

#ifndef IPM_SIZES_H_INCLUDED
#define IPM_SIZES_H_INCLUDED

#define OOKU (0.0010000000000)  /* 1/1000^1 */
#define OOMU (0.0000010000000)  /* 1/1000^2 */
#define OOGU (0.0000000010000)  /* 1/1000^3 */
#define OOKB (9.765625000e-04)  /* 1/1024^1 */
#define OOMB (9.536743164e-07)  /* 1/1024^2 */
#define OOGB (9.313225746e-10)  /* 1/1024^3 */

#define MAXSIZE_LABEL 21
#define MAXSIZE_HOSTNAME 25
#define MAXSIZE_USERNAME 9
#define MAXSIZE_FILENAME 4096
#define MAXSIZE_TXTLINE 80
#define MAXSIZE_CMDLINE 4096
#define MAXSIZE_REGION 255
#define MAXSIZE_EVENTS 16

/* MAXSIZE_HASH _must_ be a prime number */

#ifdef IPM_ENABLE_HASHXTINY
#define MAXSIZE_HASH      317
#define MAXSIZE_HASHLIMIT 280
#define IPM_GOT_HASHSIZE XTINY
#endif

#ifdef IPM_ENABLE_HASHTINY
#define MAXSIZE_HASH      3257
#define MAXSIZE_HASHLIMIT 2800
#define IPM_GOT_HASHSIZE TINY
#endif

#ifdef IPM_ENABLE_HASHBIG
#define MAXSIZE_HASH      9101977
#define MAXSIZE_HASHLIMIT 2800000
#define IPM_GOT_HASHSIZE BIG
#endif

#ifndef IPM_GOT_HASHSIZE
#define MAXSIZE_HASH      32573
#define MAXSIZE_HASHLIMIT 28000
#define IPM_GOT_HASHSIZE DEFAULT
#endif

#define MAXSIZE_CALLS IPM_NCALLS
#define MAXSIZE_TRACEMEM 0.5
#define MAXSIZE_AGGBUF 10


#define MAXSIZE_NTHREADS 16


#endif /* IPM_SIZES_H_INCLUDED */
