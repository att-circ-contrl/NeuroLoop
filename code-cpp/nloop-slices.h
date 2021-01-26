// Attention Circuits Control Laboratory - NeuroLoop project
// Data slice declarations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_SLICES_H
#define NLOOP_SLICES_H


//
// Data Buffer Class


// This represents one "slice" of sample data across all channels and
// filter banks within a signal processing pipeline.

template <class samptype_t, int bankcount, int chancount>
class nloop_SampleSlice_t
{
public:
  samptype_t data[bankcount][chancount];

  // Common operations.

  void CopyFrom(nloop_SampleSlice_t<samptype_t,bankcount,chancount> &source);
  void SetUniformValue(samptype_t newval);
};



//
// Functions


// This maps selected input slice cells to output slice cells.

template <class samptype_t, int bankcountsrc, int chancountsrc,
  int bankcountdst, int chancountdst>
void nloop_MapSlice(
  nloop_SampleSlice_t<int,bankcountdst,chancountdst> &src_banks,
  nloop_SampleSlice_t<int,bankcountdst,chancountdst> &src_chans,
  nloop_SampleSlice_t<samptype_t,bankcountsrc,chancountsrc> &source,
  nloop_SampleSlice_t<samptype_t,bankcountdst,chancountdst> &target
);


//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-slices-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
