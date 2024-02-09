# Attention Circuits Control Laboratory - NeuroLoop project
# Makefile - Test programs.
# Written by Christopher Thomas.
# Copyright (c) 2020 by Vanderbilt University. This work is licensed under
# the Creative Commons Attribution 4.0 International License.


#
# Configuration.


# Compiler flags.
# NOTE - We need to pull in various source files too!

NLOOPSRCS=	\
	../nloop-fileio.cpp

CFLAGS=-std=c++11 -I.. $(NLOOPSRCS)

# NOTE: Other flags of interest:
#   NLOOP_SIGN_SAFE_SHIFT - Manually impelments arithmetic right-shifting.
#   NLOOP_KLUDGE_LIMITS - Use macros instead of <limits>'s numeric_limits<T>.

LIMITKLUDGE=-DNLOOP_KLUDGE_LIMITS -Wno-shift-count-overflow


#
# Targets.

default: clean all

all: integerlimits


clean:
	rm -f integerlimits


# Test getting information about integer types.

integerlimits: integerlimits.cpp
	g++ $(CFLAGS) -o integerlimits integerlimits.cpp
	./integerlimits
	g++ $(CFLAGS) $(LIMITKLUDGE) -o integerlimits integerlimits.cpp
	./integerlimits
	rm -f integerlimits


#
# This is the end of the file.
