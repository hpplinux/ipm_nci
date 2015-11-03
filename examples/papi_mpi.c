#include <papi.h>
#include <mpi.h>
#include <math.h>
#include <stdio.h>

void handle_error(int i);

void handle_error(int i) {
 printf("%d\n",i);
}

int main(argc,argv)
int argc;
char *argv[];
{
  int done = 0, n, myid, numprocs, i, rc, retval, EventSet = PAPI_NULL;
  double PI25DT = 3.141592653589793238462643;
  double mypi, pi, h, sum, x, a;
  long_long values[1] = {(long_long) 0};

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  /*Initialize the PAPI library */
  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr, "PAPI library init error!\n");
    exit(1);
}

  /* Create an EventSet */
  if (PAPI_create_eventset(&EventSet) != PAPI_OK)
    handle_error(1);

/* Add Total Instructions Executed to our EventSet */
  if (PAPI_add_event(EventSet, PAPI_TOT_INS) != PAPI_OK)
    handle_error(1);

  /* Start counting */
  if (PAPI_start(EventSet) != PAPI_OK)
    handle_error(1);

  while (!done)
  {
    if (myid == 0) {
        printf("Enter the number of intervals: (0 quits) ");
        scanf("%d",&n);
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (n == 0) break;

    h   = 1.0 / (double) n;
    sum = 0.0;
    for (i = myid + 1; i <= n; i += numprocs) {
        x = h * ((double)i - 0.5);
        sum += 4.0 / (1.0 + x*x);
    }
    mypi = h * sum;

    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0,MPI_COMM_WORLD);

    if (myid == 0)
        printf("pi is approximately %.16f, Error is %.16f\n",
               pi, fabs(pi - PI25DT));
    }

   /* Read the counters */
   if (PAPI_read(EventSet, values) != PAPI_OK)
     handle_error(1);

   printf("After reading counters: %lld\n",values[0]);

   /* Start the counters */
   if (PAPI_stop(EventSet, values) != PAPI_OK)
     handle_error(1);

   printf("After stopping counters: %lld\n",values[0]);

   MPI_Finalize();
}

