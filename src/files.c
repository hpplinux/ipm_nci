#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <magic.h>
#include <math.h>
#include <ftw.h>
#include <pwd.h>
#include <grp.h>


/*
 *      files : summarize files in a directory
 *
 *      David Skinner   Oct 2005 (dskinner@nersc.gov)
 *
 *      usage: files [-xml] [-a] dirname
 *
 */


#define MAXLEN_FTYPE	80
#define MAXLEN_FNAME	800
#define MAXLEN_LIST	8192

char *user_list_select=NULL, *group_list_select=NULL;
char user_list_found[MAXLEN_LIST], group_list_found[MAXLEN_LIST];
char tag[MAXLEN_LIST];
int ftypes_num, ftypes_max = 1024;
int *sort_by,*ftypes_by_nfile, *ftypes_by_nbyte, *ftypes_by_nblock;
int *ftypes_by_cage, *ftypes_by_mage;
char *files_root, *files_root_realpath;
time_t files_start_t, files_stop_t;
int blocksize=-1, nuid_select=0, ngid_select=0;
char **user_select=NULL, **group_select=NULL;
uid_t *uid_select;
gid_t *gid_select;
struct passwd *pwent;
struct group *grent;

int usage(void);
unsigned int strhash(const char *s);
int files_report_xml(void);
int files_report_txt(void);
unsigned long long int flags=0;
magic_t my_magic=NULL;

#define DEBUG 	        (0x0000000000000001ULL << 0)
#define VERBOSE  	(0x0000000000000001ULL << 1)
#define REPORT_TXT  	(0x0000000000000001ULL << 2)
#define REPORT_XML  	(0x0000000000000001ULL << 3)
#define SORT_BY_NBYTE   (0x0000000000000001ULL << 5)
#define SORT_BY_NFILE   (0x0000000000000001ULL << 6)
#define SORT_BY_NBLOCK  (0x0000000000000001ULL << 7)
#define SORT_BY_CAGE    (0x0000000000000001ULL << 8)
#define SORT_BY_MAGE    (0x0000000000000001ULL << 9)
#define JUST_CORE       (0x0000000000000001ULL << 10)

int human_fname(char *afname, char *fname);
int human_number(char *sx, double x);
int human_byte(char *sx, double x);
int human_age(char *sx, double x);

#ifdef ENABLE_NFTW
int pfile(const char *fname, const struct stat *st, int flag, struct FTW *s); 
#else 
int pfile(const char *fname, const struct stat *st, int flag); 
#endif

typedef struct ftype_ent {
 char desc[MAXLEN_FTYPE]; 
 unsigned long long int nfile,nbyte,nblock;
 time_t cage_min, cage_max;
 time_t mage_min, mage_max;
 off_t byte_min, byte_max;
 blkcnt_t block_min, block_max;
 double cage_avg, mage_avg, byte_avg, block_avg;
 char cage_minf[MAXLEN_FNAME], cage_maxf[MAXLEN_FNAME];
 char mage_minf[MAXLEN_FNAME], mage_maxf[MAXLEN_FNAME];
 char byte_minf[MAXLEN_FNAME], byte_maxf[MAXLEN_FNAME];
 char block_minf[MAXLEN_FNAME], block_maxf[MAXLEN_FNAME];
} ftype_ent;

#define CLEAR_FTYPE(a) {\
 sprintf(a.desc,"");\
 a.nfile = a.nbyte = a.nblock = 0; \
 a.cage_min = a.cage_max = 0;\
 a.mage_min = a.mage_max = 0;\
 a.byte_min = a.byte_max = 0;\
 a.block_min = a.block_max = 0;\
 a.cage_avg = a.cage_avg =  0.0;\
 a.byte_avg = a.block_avg =  0.0;\
 sprintf(a.cage_minf,""); sprintf(a.cage_maxf,"");\
 sprintf(a.mage_minf,""); sprintf(a.mage_maxf,"");\
 sprintf(a.byte_minf,""); sprintf(a.byte_maxf,"");\
 sprintf(a.block_minf,""); sprintf(a.block_maxf,"");\
}

