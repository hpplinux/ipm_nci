#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

static int ipm_get_cmdline(char *cmdl, char *rpath) {
 int i, ii, fd, rv, blen;
 FILE *fh;
 char *cp,*pp,*up,cbuf[MAXSIZE_CMDLINE];
 struct stat st;
#ifdef OS_AIX
 psinfo_t  aixinfo;
#endif

#ifdef OS_LINUX
#define LINUX_CMDL_PROC
#endif

#ifdef OS_AIX
#define AIX_CMDL_PROC
#endif


 sprintf(cmdl, "unknown");

#ifdef LINUX_BGL
 pp = getenv("OLDPWD");
 if(pp) {
 }
#endif

#ifdef LINUX_CMDL_PROC
 cmdl[0] = '\0';
 fh = fopen("/proc/self/cmdline","r"); 
 if(fh) {
/*
 rv=fscanf(fh,"%s",cmdl);
 printf("rv %d cmdl %s\n", rv, cmdl);
 rv=fscanf(fh,"%s",cmdl);
 printf("rv %d cmdl %s\n", rv, cmdl);
*/
  ii = 0;
  fgets(cmdl,MAXSIZE_CMDLINE,fh);
  for(i=1;i<MAXSIZE_CMDLINE;i++)  {
   if(cmdl[i]==0 && ii == 0) {
    cmdl[i]=' '; ii = 1;
   } else if(cmdl[i]==0 && ii == 1) {
    break;
   } else {
    ii = 0;
   }
  }
  fclose(fh);
 } else {
  sprintf(cmdl, "unknown");
 }
#endif

#ifdef AIX_CMDL_PS
 sprintf(cbuf,"/usr/bin/ps  -p %d -o \"%%a\" | /usr/bin/tail -1", task.pid);
 fh = popen(cbuf,"r"); 
 if(fh) {
 fgets(cmdl,MAXSIZE_CMDLINE,fh);
 for(i=0;i<strlen(cmdl);i++) {
  if(cmdl[i] == '\n') {cmdl[i] = '\0'; break;}
 }
 pclose(fh);
 } else {
  sprintf(cmdl,"unknown");
 }
#endif

#ifdef AIX_CMDL_PROC
 sprintf(cbuf, "/proc/%d/psinfo", task.pid);
 fd = open(cbuf, O_RDONLY);
 if(fd < 0) {
  sprintf(cmdl, "unknown");
 } else {
  i = read(fd, &aixinfo, sizeof(aixinfo));
  if (i == sizeof(aixinfo)) {
   sprintf(cmdl, "%s", aixinfo.pr_psargs);
  } else {
   sprintf(cmdl, "unknown");
  }
  close(fd);
 }
#endif

/*
** determine real path
*/

#ifdef REALPATH_VIAPS
#define REALPATH_DONE
  if(strcmp("unknown", cmdl)) {
    cp = strchr(cmdl, ' '); 
    if(cp) *cp = 0; 		/* break/truncate to argv[0] except '\ ' */ 
    sprintf(cbuf,"/usr/bin/which %s", cmdl);
    
 if(task.flags & DEBUG) {
   printf("IPM: %d ipm_get_which(%s)(%s) \n", task.mpi_rank, cmdl); 
 }
    if(task.flags & DEBUG) {
     printf("IPM: %d ipm_get_cmdline cbufp %s \n", task.mpi_rank, cbuf); 
    }
    fh = popen(cbuf,"r"); 
    if(fh) {
    fgets(rpath,MAXSIZE_CMDLINE,fh);
    for(i=0;i<strlen(rpath);i++) {
     if(rpath[i] == '\n') { rpath[i] = '\0'; break; }
    }
    if(cp) *cp = ' ';  	/* fix the string */
    pclose(fh);
    } else {
    sprintf(rpath, "unknown");
    }
   } else {
    sprintf(rpath, "unknown");
   }

#endif

#ifdef REALPATH_VIAENV 
#define REALPATH_DONE
  if(strcmp("unknown", cmdl)) {
   blen = -1;
   for(i=0;i<strlen(cmdl);i++) {
    if(cmdl[i] == ' ') {blen = i; break;}
   }
   if(blen < 0) { blen = strlen(cmdl); /* no space in name */}

   /* already full path --> strncpy */
   if(cmdl[0] == '/') {
    strncpy(rpath,cmdl,blen);
    if(task.flags & DEBUG) {
     printf("IPM: %d ipm_get_cmdline absolute, is %s in %s (%d) \n",
      task.mpi_rank, cmdl, cp, stat(rpath,&st));
    }
   }

   /* relative path I --> strncat */
   if(cmdl[0] == '.') {
    pp=getenv("PWD");
    strncpy(rpath,pp,strlen(pp));
    strncat(rpath,"/",1);
    strncat(rpath,cmdl,blen);
   }

   if(cmdl[0] != '.' && cmdl[0] != '/') {
   /* relative path II --> walk,strncat */
    pp=getenv("PATH"); 
    cp = strtok_r(pp,":",&up); 
    while(cp) {
     sprintf(rpath,"%s/",cp);
     strncat(rpath,cmdl,blen);
     if(task.flags & DEBUG) {
      printf("IPM: %d ipm_get_cmdline walk, is %s in %s (%d) \n",
        task.mpi_rank, cmdl, cp, stat(rpath,&st));
     }
     if(!stat(rpath,&st)) {
      cp = NULL;
     } else {
      cp = strtok_r(NULL,":",&up); 
      if(!cp) {
       sprintf(rpath, "unknown");
      }
     }
    }
   }
  } else {
   sprintf(rpath, "unknown");
  }

  if(stat(rpath,&st)) {
   sprintf(rpath, "unknown");
  }

#endif

#ifndef REALPATH_DONE
  if(strcmp("unknown", cmdl)) {
   blen = -1;
   for(i=0;i<strlen(cmdl);i++) {
    if(cmdl[i] == ' ') {blen = i; break;}
   }
   if(blen < 0) { blen = strlen(cmdl); /* no space in name */ }
   for (i=0;i<MAXSIZE_CMDLINE;i++){
   cbuf[i] = (char)0;
   }
   strncpy(cbuf,cmdl,blen);
    cp = realpath(cbuf,rpath);
    if(!cp) {
    sprintf(rpath, "unknown");
   }
  } else {
   sprintf(rpath, "unknown");
  }

#endif

 if(task.flags & DEBUG) {
   printf("IPM: %d ipm_get_cmdline(%s)(%s) \n", task.mpi_rank, cmdl, rpath); 
 }

 return 0;
}

