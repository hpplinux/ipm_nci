#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
 char region_name[4096];

 MPI_Pcontrol(1,"get_region();",(void *)region_name); 
 return 0;
}