#define HASH_INSERT(a,key,hkey) {\
 collisions = 0;\
 key = strhash(a.desc); \
 hkey = (fkey%MAXSIZE_HASH+collisions*(1+fkey%(MAXSIZE_HASH-2)))%MAXSIZE_HASH; \
}

/*
 *
 */

ftype_ent *t,T;

int usage(void) {
 printf("\t\n");
 printf("usage: files [options] directory\n");
 printf("\t\n");
 printf("summary of files in a directory by type, number ownership and size\n");
 printf("\t\n");
 printf("\t-u csv_list\tonly show files owned by users in list\n");
 printf("\t-g csv_list\tonly show files owned by groups in list\n");
 printf("\t-sort field\torder output by {bytes, files, blocks, cage, mage}\n");
 printf("\t--core\tjust show all corefiles\n");
 printf("\t--xml\toutput XML\n");
 printf("\n");
 exit(1);
}

int main(int argc, char *argv[]) {

 int i,j,k,n,rv,ftw_flags=0,ftw_nopen_fd=40;
 char *cp,*cp2;
 struct stat st;

 flags = 0;

 flags |= SORT_BY_NBYTE;
 flags |= REPORT_TXT;

 while(--argc && argv++) {
  if(!strcmp("-v",*argv)) {
   flags |= VERBOSE;
  } else if(!strcmp("--debug",*argv)) {
   flags |= DEBUG;
  } else if(!strcmp("--xml",*argv)) {
   flags |= REPORT_XML;
   flags &= ~REPORT_TXT;
  } else if(!strcmp("-sort",*argv)) {
    --argc; argv++;
    flags &= ~SORT_BY_NBYTE;
    if(*argv) {
     if(!strcmp("bytes",*argv)) { flags |= SORT_BY_NBYTE; }
     if(!strcmp("blocks",*argv)) { flags |= SORT_BY_NBLOCK; }
     if(!strcmp("files",*argv)) { flags |= SORT_BY_NFILE; }
     if(!strcmp("mage",*argv)) { flags |= SORT_BY_MAGE; }
     if(!strcmp("cage",*argv)) { flags |= SORT_BY_CAGE; }
    } else {
     usage();
    }
  } else if(!strcmp("-u",*argv)) {
    --argc; argv++;
    i = strlen(*argv); 
    if(i>MAXLEN_LIST) {
     printf("user list too long (>%d char)\n",MAXLEN_LIST); exit(1);
    }
    user_list_select = (char *)malloc((size_t)(i*sizeof(char)));
    strncpy(user_list_select,*argv, i);
  } else if(!strcmp("-g",*argv)) {
    --argc; argv++;
    i = strlen(*argv); 
    if(i>MAXLEN_LIST) {
     printf("group list too long (>%d char)\n",MAXLEN_LIST); exit(1);
    }
    group_list_select = (char *)malloc((size_t)(i*sizeof(char)));
    strncpy(group_list_select,*argv, i);
  } else if(!strcmp("-m",*argv)) {
    --argc; argv++;
    ftypes_max = atoi(*argv);
  } else if(!strcmp("--core",*argv)) {
    flags |= JUST_CORE;
    flags &= ~REPORT_TXT;
  } else {
    rv = stat(*argv, &st);
    if(rv) { 
     perror("dir not found ");
     usage();
    }
    i = strlen(*argv); 
    if(i>MAXLEN_LIST) {
     printf("path name too long (>%d char)\n",MAXLEN_LIST); exit(1);
    }
    files_root = (char *)malloc((size_t)(i*sizeof(char)));
    strncpy(files_root,*argv, i);
    files_root_realpath = (char *)malloc((size_t)(MAXLEN_LIST*sizeof(char)));
    cp = realpath(files_root,files_root_realpath);
    if(!cp) {
     printf("path = %s\n", files_root);
     perror("realpath");
     exit(1);
    }
    
  }
 }

 t = (ftype_ent *)malloc((size_t)(ftypes_max*sizeof(ftype_ent)));

 if(user_list_select) { /* break up the list and determine uids */
  for(nuid_select=1, i=0;i<strlen(user_list_select); i++) {
   if(user_list_select[i] == ',') nuid_select ++;
  }
  user_select = (char **)malloc((size_t)(nuid_select*sizeof(char*)));
  user_select[0] = user_list_select;
  uid_select = (uid_t *)malloc((size_t)(nuid_select*sizeof(uid_t)));
  for(i=0;i<nuid_select;i++) uid_select[i] = (uid_t)NULL;
  k = strlen(user_list_select);
  for(j=1, i=0;i<k; i++) {
   if(user_list_select[i] == ',')  {
    user_select[j] = user_list_select+i+1;
    user_list_select[i] = '\0';
    j++;
   }
  }
  while(pwent = getpwent()) {
   for(i=0;i<nuid_select;i++) { 
    if(!strcmp(pwent->pw_name,user_select[i])) {
     uid_select[i] = pwent->pw_uid;
    }
   }
  }
  setpwent();
 }

 if(group_list_select) { /* break up the list and determine gids */
  for(ngid_select=1, i=0;i<strlen(group_list_select); i++) {
   if(group_list_select[i] == ',') ngid_select ++;
  }
  group_select = (char **)malloc((size_t)(ngid_select*sizeof(char*)));
  group_select[0] = group_list_select;
  gid_select = (gid_t *)malloc((size_t)(ngid_select*sizeof(gid_t)));
  for(i=0;i<ngid_select;i++) gid_select[i] = (gid_t)NULL;
  k = strlen(group_list_select);
  for(j=1, i=0;i<k; i++) {
   if(group_list_select[i] == ',')  {
    group_select[j] = group_list_select+i+1;
    group_list_select[i] = '\0';
    j++;
   }
  }
  while((grent = getgrent()) != NULL) {
   for(i=0;i<ngid_select;i++) { 
    if(!strcmp(grent->gr_name,group_select[i])) {
     gid_select[i] = grent->gr_gid;
    }
   }
  }
  setgrent();
 }

 if(flags & DEBUG) {
  if(user_list_select) {
   printf("USER_SELECT: %d ", nuid_select);
   for(i=0;i<nuid_select;i++) printf("%s ",user_select[i]);
   printf("\n");
  } else {
   printf("USER_SELECT: * *\n");
  }
  if(group_list_select) {
   printf("GROUP_SELECT: %d ", ngid_select);
   for(i=0;i<ngid_select;i++) printf("%s ",group_select[i]);
   printf("\n");
  } else {
   printf("GROUP_SELECT: * *\n");
  }
 }

 if(flags & VERBOSE) {
  printf("MEM: allocated %d bytes for %d ftypes_max\n",
	 ftypes_max*sizeof(ftype_ent), ftypes_max);
 }

 for(i=0;i<ftypes_max;i++) {
  CLEAR_FTYPE(t[i]);
 }

 files_start_t = time(NULL);

#ifdef ENABLE_NFTW
 if(ftw_flags == 0) {
  ftw_flags = FTW_PHYS;
 }
#endif

 my_magic = magic_open(MAGIC_NONE|MAGIC_PRESERVE_ATIME);
 if(!my_magic) {
  printf("error magic_open %d\n", my_magic); exit(1);
 }

 rv = magic_load(my_magic, NULL);
 if(rv) {
  printf("error magic_load rv=%d. \n",rv); exit(1);
 }

 rv = stat(files_root,&st);

 if(rv) { 
  usage();
  perror("directory not found ");
 }

 user_list_found[0]=',';
 group_list_found[0]=',';

#ifdef ENABLE_NFTW
 nftw(files_root,&pfile,ftw_nopen_fd,ftw_flags);
#else 
 ftw(files_root,&pfile,ftw_nopen_fd);
#endif

 /* first pass */
 CLEAR_FTYPE(T); ftypes_num=0;
 sprintf(T.desc,"*");
 for(i=0;i<ftypes_max;i++) {
   if(t[i].nfile > 0 ) {
   ftypes_num ++;
   if(T.nfile == 0) {
    T.nfile = t[i].nfile;
    T.nbyte = t[i].nbyte;
    T.cage_min = t[i].cage_min;
    T.cage_max = t[i].cage_max;
    T.mage_min = t[i].mage_min;
    T.mage_max = t[i].mage_max;
    strncpy(T.cage_minf,t[i].cage_minf,MAXLEN_FNAME);
    strncpy(T.cage_maxf,t[i].cage_maxf,MAXLEN_FNAME);
    strncpy(T.mage_minf,t[i].mage_minf,MAXLEN_FNAME);
    strncpy(T.mage_maxf,t[i].mage_maxf,MAXLEN_FNAME);
    strncpy(T.byte_minf,t[i].byte_minf,MAXLEN_FNAME);
    strncpy(T.byte_maxf,t[i].byte_maxf,MAXLEN_FNAME);
    strncpy(T.block_minf,t[i].block_minf,MAXLEN_FNAME);
    strncpy(T.block_maxf,t[i].block_maxf,MAXLEN_FNAME);
   } else {
    T.nfile += t[i].nfile;
    T.nbyte += t[i].nbyte;
    T.nblock += t[i].nblock;
    if(t[i].cage_min < T.cage_min) {
     T.cage_min = t[i].cage_min;
     strncpy(T.cage_minf,t[i].cage_minf,MAXLEN_FNAME);
    }
    if(t[i].cage_max > T.cage_max) {
     T.cage_max = t[i].cage_max;
     strncpy(T.cage_maxf,t[i].cage_maxf,MAXLEN_FNAME);
    }
    if(t[i].mage_min < T.mage_min) {
     T.mage_min = t[i].mage_min;
     strncpy(T.mage_minf,t[i].mage_minf,MAXLEN_FNAME);
    }
    if(t[i].mage_max > T.mage_max) {
     T.mage_max = t[i].mage_max;
     strncpy(T.mage_maxf,t[i].mage_maxf,MAXLEN_FNAME);
    }
    if(t[i].byte_max > T.byte_max) {
     T.byte_max = t[i].byte_max;
     strncpy(T.byte_maxf,t[i].byte_maxf,MAXLEN_FNAME);
    }
    if(t[i].byte_min < T.byte_min) {
     T.byte_min = t[i].byte_min;
     strncpy(T.byte_minf,t[i].byte_minf,MAXLEN_FNAME);
    }
    if(t[i].block_max > T.block_max) {
     T.block_max = t[i].block_max;
     strncpy(T.block_maxf,t[i].block_maxf,MAXLEN_FNAME);
    }
    if(t[i].block_min < T.block_min) {
     T.block_min = t[i].block_min;
     strncpy(T.block_minf,t[i].block_minf,MAXLEN_FNAME);
    }
   }
   }
  }

 /* second pass -- unhash */

 ftypes_by_nfile = (int *)malloc((size_t)(ftypes_num*sizeof(int)));
 ftypes_by_nbyte = (int *)malloc((size_t)(ftypes_num*sizeof(int)));
 ftypes_by_nblock = (int *)malloc((size_t)(ftypes_num*sizeof(int)));
 ftypes_by_cage = (int *)malloc((size_t)(ftypes_num*sizeof(int)));
 ftypes_by_mage = (int *)malloc((size_t)(ftypes_num*sizeof(int)));

 if(!ftypes_by_mage) {
  printf("malloc failed for sort indices (N=%d)\n", ftypes_num);
  exit(1);
 }

 if(flags & SORT_BY_NBYTE) { sort_by = ftypes_by_nbyte; }
 if(flags & SORT_BY_NFILE) { sort_by = ftypes_by_nfile; }
 if(flags & SORT_BY_NBLOCK) { sort_by = ftypes_by_nblock; }
 if(flags & SORT_BY_CAGE) { sort_by = ftypes_by_cage; }
 if(flags & SORT_BY_MAGE) { sort_by = ftypes_by_mage; }

/* initialize sort indices and make averages right */
 j = 0; 
 for(i=0;i<ftypes_max;i++) {
  if(t[i].nfile > 0 ) {
   ftypes_by_nfile[j] = i;
   ftypes_by_nbyte[j] = i;
   ftypes_by_nblock[j] = i;
   ftypes_by_mage[j] = i;
   ftypes_by_cage[j] = i;
   t[i].cage_avg /= t[i].nfile;
   t[i].mage_avg /= t[i].nfile;
   t[i].byte_avg = (1.0*t[i].nbyte)/t[i].nfile;
   t[i].block_avg =(1.0*t[i].nblock)/t[i].nfile;
   if(flags & DEBUG) {
    printf("TYPE: %s %d %d\n", t[i].desc, j, i);
   }
   j++;
  }
 } 

 /* third pass -- sort */

 for(i=0;i<ftypes_num;i++) {
  for(j=0;j<i;j++) {
   if(t[ftypes_by_nfile[i]].nfile > t[ftypes_by_nfile[j]].nfile) {
    k = ftypes_by_nfile[j];
    ftypes_by_nfile[j] = ftypes_by_nfile[i];
    ftypes_by_nfile[i] = k;
   }
   if(t[ftypes_by_nbyte[i]].nbyte > t[ftypes_by_nbyte[j]].nbyte) {
    k = ftypes_by_nbyte[j];
    ftypes_by_nbyte[j] = ftypes_by_nbyte[i];
    ftypes_by_nbyte[i] = k;
   }
   if(t[ftypes_by_nblock[i]].nblock > t[ftypes_by_nblock[j]].nblock) {
    k = ftypes_by_nblock[j];
    ftypes_by_nblock[j] = ftypes_by_nblock[i];
    ftypes_by_nblock[i] = k;
   }
   if(t[ftypes_by_cage[i]].cage_avg < t[ftypes_by_cage[j]].cage_avg) {
    k = ftypes_by_cage[j];
    ftypes_by_cage[j] = ftypes_by_cage[i];
    ftypes_by_cage[i] = k;
   }
   if(t[ftypes_by_mage[i]].mage_avg < t[ftypes_by_mage[j]].mage_avg) {
    k = ftypes_by_mage[j];
    ftypes_by_mage[j] = ftypes_by_mage[i];
    ftypes_by_mage[i] = k;
   }
  }
 }

 if(flags & DEBUG) {
  for(i=0;i<ftypes_num;i++) {
    printf("STYPE: %d byte %d block %d file %d cage %d mage %d\n", i, 
	 ftypes_by_nbyte[i],
	 ftypes_by_nblock[i],
	 ftypes_by_nfile[i],
	 ftypes_by_cage[i],
	 ftypes_by_mage[i]);
  }
 }

 if(flags & REPORT_TXT) files_report_txt();
 if(flags & REPORT_XML) files_report_xml();

 magic_close(my_magic);
 return 0;
 /* Tomorrow is already here */
}

