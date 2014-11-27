#!/bin/sh
#
# Build the 'evolve_batch' program in linux.
#
# Assumes that 'evolve_simulator' was already built. Go there
# first and run linux_build.sh in that directory. Then come back
# here and run:
#
#	sh ./linux_build.sh
#
# You will end up with a nice little executable called 'evolve_batch'.
#
#
#
#

#OPTS="-g"
OPTS="-O -DNDEBUG"

echo g++ $OPTS evolve_batch.cpp -I ../include ../lib/evolve_simulator.a -o evolve_batch
g++ $OPTS evolve_batch.cpp -I ../include ../lib/evolve_simulator.a -o evolve_batch

echo cp evolve_batch ../bin
cp evolve_batch ../bin

