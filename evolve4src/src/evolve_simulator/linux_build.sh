#!/bin/sh
#
# Build 'evolve_simulator.a' library
#
# To use this file just shove it into a shell like so:
#
#	sh ./linux_build.sh
#
# Nothing fancy here.. it just GCC's each file in the evolve_sim module and
# shoves it into a static library etc....
#
# We assume the preprocessor symbol "__linux__" is defined for us.
#
# For code that differs from windows look for the:
#
# #ifndef __linux__
#
# Use stdafx.h when possible to overcome any porting issues using
# macros, defines, hacks, and so on.
#
#
#


#OPTS="-g"
OPTS="-O -DNDEBUG"

ALL_SOURCE="	evolve_io_binary1.cpp	\
		evolve_io_binary2.cpp	\
		evolve_io_binary3.cpp	\
		evolve_io_binary4.cpp	\
		evolve_io_binary5.cpp	\
		evolve_io_binary6.cpp	\
		evolve_io_binary7.cpp	\
		evolve_io_binary8.cpp	\
		evolve_io_binary9.cpp	\
		evolve_io_binary.cpp	\
		evolve_io_ascii.cpp	\
		evolve_io.cpp		\
		keylist.cpp		\
		kforth_compiler.cpp	\
		kforth_execute.cpp	\
		kforth_mutate.cpp	\
		organism.cpp		\
		random.cpp		\
		spore.cpp		\
		player.cpp		\
		cell.cpp		\
		universe.cpp		\
		phascii.cpp"

ALL_OBJECT="	evolve_io_binary1.o	\
		evolve_io_binary2.o	\
		evolve_io_binary3.o	\
		evolve_io_binary4.o	\
		evolve_io_binary5.o	\
		evolve_io_binary6.o	\
		evolve_io_binary7.o	\
		evolve_io_binary8.o	\
		evolve_io_binary9.o	\
		evolve_io_binary.o	\
		evolve_io_ascii.o	\
		evolve_io.o		\
		keylist.o		\
		kforth_compiler.o	\
		kforth_execute.o	\
		kforth_mutate.o		\
		organism.o		\
		random.o		\
		spore.o			\
		player.o		\
		cell.o			\
		universe.o		\
		phascii.o"

for F in $ALL_SOURCE; do
	echo gcc $OPTS -c $F
	gcc $OPTS -c $F
done

echo ar r evolve_simulator.a $ALL_OBJECT
ar r evolve_simulator.a $ALL_OBJECT

echo ranlib evolve_simulator.a
ranlib evolve_simulator.a

echo cp evolve_simulator.a ../lib
cp evolve_simulator.a ../lib

echo cp evolve_simulator.h ../include
cp evolve_simulator.h ../include

