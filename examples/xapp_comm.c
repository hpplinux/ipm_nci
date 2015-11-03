#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
 
main(argc, argv)
 
int                     argc;
char                    *argv[];
 
{
	int		i,j, you, eyou;
        int             rank, erank, size, esize, data, *iranks;
        MPI_Status      status;
        MPI_Comm        libcomm;
        MPI_Group	iwgroup, libgroup;
 
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        iranks = (int *)malloc((size_t)(sizeof(int)*size));

        for(i=j=0; i < size; i+=1) {
         if(i%2 == 0) {
          iranks[j] = i;
          j++;
         }
        }

        you = (rank%2)?(rank-1):(rank+1);;
 
        if (size < 2) {
                if ( ! rank) printf("communicator: must have two processes\n");
                MPI_Finalize();
                exit(0);
        }
 
        erank = esize = -1;

        if(0) {
         MPI_Comm_dup(MPI_COMM_WORLD, &libcomm);
        } else {
         MPI_Comm_group(MPI_COMM_WORLD, &iwgroup);
         MPI_Group_incl(iwgroup,size/2,iranks, &libgroup);
         MPI_Comm_create(MPI_COMM_WORLD,libgroup,&libcomm);
         if(rank%2 == 0) {
          MPI_Comm_rank(libcomm, &erank);
          MPI_Comm_size(libcomm, &esize);
          eyou = (erank%2)?(erank-1):(erank+1);;
         }
        }
 
        if(0) {
        if (rank%2==0) {
                data = 12345;
                MPI_Send(&data, 1, MPI_INT, you, 5, MPI_COMM_WORLD);
        } else {
                MPI_Recv(&data, 1, MPI_INT, you, 5, MPI_COMM_WORLD, &status);
                printf("%d received data = %d\n", rank, data);
        }
        }
 
        if(erank >= 0) {
         if (erank%2==0) {
                data = 6789;
                MPI_Send(&data, 1, MPI_INT, eyou, 5, libcomm);
         } else {
                MPI_Recv(&data, 1, MPI_INT, eyou, 5, libcomm, &status);
                printf("%d %d received libcomm data = %d\n", rank, erank,data);
         }
        MPI_Comm_free(&libcomm);
        }

        MPI_Finalize();
        exit(0);
}
