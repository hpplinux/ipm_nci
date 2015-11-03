#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef SQLITE

#include "sqlite3.h"

char buf[4096];
char *errmsg;

static int sql_callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void select_stmt( sqlite3* db, const char* stmt) {
  char *errmsg;
  int   ret;
  int   nrecs = 0;
  int  first_row = 1;

  ret = sqlite3_exec(db, stmt, sql_callback, &nrecs, &errmsg);

  if(ret!=SQLITE_OK) {
    printf("Error in select statement %s [%s].\n", stmt, errmsg);
  }
  else {
    printf("\n   %d records returned.\n", nrecs);
  }
}

#define IPM_LOG_OEVENT(region,call,rank,size,wall) {\
 sprintf(buf,"insert or replace into events values (%d,%d,%d,%d,%f,%f,%f)", region, call, rank, size, wall,wall,wall);\
 if(sqlite3_exec(db,buf,0,0,&errmsg)!=SQLITE_OK) {printf("err [%s]\n",errmsg);};\
}


#define IPM_LOG_EVENT(region,call,rank,size,wall) {\
 rv=sqlite3_bind_double(ustmt,1,wall);\
 sqlite3_bind_int(ustmt,2,region);\
 sqlite3_bind_int(ustmt,3,call);\
 sqlite3_bind_int(ustmt,4,rank);\
 sqlite3_bind_int(ustmt,5,size);\
 rv=sqlite3_step(ustmt);\
 printf("u rv=%d\n",rv);\
 rv=sqlite3_changes(db);\
 printf("c rv=%d\n",rv);\
 if(!rv) {\
  rv=sqlite3_bind_int(istmt,1,region);\
  rv=sqlite3_bind_int(istmt,2,call);\
  rv=sqlite3_bind_int(istmt,3,rank);\
  rv=sqlite3_bind_int(istmt,4,size);\
  rv=sqlite3_bind_double(istmt,5,wall);\
  rv=sqlite3_bind_double(istmt,6,wall);\
  rv=sqlite3_bind_double(istmt,7,wall);\
  rv=sqlite3_step(istmt);\
  printf("i rv=%d\n",rv);\
  rv=sqlite3_changes(db);\
  printf("c rv=%d\n",rv);\
 }\
}

#endif

unsigned long long int flags=0;

#include <ipm.h>
static struct ipm_taskdata task;
static struct ipm_jobdata job;
#include "../src/ipm_init.c"
#include "../src/ipm_util.c"
#include "../src/ipm_hpm.c"
#include "../src/ipm_env.c"
#include "../src/ipm_trace.c"


int main(int argc, char *argv[]) {
 int i,j,k,ib;
 int max_keys = 124096;
 int region,call,rank,size;
 int check_errs=0,region_err,call_err,rank_err,size_err;
 IPM_TICK_TYPE T1,T2;
 IPM_KEY_TYPE key,key_err,hkey;
 double t1,t2,tstart,tstop,wall;
#ifdef SQLITE
 int nrecs; 
 char *errmsg;
 sqlite3* db;
 sqlite3_stmt *ustmt,*istmt;
 int rv;
#endif 

 
 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
   printf("VERBOSE ");
  } else if(!strcmp("-c",*argv)) {
    check_errs=1;
  } else if(!strcmp("-n",*argv)) {
    --argc; argv++;
    max_keys = atoi(*argv);
  }
 }

#ifdef SQLITE
 sqlite3_open(":memory:", &db);
 sqlite3_exec(db, "create table events(region int,call int, rank int,size int,n int ,t_tot real,t_min real ,t_max real)", sql_callback, &nrecs, &errmsg);
 rv=sqlite3_prepare( db, "update events set n=n+1, t_tot=t_tot+$WALL, t_max=max(t_max,$WALL), t_min=min(t_min,$WALL) where region=? and call=? and rank=? and size=?",-1,&ustmt,0); 
 if(rv!=SQLITE_OK) { printf("%d\n", sqlite3_errcode(db)); exit(1); }
 rv=sqlite3_prepare( db, "insert into events values (?,?,?,?,0,?,?,?)",-1,&istmt,0);
 if(rv!=SQLITE_OK) { printf("%d\n", sqlite3_errcode(db)); exit(1); }
