// Attention Circuits Control Laboratory - NeuroLoop project
// Top-level include file for embedded systems.
// This leaves out file I/O and other workstation-specific features.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Standard library includes.

#include <stdint.h>
#include <stdlib.h>

// NOTE - Some embedded compilers, like 2014 avr-gcc, don't support <limits>!
// Give the option of compiling without it.
#ifndef NLOOP_KLUDGE_LIMITS
// NOTE - We need C++11 to handle 64-bit types with this library.
#include <limits>
#endif


//
// NeuroLoop includes.

#include "nloop-misc.h"
#include "nloop-preproc.h"
#include "nloop-biquads.h"


//
// This is the end of the file.