int truncate_prefix_nentry = 2;
char *truncate_prefix[] = {
"symbolic link",
"broken symbolic link"
 };

int truncate_abbriv_nentry = 2;
char *truncate_abbriv[] = {
"script text",
"ELF 32-bit LSB executable"
};

/* most/all of the below are handled by the "," rule */
/*
"gzip compressed data",
"Zip archive data",
"ELF 32-bit LSB executable",
"ELF 32-bit LSB relocatable",
"ELF 32-bit LSB core file",
"ELF 32-bit LSB shared object",
"PostScript document text",
"PNG image data",
"Mathematica, or Pascal",
"COFF format alpha executable",
"ASCII C++ program text", 
"ASCII C program text",
"ASCII English text", 
"ASCII text",
"ASCII Java program text",
"GIF image data",
"MS-DOS executable",
"PC bitmap data",
"SGI image data",
"MS Windows PE 32-bit Intel",
"Targa image data",
"PDF document",
"JPEG image data"
*/

#ifdef ENABLE_NFTW
int pfile(const char *fname, const struct stat *st, int flag, struct FTW *s)
#else 
int pfile(const char *fname, const struct stat *st, int flag)
#endif
{

  int i,j,k,collisions;
  const char *ftype;
  static char ftype_last[MAXLEN_FTYPE]={"NULL"};
  static char ftype_local[MAXLEN_FTYPE]={"NULL"};
  static unsigned int key,key_last;
  char *cp;
  uid_t uid_last;
  gid_t gid_last;

  /* start filter */

  if(flags & JUST_CORE) {
   if(!strncmp(fname,"core",4) || !strncmp(fname,"coredir.",8)) {
    printf("%s\n", fname);
   }
   return;
  }

  if(user_list_select) {
   for(j=0,i=0;i<nuid_select;i++) { if(st->st_uid == uid_select[i]) j=1; }
   if(j==0) return 0;
  }

  if(group_list_select) {
   for(j=0,i=0;i<ngid_select;i++) { if(st->st_gid == gid_select[i]) j=1; }
   if(j==0) return 0;
  }

  /* end filter */

  if(st->st_blksize > blocksize) {
   blocksize = st->st_blksize;
   if(flags & DEBUG) {
    printf("BLOCK: \tnew size = %d\n", blocksize);
   }
  }

  ftype = magic_file(my_magic,fname);
  strncpy(ftype_local,ftype,MAXLEN_FTYPE);

  cp=strchr(ftype_local, ',');
  if(cp) {
   *cp = '\0'; /* the comma rule for abbreviating types */
  }

  for(i=0;i<truncate_prefix_nentry;i++) {
   if(!strncmp(ftype_local, truncate_prefix[i], strlen(truncate_prefix[i]))) {
    sprintf(ftype_local, truncate_prefix[i]);
    break;
   }
  }

  for(i=0;i<truncate_abbriv_nentry;i++) {
   if(strstr(ftype_local, truncate_abbriv[i])) {
    sprintf(ftype_local, truncate_abbriv[i]);
    break;
   }
  }

/* now find the key */
  if(!strncmp(ftype_local,ftype_last,MAXLEN_FTYPE)) { 
/* ftypes are not well mixed, check the last one */
   key = key_last; 
  } else { 
/* compute the key and find the next matching or free bucket */
   key = (strhash(ftype_local))%ftypes_max;
   collisions = 0;
   while(t[key].nfile != 0 && strncmp(t[key].desc,ftype_local,MAXLEN_FTYPE)) {
    if(flags & DEBUG) {
    printf("COLLIDE: %d %d %s %s || %s \n", collisions, key, fname, t[key].desc, ftype_local);
    }
    key = ((key<ftypes_max-1)?(key+1):(0));
    collisions++; if(collisions==ftypes_max-1) {
     printf("ftypes_max exceeded. exitiing.\n");
     exit(1);
    }
   }
  }


  /* valid key, now it's on */

   if(t[key].nfile == 0) {
    strncpy(t[key].desc,ftype_local,MAXLEN_FTYPE);
    /* printf("%d %s \n", key, t[key].desc); */
    t[key].nfile = 1;
    t[key].nbyte = st->st_size;
    t[key].nblock = st->st_blocks;
    t[key].byte_min = st->st_size;
    t[key].byte_max = st->st_size;
    t[key].block_min = st->st_blocks;
    t[key].block_max = st->st_blocks;
    t[key].cage_min = st->st_ctime;
    t[key].cage_max = st->st_ctime;
    t[key].cage_avg = st->st_ctime;
    t[key].mage_min = st->st_mtime;
    t[key].mage_max = st->st_mtime;
    t[key].mage_avg = st->st_mtime;
/*
    printf("%s %d %d %d %d\n",
	 fname,st->st_size,st->st_blocks,st->st_ctime,st->st_mtime);
*/
    strncpy(t[key].cage_minf,fname,MAXLEN_FNAME);
    strncpy(t[key].cage_maxf,fname,MAXLEN_FNAME);
    strncpy(t[key].mage_minf,fname,MAXLEN_FNAME);
    strncpy(t[key].mage_maxf,fname,MAXLEN_FNAME);
    strncpy(t[key].byte_minf,fname,MAXLEN_FNAME);
    strncpy(t[key].byte_maxf,fname,MAXLEN_FNAME);
   } else {
    t[key].nfile ++;
    t[key].nbyte += st->st_size;
    t[key].nblock += st->st_blocks;
    t[key].cage_avg += st->st_ctime;
    t[key].mage_avg += st->st_mtime;
    if(st->st_ctime < t[key].cage_min) {
     t[key].cage_min = st->st_ctime;
     strncpy(t[key].cage_minf,fname,MAXLEN_FNAME);
    }
    if(st->st_ctime > t[key].cage_max) {
     t[key].cage_max = st->st_ctime;
     strncpy(t[key].cage_maxf,fname,MAXLEN_FNAME);
    }
    if(st->st_mtime < t[key].mage_min) {
     t[key].mage_min = st->st_mtime;
     strncpy(t[key].mage_minf,fname,MAXLEN_FNAME);
    }
    if(st->st_mtime > t[key].mage_max) {
     t[key].mage_max = st->st_mtime;
     strncpy(t[key].mage_maxf,fname,MAXLEN_FNAME);
    }
    if(st->st_size > t[key].byte_max) {
     t[key].byte_max = st->st_size;
     strncpy(t[key].byte_maxf,fname,MAXLEN_FNAME);
    }
    if(st->st_size < t[key].byte_min) {
     t[key].byte_min = st->st_size;
     strncpy(t[key].byte_minf,fname,MAXLEN_FNAME);
    }
    if(st->st_blocks > t[key].block_max) {
     t[key].block_max = st->st_blocks;
     strncpy(t[key].block_maxf,fname,MAXLEN_FNAME);
    }
    if(st->st_blocks < t[key].block_min) {
     t[key].block_min = st->st_blocks;
     strncpy(t[key].block_minf,fname,MAXLEN_FNAME);
    }
     
   }

  if(flags & DEBUG) {
   printf("FILE: %10d %s %ld %ld %ld %ld\n",
	 key,fname,st->st_size,t[key].nbyte,t[key].byte_min,t[key].byte_max);
  }

  if(st->st_uid != uid_last) {
   pwent = getpwuid(st->st_uid);
   if(pwent) {
    sprintf(tag,",%s,",pwent->pw_name);
    if(!strstr(user_list_found,tag)) {
     strcat(user_list_found,tag+1);
    }
   }
  }

  if(st->st_gid != gid_last) {
   grent = getgrgid(st->st_gid);
   if(grent) {
    sprintf(tag,",%s,",grent->gr_name);
    if(!strstr(group_list_found,tag)) {
     strcat(group_list_found,tag+1);
    }
   }
  }

  strncpy(ftype_last,ftype_local,MAXLEN_FTYPE);
  key_last = key;
  uid_last = st->st_uid;
  gid_last = st->st_gid;

  return 0;
  /* C=T */
}

