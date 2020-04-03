#!/bin/sh
#$ -V
#$ -cwd
#$ -S /bin/bash
#$ -N gaussian
#$ -o $JOB_NAME.o$JOB_ID
#$ -e $JOB_NAME.e$JOB_ID
#$ -q omni
#$ -pe sm 36
#$ -l h_vmem=5.3G
#$ -l h_rt=48:00:00
#$ -P quanah

module load gnu7/7.3.0

echo "Testing example_73.exe ... created by makefile"
./gaussian.exe -S 1 -N 100000000 -T 1
./gaussian.exe -S 1 -N 100000000 -T 2
./gaussian.exe -S 1 -N 100000000 -T 4
./gaussian.exe -S 1 -N 100000000 -T 8
./gaussian.exe -S 1 -N 100000000 -T 16
./gaussian.exe -S 1 -N 100000000 -T 32 
echo -e "###\n"
