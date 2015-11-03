#PBS -S /usr/bin/tcsh
#PBS -N IPM_ptest
#PBS -l mppwidth=4
#PBS -l mppnppn=1
#PBS -l walltime=00:15:00
#PBS -j eo
#PBS -V

if [ "$PBS_O_WORKDIR" != "" ] ;then  cd $PBS_O_WORKDIR; fi

unset MPIRUN
export MPIRUN="aprun -n 4 -N 1"

./runtests.sh