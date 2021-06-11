// Attention Circuits Control Laboratory - NeuroLoop project
// FIR filter declarations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_FIR_H
#define NLOOP_FIR_H


//
// FIR Filter Classes

// Representation of a FIR filter.
// NOTES:
// This uses fixed-point arithmetic:
// y[t] = (1/2^b) sum(k=0..(n-1))( a[k] * x[t+k] )
// The "1/2^b" operation is implemented as a bit-shift.
//
// NOTE - If you use a signed sample type rather than unsigned, do one of
// two things:
// a) Define "NLOOP_SIGN_SAFE_SHIFT".
// or
// b) Set compiler flags to force "arithmetic" shift-right operations for
// signed data (sign-preserving shift).
// The sign-safe code is slower but doesn't rely on the compiler.
//
// The index type is used for indexing and for masking with circular buffers.
// It's typically an unsigned integer type.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
class nloop_FIRFilter_t
{
protected:
  // FIR coefficients.
  uint8_t fracbits;
  indextype_t coeffcount;
  samptype_t coeffs[maxcoeffs];

public:
  // Constructor.
  nloop_FIRFilter_t(void);
  // Default destructor is fine.


  // Processing functions.

  // This operates on linear buffers.
  // NOTE - Elements [0]..[n-1] are read. y[0] is returned.
  samptype_t ApplyFIROnceLinear(samptype_t *inbuf);

  // This operates on circular buffers.
  // NOTE - Buffer size must be a power of two! The mask is used for
  // wrapping. Elements [0]..[n-1] (modulo buffer length) are read.
  // y[0] is returned.
  samptype_t ApplyFIROnceCircular(
    samptype_t *inbuf, indextype_t inptr, indextype_t inbufmask
  );


  // Accessors.

  // This sets all coefficients to zero and fracbits to zero.
  // This is a valid filter configuration with zero output.
  void BlankCoefficients(void);

  void SetFracBits(uint8_t newbits);
  uint8_t GetFracBits(void);

  void SetCoeffCount(indextype_t newcount);
  indextype_t GetCoeffCount(void);

  void SetOneCoefficient(indextype_t coeffidx, samptype_t coeffval);
  samptype_t GetOneCoefficient(indextype_t coeffidx);

  void SetAllCoefficients(uint8_t newbits, indextype_t newcoeffcount,
    nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &newcoeffs);
  void GetAllCoefficients(uint8_t &oldbits, indextype_t &oldcoeffcount,
    nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &oldcoeffs);
};



// Representation of an array of FIR filters, indexed by channel and by
// bank. Each bank has one FIR filter shared by all channels. Each channel
// has one input buffer shared by all banks.
//
// The reason for doing this at the library level is to allow 1:1 mapping
// between the C++ implementation and the HDL implementation (which will
// time-multiplex channels and possibly banks to conserve hardware).
//
// NOTE - Buffer length must be a power of two, for masking.
//
// NOTE - Filter output will take one FIR length to stabilize. The input
// buffers can be fast-settled to sidestep this.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
class nloop_FIRFilterBank_t
{
protected:
  // FIR instances.
  // This is a one-dimensional array; the FIR filters don't have internal
  // signal processing state.
  nloop_FIRFilter_t<samptype_t,indextype_t,maxcoeffs> firs[bankcount];

  // Input data buffers.
  // This is a one-dimensional array; input data isn't modified, so it can
  // be shared across banks.
  samptype_t inbufs[chancount][buflen];
  indextype_t bufptr;

  // Number of channels and banks that are actually being used.
  // This lets us change geometry at run-time.
  int chans_active;
  int banks_active;


public:
  // Constructor.
  nloop_FIRFilterBank_t(void);
  // Default destructor is fine.


  // Processing functions.

  void ApplyBankOnce(
    nloop_SampleSlice_t<samptype_t,1,chancount> &indata,
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &outdata
  );


  // Accessors.

  int GetActiveChans(void);
  void SetActiveChans(int new_chans);

  int GetActiveBanks(void);
  void SetActiveBanks(int new_banks);

  // This calls BlankCoefficients() for one or all filters.
  void BlankAllFilters(void);
  void BlankOneFilter(int banknum);

  void SetOneCoefficient(int banknum,
    indextype_t coeffidx, samptype_t coeffval);
  samptype_t GetOneCoefficient(int banknum, indextype_t coeffidx);

  void SetOneGeometry(int banknum,
    uint8_t newfracbits, indextype_t newcoeffcount);
  void GetOneGeometry(int banknum,
    uint8_t &oldfracbits, indextype_t &oldcoeffcount);

  void SetBankCoefficients(int banknum,
    int newbits, indextype_t newcoeffcount,
    nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &newcoeffs);
  void GetBankCoefficients(int banknum,
    int &oldbits, indextype_t &oldcoeffcount,
    nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &oldcoeffs);

  // This zeroes the input buffers for one or all channels.
  void BlankAllInputBuffers(void);
  void BlankOneInputBuffer(int channum);
  // This copies the specified input values to the input buffers.
  void FastSettleBuffers(
    nloop_SampleSlice_t<samptype_t,1,chancount> &indata);

};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-fir-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
