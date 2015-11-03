
int callsite(char *tag) {
 unsigned long *ebp;
 asm volatile ("movl %%ebp,%0" : "=m" (ebp));
 printf("%-20.20s ", tag);
 while( *(unsigned long *)(ebp) ) {
  printf("0x%x<-", *((unsigned long *)(ebp+1)) );
  ebp = *(unsigned long *)(ebp);
 }
 printf("\n");
 return 0 ;
}

