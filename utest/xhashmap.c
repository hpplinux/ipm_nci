#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

unsigned long long int flags=0;
int irand(int lb, int ub);

#include <ipm.h>
static struct ipm_taskdata task;
#include "../src/ipm_util.c"


int main(int argc, char *argv[]) {
 int i,j,k,ib;
 int max_keys = 124096;
 int region,call,rank,byte;
 int region_err,call_err,rank_err,byte_err;
 int argv_region=0,argv_call=0, argv_rank=0, argv_byte=0;
 IPM_TICK_TYPE T1,T2;
 IPM_KEY_TYPE key,key_err,hkey,call_precision;
 double t1,t2;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    max_keys = atoi(*argv);
  } else if(!strcmp("-mpi",*argv)) {
    --argc; argv++;
    sscanf(*argv,"%d,%d,%d,%d",&argv_region,&argv_call,&argv_rank,&argv_byte);
    printf("ARGV: %d %d %d %d\n",argv_region,argv_call,argv_rank,argv_byte);
    max_keys = 0;
  }
 }

 
 IPM_CLEAR_TASKDATA(task);
 task.pid = getpid(); /* should be in ipm_init */

 IPM_TIME_BRACKET( IPM_HASH_CLEAR(task.hash));

 if(task.flags & DEBUG) {
  printf("IPM: %d hash_mem = %lld KB \\n", task.mpi_rank,
         (long long int)(size_t)((MAXSIZE_HASH+2)*sizeof(struct ipm_hash_ent))/1024);
 }

 for(i=0;i<=max_keys;i++) {


  if(i==0) {
   rank = 65535;
   rank = -1;
   byte = INT_MAX;
   call=127;
   region=254;
  } else {
   rank = irand(0,65535);
   byte = irand(0,65535);
   call = irand(0,255);
   region = irand(0,254);
  }

  if(max_keys == 0) {
   rank=argv_rank;
   byte=argv_byte;
   call=argv_call;
   region=argv_region;
  }

  if(region > 255) { printf("range error in test region = %d\n", region); exit(1); }
  if(call > 255) { printf("range error in test call = %d\n", call); exit(1); }
  if(rank > 65535) { printf("range error in test rank = %d\n", rank); exit(1); }
  if(byte > INT_MAX) { printf("range error in test byte = %d\n", byte); exit(1); }

  IPM_MPI_KEY(key,region,call,rank,byte);

  region_err = KEY_REGION(key);
  call_err = KEY_CALL(key);
  byte_err = KEY_BYTE(key);
  rank_err = KEY_RANK(key);

  IPM_HASH_HKEY(task.hash,key,hkey);

    if(flags & VERBOSE) {
    IPM_MPI_KEY(key_err,region_err,call_err,rank_err,byte_err);
    printf("key        "); IPM_SHOWBITS(key); printf("\n");
    printf("key_err    "); IPM_SHOWBITS(key_err); printf("\n");
    printf("norank     "); IPM_SHOWBITS(KEY_MASK_NORANK); printf("\n");

     printf("regi(mask) "); IPM_SHOWBITS(KEY_MASK_REGION); printf("\n");
     printf("call(mask) "); IPM_SHOWBITS(KEY_MASK_CALL); printf("\n");
     call_precision = CALL_BUF_PRECISION(call_err);
     printf("call(opts) "); IPM_SHOWBITS(CALL_MASK_BPREC); printf("\n");
     printf("rank(mask) "); IPM_SHOWBITS(KEY_MASK_RANK); printf("\n"); 
     printf("byte(mask) "); IPM_SHOWBITS(KEY_MASK_BYTE); printf("\n");
     IPM_MPI_KEY(key_err,region_err,0,0,0);
     printf("regi(code) "); IPM_SHOWBITS(key_err); printf("\n");
     printf("regi(hash) "); IPM_SHOWBITS(key); printf("\n");

     IPM_MPI_KEY(key_err,0,call_err,0,0);
     printf("call(code) "); IPM_SHOWBITS(key_err); printf("\n");
     printf("call(hash) "); IPM_SHOWBITS(key); printf("\n");

     IPM_MPI_KEY(key_err,0,0,rank_err,0);
     printf("rank(code) "); IPM_SHOWBITS(key_err); printf("\n");
     printf("rank(hash) "); IPM_SHOWBITS(key); printf("\n");

     IPM_MPI_KEY(key_err,0,0,0,byte_err);
     printf("byte(code) "); IPM_SHOWBITS(key_err); printf("\n");
     printf("byte(hash) "); IPM_SHOWBITS(key); printf("\n");
    }

  if(region_err == region &&
       call_err == call &&
       byte_err == byte &&
       rank_err == rank) {
  printf("hkey %10lld nkey %lld coll %lld map code(%d,%d,%d,%d) --> hash(%d,%d,%d,%d)\n",
         hkey, task.hash_nkey, collisions, 
	 region, call, rank, byte,
	 KEY_REGION(key),
	 KEY_CALL(key),
	 KEY_RANK(key),
	 KEY_BYTE(key));
   } else {
    if(region_err != region) {
     printf("ERR: regi(code) %d <--> region(hash) %d\n", region,region_err);
    }
    if(call_err != call) {
     printf("ERR: call(code) %d <--> call(hash) %d\n", call,call_err);
    }
    if(rank_err != rank) {
     printf("ERR: rank(code) %d <--> rank(hash) %d\n", rank,rank_err);
    }
    if(byte_err != byte) {
     printf("ERR: byte(code) %d <--> byte(hash) %d\n", byte,byte_err);
    }
    exit(1);
    }

    

 }

 return 0;
}

int irand(int lb, int ub) {
 double x;
 x = drand48();
 return (int)(lb+x*(ub-lb));
}

