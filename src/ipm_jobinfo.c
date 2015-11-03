/***************************************/
/* Batch queue specific stuff          */
/***************************************/

#include <sys/types.h>
#ifndef IPM_DISABLE_PWENT
#include <pwd.h>
#include <grp.h>
#endif


static int ipm_get_jobdata(char *jid, char *jun, char *jgn) {
#ifndef IPM_DISABLE_PWENT
 struct passwd            *pwent;
 struct group             *grent;
#endif
 char *cp=(char)0;

 cp = getenv("PBS_JOBID"); 
 if(!cp) cp = getenv("LOADL_STEP_ID");
 if(!cp) cp = getenv("SLURM_JOBID");
 if(!cp) cp = getenv("JOB_ID");
 if(!cp) cp = getenv("LSB_JOBID");
 if(!cp) {
  sprintf(jid, "unknown"); 
 } else {
  sprintf(jid, "%s", cp); 
 }

#ifndef IPM_DISABLE_PWENT
 pwent = getpwuid(getuid());
 grent = getgrgid(pwent->pw_gid);
 if(pwent) strncpy(jun,pwent->pw_name,MAXSIZE_USERNAME);
 if(grent) strncpy(jgn,grent->gr_name,MAXSIZE_USERNAME);
#else
#if defined (LINUX_BGL) || defined (LINUX_XT3) || defined (LINUX_XT4) || defined (LINUX_XT5) 
cp = getenv("USER");
if (cp) {
sprintf(jun, "%s", cp);
 }
cp = getenv("GROUP");
if (cp) {
sprintf(jgn, "%s", cp);
 }
#else
 strncpy(jun,"unknown",MAXSIZE_USERNAME);
 strncpy(jgn,"unknown",MAXSIZE_USERNAME);
#endif
#endif
#ifdef OS_AIX
 cp = getenv("LOADL_STEP_ACCT");
 if(cp) {
  strncpy(jgn,cp,MAXSIZE_USERNAME);
 }
#endif


 return 0;
}