unsigned int strhash(const char *s) {
 unsigned int sum = 0;
 int i,j,l;
 int p = sizeof(unsigned int)/sizeof(char);

 l = strlen(s); 
 j = 128;
 for(i=0;i<l;i++) {
/*
  sum += (int)s[i];
  sum ^= s[i] << (8*(p-1-(i%p)));
  sum += (((int)s[i])*sum%2)%j;
*/
  sum = (sum<<4)^(sum>>28)^s[i];
 }
 return sum;
}

int files_report_txt(void) { 

 int i,j,k;
 char hage[20], hage2[20];
 char hbyte[20], hbyte2[20];
 char hnum[20];
 char *cp1, *cp2;

 user_list_found[strlen(user_list_found)-1] = '\0';
 group_list_found[strlen(group_list_found)-1] = '\0';

 printf("#############################################################################\n");
 printf("# root   : %s \n", files_root_realpath);
/*
 cp1 = user_list_found+1;
 cp2 = group_list_found+1;
 while(strlen(cp1) > 0 || strlen(cp2) > 0) {
*/
  printf("# users  : %-30.30s   groups: %-26.26s\n",
	user_list_found+1,
	group_list_found+1);
/*
 }
*/
 human_byte(hbyte,T.nbyte);
 printf("# data   :  %8.8s                        files : %10lld\n", hbyte,T.nfile);
 human_byte(hbyte,T.byte_max);
 human_byte(hbyte2,T.byte_min);
 printf("# largest:  %8.8s                      smallest:   %8.8s\n",hbyte,hbyte2 );
 human_age(hage,files_start_t-T.cage_min);
 human_age(hage2,files_start_t-T.cage_max);
 printf("# oldest :  %8.8s                        newest:    %8.8s \n", hage, hage2);


 printf("#\n");
 printf("# filetype                       [files]  [bytes]  [blocks]  <create>  <mod>\n");
 for(i=0;i<ftypes_num;i++) {
  j = sort_by[i];
  human_byte(hbyte,t[j].nbyte);
  human_byte(hbyte2,t[j].nblock*512);
  human_age(hage,files_start_t-t[j].cage_avg);
  human_age(hage2,files_start_t-t[j].mage_avg);
  printf("# %-26.26s %10lld %8.8s  %8.8s %8.8s %8.8s\n",
	 t[j].desc, t[j].nfile,hbyte,hbyte2,hage2,hage);
 }

 printf("#############################################################################\n");
}

