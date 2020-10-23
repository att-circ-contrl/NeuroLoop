// Attention Circuits Control Laboratory - NeuroLoop project
// Top-level include file.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

// Standard library includes.
#include <stdint.h>
#include <stdlib.h>


//
// Signal-Processing Classes


// Representation of an IIR biquad.
// NOTES:
// We're using a Direct Form 1 implementation.
// y[n] = (1/a0)( b0 x[n] + b1 x[n-1] + b2 x[n-2] - a1 y[n-1] - a2 y[n-2] )
// a0 is a power of two, for fast division (a0 = 2^a0bits).
// The transfer function is:
// H(z) = (b0 + b1*z^-1 + b2*z^-2) / (a0 + a1*z^-1 + a2*z^-2)
//
// NOTE - If you use a signed sample type rather than unsigned, set compiler
// flags to force an "arithmetic" shift-right operation on such types
// (preserving sign). If you can't guarantee this, stick to unsigned types.
//
// The index type is used for indexing and masking with circular buffers.
// It's typically an unsigned integer type.

template <class samptype_t, class indextype_t> class nloop_IIRBiquad
{
protected:
  // Biquad filter coefficients.
  uint8_t den0_bits;
  samptype_t den1, den2;
  samptype_t num0, num1, num2;

public:
  // Default constructor and destructor are fine.
  // Coefficients initialized to zero give a valid filter with zero output.

  // Processing functions.

  // This operates on linear buffers.
  // NOTE - Elements [0], [-1], and [-2] are read/written.
  void ApplyBiquadOnceLinear(samptype_t *inbuf, samptype_t *outbuf);

  // This operates on circular buffers.
  // NOTE - Buffer size must be a power of two! The mask is used for
  // wrapping. Elements [n], [n-1], and [n-2] are read/written.
  void ApplyBiquadOnceCircular(
    samptype_t *inbuf, indextype_t inptr, indextype_t inbufmask,
    samptype_t *outbuf, indextype_t outptr, indextype_t outbufmask
  );


  // Accessors.

  void GetCoefficients(uint8_t &old_den0bits,
    samptype_t &old_den1, samptype_t &old_den2,
    samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2);

  void SetCoefficients(uint8_t new_den0bits,
    samptype_t new_den1, samptype_t new_den2,
    samptype_t new_num0, samptype_t new_num1, samptype_t new_num2);
};




// Representation of a cascaded chain of one or more IIR biquads.
// Note - This operates on single samples, rather than input/output arrays.
// Filters may take time to stabilize as a result.

#define NLOOP_IIRBIQUADCHAIN_BUFSIZE 4

template <class samptype_t, class indextype_t, int stagecount>
class nloop_IIRBiquadChain
{
protected:
  // Biquad instances.
  nloop_IIRBiquad<samptype_t, indextype_t> biquads[stagecount];

  // Buffers for intermediate results.
  // Make life easier on ourselves and buffer input and output too.
  // Make life easier on ourselves and make buffers power-of-2 length.
  samptype_t buffers[stagecount+1][NLOOP_IIRBIQUADCHAIN_BUFSIZE];
  int bufptr;

  // Number of stages that are actually being used.
  // This lets us change geometry at run-time rather than compile-time.
  int stages_active;

public:
  // Default constructor and destructor are fine.
  // Filters are initialized as valid filters with zero output.

  // Processing functions.

  // This will still work with zero active stages (copying input to output).
  // NOTE - This only reads the addressed elements. History is kept in
  // internal buffers. As a result, this takes time to stabilize.
  void ApplyChainOnce(samptype_t &indata, samptype_t &outdata);


  // Accessors.

  int GetActiveStages(void);
  void SetActiveStages(int new_stages);

  void GetCoefficients(int stagenum,
    uint8_t &old_den0bits, samptype_t &old_den1, samptype_t &old_den2,
    samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2);

  void SetCoefficients(int stagenum,
    uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
    samptype_t new_num0, samptype_t new_num1, samptype_t new_num2);

  // This lets us stuff the internal buffers with values that will settle
  // quickly. The constructor initializes to zero, which may take a while
  // to settle to mid-range for unsigned input and a slow filter.
  void StuffBufferStage(int stagenum, samptype_t value);
};



// Representation of an array of IIR filters, indexed by channel and by
// bank. Filter instances within the same bank have the same coefficients.
//
// The reason for doing this at the library level is to allow 1:1 mapping
// between the C++ implementation and the HDL implementation (which will
// time-multiplex channels and possibly banks to conserve hardware).
//
// NOTE - This operates on single slices, rather than many-sample arrays.
// Filters may take time to stabilize as a result.

template <class samptype_t, class indextype_t, int stagecount,
  int bankcount, int chancount>
class nloop_IIRFilterBank
{
protected:
  // Biquad chain instances.
  // This is a two-dimensional array; even though coefficients are shared,
  // buffered data will vary.
  // NOTE - This means coefficient values are replicated, which may cause
  // space issues in embedded systems.
  nloop_IIRBiquadChain<samptype_t, indextype_t, stagecount>
    biquads[bankcount][chancount];

  // Number of channels and banks that are actually being used.
  // This lets us change geometry at run-time rather than compile-time.
  int chans_active;
  int banks_active;


public:
  // Default constructor and destructor are fine.
  // Filters are initialized as valid filters with zero output.

  // Type definitions.
  typedef samptype_t sampleslice_t[bankcount][chancount];

  // Processing functions.

  // This will still work with zero active stages (copying input to output).
  // NOTE - This only manipulates active channels and banks. Unused parts
  // of the output array will get stale.
  void ApplyBankOnce(sampleslice_t &indata, sampleslice_t &outdata);


  // Accessors.

  int GetActiveStages(void);
  void SetActiveStages(int new_stages);

  int GetActiveChans(void);
  void SetActiveChans(int new_chans);

  int GetActiveBanks(void);
  void SetActiveBanks(int new_banks);

  void GetCoefficients(int stagenum, int banknum,
    uint8_t &old_den0bits, samptype_t &old_den1, samptype_t &old_den2,
    samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2);

  void SetCoefficients(int stagenum, int banknum,
    uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
    samptype_t new_num0, samptype_t new_num1, samptype_t new_num2);

  // This lets us stuff the internal buffers with values that will settle
  // quickly. The constructor initializes to zero, which may take a while
  // to settle to mid-range for unsigned input and a slow filter.
  void StuffBufferStage(int stagenum, int banknum, int channum,
    samptype_t value);
};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at compile-time.

#include "nloop-biquads-inc.cpp"


//
// This is the end of the file.
