// Attention Circuits Control Laboratory - NeuroLoop project
// Biquad filter declarations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_BIQUADS_H
#define NLOOP_BIQUADS_H


//
// IIR Biquad Filter Classes


// Representation of an IIR biquad.
// NOTES:
// We're using a Direct Form 1 implementation.
// y[n] = (1/a0)( b0 x[n] + b1 x[n-1] + b2 x[n-2] - a1 y[n-1] - a2 y[n-2] )
// a0 is a power of two, for fast division (a0 = 2^a0bits).
// The transfer function is:
// H(z) = (b0 + b1*z^-1 + b2*z^-2) / (a0 + a1*z^-1 + a2*z^-2)
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

template <class samptype_t, class indextype_t> class nloop_IIRBiquad_t
{
protected:
  // Biquad filter coefficients.
  uint8_t den0_bits;
  samptype_t den1, den2;
  samptype_t num0, num1, num2;

public:
  // Default initialization should give zero coefficients, but force anyways.
  nloop_IIRBiquad_t(void);
  // Default destructor is fine.


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

  // This sets all coefficients to zero and den0bits to zero.
  // This is a valid filter configuration with zero output.
  void BlankCoefficients(void);

  void GetCoefficients(uint8_t &old_den0bits,
    samptype_t &old_den1, samptype_t &old_den2,
    samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2);

  void SetCoefficients(uint8_t new_den0bits,
    samptype_t new_den1, samptype_t new_den2,
    samptype_t new_num0, samptype_t new_num1, samptype_t new_num2);
};




// Representation of a cascaded chain of one or more IIR biquads.
// Note - This operates on single samples, rather than buffered arrays.
// Filters may take time to stabilize as a result.

#define NLOOP_IIRBIQUADCHAIN_BUFSIZE 4

template <class samptype_t, class indextype_t, int stagecount>
class nloop_IIRBiquadChain_t
{
protected:
  // Biquad instances.
  nloop_IIRBiquad_t<samptype_t, indextype_t> biquads[stagecount];

  // Buffers for intermediate results.
  // Make life easier on ourselves and buffer input and output too.
  // Make life easier on ourselves and make buffers power-of-2 length.
  samptype_t buffers[stagecount+1][NLOOP_IIRBIQUADCHAIN_BUFSIZE];
  int bufptr;

  // Number of stages that are actually being used.
  // This lets us change geometry at run-time rather than compile-time.
  int stages_active;

public:
  // Default initialization should give zero coefficients, but force anyways.
  nloop_IIRBiquadChain_t(void);
  // Default destructor is fine.


  // Processing functions.

  // This will still work with zero active stages (copying input to output).
  // NOTE - This only reads the addressed elements. History is kept in
  // internal buffers. As a result, this takes time to stabilize.
  void ApplyChainOnce(samptype_t &indata, samptype_t &outdata);


  // Accessors.

  int GetActiveStages(void);
  void SetActiveStages(int new_stages);

  // This sets all coefficients to zero and den0bits to zero.
  // This is a valid filter configuration with zero output.
  void BlankCoefficients(void);

  void GetCoefficients(int stagenum,
    uint8_t &old_den0bits, samptype_t &old_den1, samptype_t &old_den2,
    samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2);

  void SetCoefficients(int stagenum,
    uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
    samptype_t new_num0, samptype_t new_num1, samptype_t new_num2);

  // This lets us stuff the internal buffers with values that will settle
  // quickly. Buffers are either stuffed with the input value (suitable for
  // low-pass stages) or with zero (suitable for high-pass and band-pass).
  void FastSettleBuffers(samptype_t &indata, bool (&copy_input)[stagecount]);
};



// Representation of an array of IIR filters, indexed by channel and by
// bank. Filter instances within the same bank have the same coefficients.
// Inputs from a given channel are replicated across banks.
//
// The reason for doing this at the library level is to allow 1:1 mapping
// between the C++ implementation and the HDL implementation (which will
// time-multiplex channels and possibly banks to conserve hardware).
//
// NOTE - This operates on single slices, rather than buffered arrays.
// Filters may take time to stabilize as a result.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
class nloop_IIRFilterBank_t
{
protected:
  // Biquad chain instances.
  // This is a two-dimensional array; even though coefficients are shared,
  // buffered data will vary.
  // NOTE - This means coefficient values are replicated, which may cause
  // space issues in embedded systems.
  nloop_IIRBiquadChain_t<samptype_t, indextype_t, stagecount>
    biquads[bankcount][chancount];

  // Number of channels and banks that are actually being used.
  // This lets us change geometry at run-time rather than compile-time.
  int chans_active;
  int banks_active;


public:
  // Default initialization should give zero coefficients, but force anyways.
  nloop_IIRFilterBank_t(void);
  // Default destructor is fine.


  // Processing functions.

  // This will still work with zero active stages (copying input to output).
  // NOTE - This only manipulates active channels and banks. Unused parts
  // of the output array will get stale.
  void ApplyBankOnce(
    nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    nloop_SampleSlice_t<samptype_t, bankcount, chancount> &outdata
  );


  // Accessors.

  int GetActiveStages(void);
  void SetActiveStages(int new_stages);

  int GetActiveChans(void);
  void SetActiveChans(int new_chans);

  int GetActiveBanks(void);
  void SetActiveBanks(int new_banks);

  // This sets all coefficients to zero and den0bits to zero.
  // This is a valid filter configuration with zero output.
  void BlankCoefficients(void);

  void GetCoefficients(int stagenum, int banknum,
    uint8_t &old_den0bits, samptype_t &old_den1, samptype_t &old_den2,
    samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2);

  // Coefficients are updated for all channels, not just active channels.
  void SetCoefficients(int stagenum, int banknum,
    uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
    samptype_t new_num0, samptype_t new_num1, samptype_t new_num2);

  // This lets us stuff the internal buffers with values that will settle
  // quickly. Buffers are either stuffed with the input value (suitable for
  // low-pass stages) or with zero (suitable for high-pass and band-pass).
  // This updates all channels, not just active channels.
  void FastSettleBuffers(
    nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    bool (&copy_input)[stagecount]
  );
};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-biquads-inc.cpp"


// End of wrapper.
#endif

//
// This is the end of the file.
