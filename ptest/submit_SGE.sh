#!/bin/bash               
#$ -V                     # Inherit the submission environment
#$ -cwd                   # Start job in  submission directory
#$ -N IPM_ptest           # Job Name
#$ -j y                   # combine stderr & stdout into stdout    
#$ -o $JOB_NAME.o$JOB_ID  # Name of the output file (eg. myMPI.oJobID)
#$ -pe 16way 32           # Requests 16 cores/node, 32 cores total
#$ -q normal              # Queue name
#$ -l h_rt=00:15:00       # Run time (hh:mm:ss) - 1.5 hours
## -M myEmailAddress      # Email notification address (UNCOMMENT)
## -m be                  # Email at Begin/End of job  (UNCOMMENT)

set -x                    #{echo cmds, use "set echo" in csh}
ibrun ./a.out             # Run the MPI executable named "a.out"

unset MPIRUN
export MPIRUN="ibrun "

./runtests.sh > IPM_ptest.out IPM_ptest.err