#include <stdio.h>
#include <stdlib.h>
#include <papi.h>

#define NUM_FLOPS 1023
volatile double a = 0.5, b = 2.2;
 
void do_flops(int n)
{
   int i;
   double c = 0.11;
                                                                                
   for (i = 0; i < n; i++) {
      c += a * b;
   }
}
 


int main(int argc, char **argv) {
   int retval, num_tests = 1, tmp;
   int EventSet1;
   int PAPI_event, mask1;
   int num_events1;
   long_long **values;
   long_long elapsed_us, elapsed_cyc;
   char event_name[PAPI_MAX_STR_LEN], add_event_str[PAPI_MAX_STR_LEN];
   const PAPI_hw_info_t *hw_info;

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   retval = PAPI_set_debug(PAPI_VERB_ECONT);
   hw_info = PAPI_get_hardware_info();
   EventSet1 = add_two_events(&num_events1, &PAPI_event, hw_info, &mask1);
   retval = PAPI_event_code_to_name(PAPI_event, event_name);
   elapsed_us = PAPI_get_real_usec();
   elapsed_cyc = PAPI_get_real_cyc();
   retval = PAPI_start(EventSet1);
   do_flops(NUM_FLOPS);
   retval = PAPI_stop(EventSet1, values[0]);
   retval = PAPI_start(EventSet1);
   do_flops(NUM_FLOPS);
   retval = PAPI_stop(EventSet1, values[0]);
   elapsed_us = PAPI_get_real_usec() - elapsed_us;
   elapsed_cyc = PAPI_get_real_cyc() - elapsed_cyc;
   remove_test_events(&EventSet1, mask1);

   exit(1);
}
