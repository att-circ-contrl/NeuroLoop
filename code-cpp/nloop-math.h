// Attention Circuits Control Laboratory - NeuroLoop project
// Miscellaneous math routines.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_MATH_H
#define NLOOP_MATH_H


//
// Functions

// Single-sample function for fast modulo arithmetic of non-negative integers.
// This tests for quotients up to (2^count)-1.
// Division is expensive, especially in FPGAs, so for situations where we
// know that the quotient is small, shift-and-subtract can be less
// resource-intensive.

// NOTE - We're counting on the compiler being smart enough to inline this.
template <class datatype_t, int subcount>
datatype_t nloop_FastModulo(datatype_t sample, datatype_t modulus);


// Slice-based function for fast modulo arithmetic of non-negative integers.
// This tests for quotients up to 2^(count-1).
// Division is expensive, especially in FPGAs, so for situations where we
// know that the quotient is small, shift-and-subtract can be less
// resource-intensive.
// Input and output may reference the same object.

template <class datatype_t, int subcount, int bankcount, int chancount>
void nloop_FastModulo_Bank(
  nloop_SampleSlice_t<datatype_t,bankcount,chancount> &indata,
  nloop_SampleSlice_t<datatype_t,bankcount,chancount> &moduli,
  nloop_SampleSlice_t<datatype_t,bankcount,chancount> &outdata
);



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-math-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