#endif 

 task.flags = 0;
 IPM_TIME_BRACKET( ipm_init(task.flags));


 if(task.flags & DEBUG) {
  printf("IPM: %d hash_mem = %lld KB \\n", task.mpi_rank,
         (long long int)(size_t)((MAXSIZE_HASH+2)*sizeof(struct ipm_hash_ent))/1024);
 }

 IPM_TIME_GTOD(tstart);

 for(i=0;i<=max_keys;i++) {

  rank = 1;
  region = 1;
  call = i%5+1;
  size = i;
  if(i<=10) {
   rank = 65525+i;
   size = 2147483637+i; /* 2^31 - 1*/
   call = 245+i;
   region = 245+i;
  }
  wall = 0.1*((rank+call+size+region)%3);

  if(region > 255) { printf("range error in test region = %d\n", region); }
  if(call > 255) { printf("range error in test call = %d\n", call); }
  if(rank > 65535) { printf("range error in test rank = %d\n", rank); }
  if(size > INT_MAX) { printf("range error in test size = %d\n", size); }

/*  IPM_LOG_EVENT(region,call,rank,size,wall); */

 rv=sqlite3_bind_double(ustmt,1,wall);
 sqlite3_bind_int(ustmt,2,region);
 sqlite3_bind_int(ustmt,3,call);
 sqlite3_bind_int(ustmt,4,rank);
 sqlite3_bind_int(ustmt,5,size);
 rv=sqlite3_step(ustmt);
 printf("u rv=%d\n",rv);
 sqlite3_finalize(ustmt);
 sqlite3_reset(ustmt);
 rv=sqlite3_changes(db);
 printf("cu rv=%d\n",rv);
 if(!rv) {
  rv=sqlite3_bind_int(istmt,1,region);
  rv=sqlite3_bind_int(istmt,2,call);
  rv=sqlite3_bind_int(istmt,3,rank);
  rv=sqlite3_bind_int(istmt,4,size);
  rv=sqlite3_bind_double(istmt,5,wall);
  rv=sqlite3_bind_double(istmt,6,wall);
  rv=sqlite3_bind_double(istmt,7,wall);
  rv=sqlite3_step(istmt);
  printf("i rv=%d\n",rv);
  sqlite3_finalize(istmt);
  sqlite3_reset(istmt);
  rv=sqlite3_changes(db);
  printf("ci rv=%d\n",rv);
 }


  if(0)  {
  printf("key  ");
  if(i<11) {
   IPM_SHOWBITS(key);
   printf("\n");
  }
  }

  if(0) {
   printf("hkey %10lld nlog %lld nkey %lld coll %lld r %3d c %3d r %6d s %d\n",
         hkey, task.hash_nlog, task.hash_nkey, collisions,
	 KEY_REGION(key),
	 KEY_CALL(key),
	 KEY_RANK(key),
	 KEY_BYTE(key));
 }
 if(check_errs) {
  region_err = KEY_REGION(key);
  call_err = KEY_CALL(key);
  size_err = KEY_BYTE(key);
  rank_err = KEY_RANK(key);
  

  if(region_err == region &&
     call_err == call &&
     size_err == (size & CALL_BUF_PRECISION(call)) &&
     rank_err == rank) {
   if(0) {
   printf("hkey %10lld nlog %lld nkey %lld coll %lld r %3d c %3d r %6d s %d\n",
         hkey, task.hash_nlog, task.hash_nkey, collisions,
	 KEY_REGION(key),
	 KEY_CALL(key),
	 KEY_RANK(key),
	 KEY_BYTE(key));
    }
   } else {
   if(region_err != region || call_err != call || rank_err != rank || size_err != size) {
    printf("ERR: region(code) %d != region(hash) %d\n", region,region_err);
    IPM_MPI_KEY(key_err,region_err,0,0,0);
    printf("ERR: region(code) "); IPM_SHOWBITS(key_err);
    printf("\n");
    printf("ERR: region(hash) "); IPM_SHOWBITS(key);
    printf("\n");
    printf("ERR: call(code) %d != call(hash) %d\n", call,call_err);
    IPM_MPI_KEY(key_err,0,call_err,0,0);
    printf("ERR: call(code) "); IPM_SHOWBITS(key_err);
    printf("\n");
    printf("ERR: call(hash) "); IPM_SHOWBITS(key);
    printf("\n");
    printf("ERR: rank(code) %d !=rank(hash) %d\n", rank,rank_err);
    IPM_MPI_KEY(key_err,0,0,rank_err,0);
    printf("ERR: rank(code) "); IPM_SHOWBITS(key_err);
    printf("\n");
    printf("ERR: rank(hash) "); IPM_SHOWBITS(key);
    printf("\n");
    printf("ERR: size(code) %d !=size(hash) %d\n", size,size_err);
    IPM_MPI_KEY(key_err,0,0,0,size_err);
    printf("ERR: size(code) "); IPM_SHOWBITS(key_err);
    printf("\n");
    printf("ERR: size(hash) "); IPM_SHOWBITS(key);
    printf("\n");
   }
   
   }
  }
 }
 IPM_TIME_GTOD(tstop);

 printf("END: stored %d keys in %.6e seconds\n", max_keys, tstop-tstart);
#ifdef SQLITE
 sqlite3_finalize(istmt);
 sqlite3_finalize(ustmt);
 select_stmt(db,"select * from events");
 sqlite3_close(db);
#endif
 return 0;
}

