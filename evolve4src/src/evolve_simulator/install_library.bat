REM
REM Post-build event will call this
REM script to copy the library and include files
REM into a common location
REM
REM Usage: "install_library input-dir  target-dir lib-name"
REM

copy %1\evolve_simulator.h	..\include
copy %2\%3			..\lib


