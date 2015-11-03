/* This file performs the following test: start, stop and timer functionality

   - It attempts to use the following two counters. It may use less depending on
     hardware counter resource limitations. These are counted in the default counting
     domain and default granularity, depending on the platform. Usually this is 
     the user domain (PAPI_DOM_USER) and thread context (PAPI_GRN_THR).
     + PAPI_FP_INS
     + PAPI_TOT_CYC
   - Get us.
   - Start counters
   - Do flops
   - Stop and read counters
   - Get us.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <papi.h>

#define SUCCESS 1
#define FAILURE 0
#define NUM_THREADS 4
#define NUM_ITERS  1000000
#define NUM_FLOPS  20000000
#define NUM_READS 20000
#define THRESHOLD   1000000
#define L1_MISS_BUFFER_SIZE_INTS 128*1024
#define CACHE_FLUSH_BUFFER_SIZE_INTS 4*1024*1024
#define TOLERANCE   .2
#define OVR_TOLERANCE .75
#define MPX_TOLERANCE .20
#define MAX_TO_ADD 9

volatile double a = 0.5, b = 2.2;


void do_flops(int n)
{
   int i;
   double c = 0.11;
                                                                                
   for (i = 0; i < n; i++) {
      c += a * b;
   }
}


int main(int argc, char **argv)
{
   int retval, num_tests = 2, eventcnt, events[2], i, tmp;
   int EventSet1 = PAPI_NULL, EventSet2 = PAPI_NULL;
   int PAPI_event;
   long_long values1[2], values2[2];
   long_long elapsed_cyc;
   char event_name[PAPI_MAX_STR_LEN], add_event_str[PAPI_MAX_STR_LEN];


   retval = PAPI_library_init(PAPI_VER_CURRENT);

      retval = PAPI_set_debug(PAPI_VERB_ECONT);

   /* query and set up the right instruction to monitor */
   if (PAPI_query_event(PAPI_FP_OPS) == PAPI_OK)
      PAPI_event = PAPI_FP_OPS;
   else
      PAPI_event = PAPI_TOT_INS;

   retval = PAPI_event_code_to_name(PAPI_event, event_name);
   sprintf(add_event_str, "PAPI_add_event[%s]", event_name);

   retval = PAPI_create_eventset(&EventSet1);

   /* Add the events */

   retval = PAPI_add_event(EventSet1, PAPI_event);

   retval = PAPI_add_event(EventSet1, PAPI_TOT_CYC);

   /* Add them reversed to EventSet2 */

   retval = PAPI_create_eventset(&EventSet2);

   eventcnt = 2;
   retval = PAPI_list_events(EventSet1, events, &eventcnt);

   for (i = eventcnt - 1; i >= 0; i--) {
      retval = PAPI_event_code_to_name(events[i], event_name);

      retval = PAPI_add_event(EventSet2, events[i]);
   }

   elapsed_cyc = PAPI_get_real_cyc();

   retval = PAPI_start(EventSet1);

   do_flops(NUM_FLOPS);

   retval = PAPI_stop(EventSet1, values1);

   retval = PAPI_start(EventSet2);

   do_flops(NUM_FLOPS);

   retval = PAPI_stop(EventSet2, values2);


   elapsed_cyc = PAPI_get_real_cyc() - elapsed_cyc;

   retval = PAPI_cleanup_eventset(EventSet1);   /* JT */

   retval = PAPI_destroy_eventset(&EventSet1);

   retval = PAPI_cleanup_eventset(EventSet2);   /* JT */

   retval = PAPI_destroy_eventset(&EventSet2);

      printf("Test case 0: start, stop.\n");
      printf("-----------------------------------------------\n");
      tmp = PAPI_get_opt(PAPI_DEFDOM, NULL);
      tmp = PAPI_get_opt(PAPI_DEFGRN, NULL);
      printf("Using %d iterations of c += a*b\n", NUM_FLOPS);
      printf
          ("-------------------------------------------------------------------------\n");

      printf("Test type    : \t           1\t           2\n");

      printf("%ld %ld\n", values1[0], values2[1]);
      printf("%d %d\n", "PAPI_TOT_INS : \t", values1[1], values2[0]);
      printf("%ld\n", "Real cycles  : \t", elapsed_cyc);

      printf
          ("-------------------------------------------------------------------------\n");

      printf("Verification: none\n");
   exit(1);
}
