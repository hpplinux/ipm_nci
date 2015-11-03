#include <mpi.h>
#include <stdio.h>

#define CLOCK(t) t = MPI_Wtime()

MPI_Status status;

int main(int argc , char *argv[]) {

 int i, nprocs, myid;
 int data;
 double t1, t2;
 double elapse;

 MPI_Init(&argc, &argv);
 MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
 MPI_Comm_rank(MPI_COMM_WORLD, &myid);

 data = myid + 10;

 MPI_Pcontrol(1, "good");

 CLOCK(t1);
 for (i = 0; i < 2; i++)
 if (myid == 0) {
   printf("0 is sending message to 1, data is %d\n", data); 
   MPI_Send(&data, 1, MPI_INT, 1, 100, MPI_COMM_WORLD);
   printf("0 finished sending to 1\n");
 }
 else {
   printf("1 is receiving message from 0\n");
   MPI_Recv(&data, 1, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);
   printf("1 finished receiving from 0, data is %d\n", data);
 }
 CLOCK(t2);
 elapse = t2 - t1;
 printf("elapsed time is %f\n", elapse);

 MPI_Pcontrol(-1, "good");

 MPI_Finalize();
 return 0;
}

