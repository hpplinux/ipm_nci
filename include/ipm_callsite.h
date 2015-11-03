
#ifndef IPM_CALLSITE_H_INCLUDED
#define IPM_CALLSITE_H_INCLUDED

#ifdef LINUX_X86
#define GOT_KEY
#define IPM_GET_CALLSITE(s) __asm__ volatile ("movl %%eip,%0" : "=m" (s))
#define IPM_GET_CALLER(s) *(unsigned long *)(ebp);
#endif 

#ifndef GOT_KEY
#define IPM_GET_CALLSITE(s) 
#define IPM_GET_CALLER(s) -1
int callsite() {};
#endif

#undef GOT_KEY
#endif /* IPM_CALLSITE_H_INCLUDED */