int files_report_xml(void) { 
 int i,j,k;
 printf("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
 printf("<files root=\"%s\" time_start=\"%d\" >\n", 
	files_root_realpath, files_start_t);
 for(i=0;i<ftypes_num;i++) {
  j = sort_by[i];
  printf("<ftype desc=\"%s\" nfile=\"%lld\" nbyte=\"%lld\" nblock=\"%lld\">\n",
	  t[j].desc,
	  t[j].nfile,
	  t[j].nbyte,
	  t[j].nblock);
printf(
"<byte min=\"%d\" max=\"%d\" avg=\"%.5e\" minf=\"%s\" maxf=\"%s\" />\n", 
	t[j].byte_min, t[j].byte_max, t[j].byte_avg,
        t[j].byte_minf, t[j].byte_maxf);
printf(
"<block min=\"%d\" max=\"%d\" avg=\"%.5e\" minf=\"%s\" maxf=\"%s\" />\n", 
	t[j].block_min, t[j].block_max, t[j].block_avg,
        t[j].block_minf, t[j].block_maxf);
printf(
"<cage min=\"%d\" max=\"%d\" avg=\"%.5e\" minf=\"%s\" maxf=\"%s\" />\n", 
	t[j].cage_min, t[j].cage_max, t[j].cage_avg,
	t[j].cage_minf, t[j].cage_maxf);
printf(
"<mage min=\"%d\" max=\"%d\" avg=\"%.5e\" minf=\"%s\" maxf=\"%s\" />\n", 
	t[j].mage_min, t[j].mage_max, t[j].mage_avg,
	t[j].mage_minf, t[j].mage_maxf);
  printf("</ftype>\n");
 }
 printf("</files>\n");
}

int human_fname(char *afname, char *fname) {
 if(strlen(fname) > MAXLEN_FNAME) {
  strncpy(afname,fname,MAXLEN_FNAME/2-3);
  strcat(afname,"...");
  strncpy(afname,fname+strlen(fname)-(MAXLEN_FNAME/2-3),MAXLEN_FNAME/2-3);
 } else {
  strncpy(afname,fname,MAXLEN_FNAME);
 }
 return 0;
}


int human_byte(char *sx, double x) {

 if(abs(x) < 1024) {
  sprintf(sx, "%5d B", x);
 } else if(x< 1024.0*1024.0) {
  sprintf(sx, "%5.1f KB", x/(1024.0));
 } else if(x < 1024.0*1024.0*1024.0) {
  sprintf(sx, "%5.1f MB", x/(1024.0*1024.0));
 } else if(x < 1024.0*1024.0*1024.0*1024.0) {
  sprintf(sx, "%5.1f GB", x/(1024.0*1024.0*1024.0));
 } else if(x < 1024.0*1024.0*1024.0*1024.0*1024.0) {
  sprintf(sx, "%5.1f TB", x/(1024.0*1024.0*1024.0*1024.0));
 }

 return 0;
}


int human_age(char *sx, double x) {
 if(fabs(x) < 60) {
  sprintf(sx, "%5d s ", (int)x);
 } else if(fabs(x) < 60*60) {
  sprintf(sx, "%5.1f mn", (x/(60.0)));
 } else if(fabs(x) < 24*60*60) {
  sprintf(sx, "%5.1f hr", (x/(60*60.0)));
 } else if(fabs(x) < 30*24*60*60) {
  sprintf(sx, "%5.1f dy", (x/(24*60*60.0)));
 } else if(fabs(x) < 365*24*60*60) {
  sprintf(sx, "%5.1f mo", (x/(30*24*60*60.0)));
 } else {
  sprintf(sx, "%5.1f yr", (x/(365*24*60*60.0)));
 }
 return 0;
}



