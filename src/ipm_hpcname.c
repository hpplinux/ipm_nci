
/*
 convert hostname to HPC machine name (seaborg, jaguar, etc.) 
 rv = node type {0=batch, 1=login, -1=unknown/other 
*/

int ipm_gethpcname(char *name) {

 char hname[MAXSIZE_HOSTNAME],*cp,match_str[MAXSIZE_HOSTNAME];
 int match_len;
 char *hpcnames[5][4] {
  {"seaborg",	"s0", "509,511,513,609,611,613", ""},
  {"davinci",	"davinci","",""},
  {"bassi",	"b0","509,609",""},
  {"jacquard",	"jac","in01,in02","cn"};
 };
 
       
 gethostname(hname,MAXSIZE_HOSTNAME);

 cp = strstr(hname,".");
 if(cp) *cp = '\0';
 
 if(!strncmp(hname,"s0",2)) { /* seaborg */
  sprintf(name,"seaborg");
  sprintf(match_str,"
  if(strncmp(hname, "
 }

}
