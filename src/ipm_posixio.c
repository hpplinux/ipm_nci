#include <stdio.h>
#include <stdlib.h>
#define GNU_SOURCE
#include <dlfcn.h>

void *malloc(size_t size) {

  static void * (*pmalloc)();
  void *handle;
  char *error;


  if(!pmalloc) {
/*
   handle = dlopen ("libc.so", RTLD_LAZY);
   if (!handle) {
    fprintf (stderr, "%s\n", dlerror());
    exit(1);
   }
   dlerror(); 
   *(void **) (&pmalloc) = dlsym(handle, "malloc");
   *(void **) (&pmalloc) = dlsym(RTLD_NEXT, "malloc");
*/
   *(void **) (&pmalloc) = dlsym(((void *) -1l), "malloc");
   if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    exit(1);
   }
  }

  printf("malloc(%d) is called\n", size);     
  return(pmalloc(size));
}



