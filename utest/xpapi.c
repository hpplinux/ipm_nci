#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <papi.h>

int main(int argc, char **argv)
{
   int i;
   int retval;
   int print_avail_only = 0;
   PAPI_event_info_t info;
   const PAPI_hw_info_t *hwinfo = NULL;

   for (i = 0; i < argc; i++)
      if (argv[i]) {
         if (strstr(argv[i], "-a"))
            print_avail_only = PAPI_PRESET_ENUM_AVAIL;
      }

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   hwinfo = PAPI_get_hardware_info();

   if (1) {
      printf("Test case 8: Available events and hardware information.\n");
      printf
          ("-------------------------------------------------------------------------\n");
      printf("Vendor string and code   : %s (%d)\n", hwinfo->vendor_string,
             hwinfo->vendor);
      printf("Model string and code    : %s (%d)\n", hwinfo->model_string, hwinfo->model);
      printf("CPU Revision             : %f\n", hwinfo->revision);
      printf("CPU Megahertz            : %f\n", hwinfo->mhz);
      printf("CPU's in this Node       : %d\n", hwinfo->ncpu);
      printf("Nodes in this System     : %d\n", hwinfo->nnodes);
      printf("Total CPU's              : %d\n", hwinfo->totalcpus);
      printf("Number Hardware Counters : %d\n", PAPI_get_opt(PAPI_MAX_HWCTRS, NULL));
      printf("Max Multiplex Counters   : %d\n", PAPI_MPX_DEF_DEG);
      printf
          ("-------------------------------------------------------------------------\n");

      printf("The following correspond to fields in the PAPI_event_info_t structure.\n");
      
      printf("Symbol\tEvent Code\tCount |Short Description| |Long Description|\n");
      printf("The count field indicates whether it is a) available (count >= 1) and b) derived (count > 1)\n");

      i = PAPI_PRESET_MASK;
      do {
         if (PAPI_get_event_info(i, &info) == PAPI_OK) 
	   {
	     printf("%s\t0x%x\t%d |%s| |%s|\n",
		    info.symbol,
		    info.event_code,
		    info.count,
		    info.short_descr,
		    info.long_descr);
	   }
      } while (PAPI_enum_event(&i, print_avail_only) == PAPI_OK);
      printf
          ("-------------------------------------------------------------------------\n");
   }

   exit(1);
}
