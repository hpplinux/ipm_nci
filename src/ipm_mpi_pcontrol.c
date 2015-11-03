/*
MPI_Pcontrol(char *cmd, char *msg, double *val, IPM_COUNT_TYPE *cnt) {
}

*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MAXSIZE_PCMD 80

int ipm_control(const int *ctl, char *cmd, void *data);

int MPI_Pcontrol(const int ctl, ...) {
 va_list ap;
 char *cmd;
 void *data;

 va_start(ap, ctl);
 cmd = va_arg(ap, char *);
 data = va_arg(ap, void *);
 va_end(ap);

 if(strlen(cmd) > MAXSIZE_PCMD) {
  printf("IPM: %d pcontrol region label too long (%s) :  \\n",task.mpi_rank, cmd);
  return 0;
 }

 ipm_control(&ctl,cmd,data);

 return 1;
}


#ifdef WRAP_FORTRAN

void MPI_PCONTROL_F(const int *ctl, char *cmd, char *data) {

 ipm_control(ctl, cmd, data);

 return;
}

#endif

int ipm_control(const int *ctl, char *cmd, void *data) {

 int narg,vctl,got_action=0;
 char *cp,*cpb,*cpe,*cpa;
 char *action=NULL,*arg[5]={NULL,NULL,NULL,NULL,NULL};
 char lcmd[MAXSIZE_PCMD];

 if(task.flags & DEBUG) {
  if(ctl && cmd) {
   printf("IPM: %d ipm_control enter %d %s\n", task.mpi_rank, *ctl, cmd);
  } else {
   printf("IPM: %d ipm_control enter ? ?\n", task.mpi_rank);
  }
 }

 if(ctl) {
  vctl = *ctl;
 } else {
  vctl = 0;
 }

 if (vctl==0 && cmd) {
   if(strlen(cmd) > MAXSIZE_PCMD) {
        printf("IPM: %d ERROR ipm_control command too long\n",task.mpi_rank);
   }
   strncpy(lcmd, cmd, MAXSIZE_PCMD);
   cpb = strchr(lcmd,'(');
   cpe = strrchr(lcmd,')');

   if(cpb && cpe && cpb < cpe) { /* has action(?) syntax */
    action = lcmd;
    *cpb = '\0'; /* null out parenthesis */
    *cpe = '\0'; /* null out parenthesis */
     cpa = cpb+1;   /* start of args */
     narg = 0;
     if(cpe-cpb > 1) {
      arg[0] = cpa; narg = 1;
      for(cp=cpa;cp<cpe;cp++) {
      if(*cp == ',') { 
       if(narg >5) {
        printf("IPM: %d ERROR ipm_control too many arguments\n",task.mpi_rank);
       }
       arg[narg] = cpa; narg++;
      }
     }
    }

  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_control action=%s ", task.mpi_rank , action);
   if(arg[0]) printf("arg[0]=%s ", arg[0]);
   if(arg[1]) printf("arg[1]=%s ", arg[1]);
   if(arg[2]) printf("arg[2]=%s ", arg[2]);
   if(arg[3]) printf("arg[3]=%s ", arg[3]);
   if(arg[4]) printf("arg[4]=%s ", arg[4]);
   printf("\n");
  }

  /* check for known vocabulary of actions */
  got_action = 0;
  if(!got_action && !strcmp(action,"enter_region")) {
    ipm_region(1,arg[0]); got_action=1;
  }
  if(!got_action && !strcmp(action,"exit_region")) {
    ipm_region(-1,arg[0]); got_action=1;
  }

  if(!got_action && !strcmp(action,"get_region")) {
    strncpy((char *)data, task.region_label[task.region_current],MAXSIZE_LABEL);
    got_action=1;
  }
  if(!got_action && !strcmp(action,"get_ipmstatus")) {
    if(task.flags & IPM_INITIALIZED) {
     strncpy((char *)data, "INITIALIZED",MAXSIZE_LABEL);
    }
  }

  }
 } 

 if(got_action == 0) {
  if (vctl==0) { ipm_event(cmd);}
  if (vctl==1 || vctl==-1) { ipm_region(vctl,cmd);}
 }

  if(task.flags & DEBUG) {
   printf("IPM: %d ipm_control exit\n", task.mpi_rank);
  }
 return 1;
}

