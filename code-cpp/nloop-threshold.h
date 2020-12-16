// Attention Circuits Control Laboratory - NeuroLoop project
// Threshold-based detectors.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_THRESH_H
#define NLOOP_THRESH_H


//
// Classes


//
// Averager.

// This computes a running average of a signal using a first-order
// exponential filter, and multiplies it by a coefficient.
// For higher filter orders, use an IIR biquad instead of this.
// This uses bit-shifting instead of division for speed.
// NOTE - You need at least max(avgbits,coeffbits) bits of headroom.
// NOTE - This assumes that unsigned samptype_t contains signed data (as is
// the case after band-pass filtering).


// Individual version.

template <class samptype_t, uint8_t coeffbits>
class nloop_Averager_t
{
protected:
  samptype_t running_sum;
  samptype_t coeff;
  uint8_t avgbits;

public:
  // This updates the running average based on the input.
  // The approximate settling time is 2^avgbits samples.
  // The output value is ( average * coeff / 2^coeffbits ).
  samptype_t UpdateAverage(samptype_t indata);

  // Use InitAverage() to avoid startup transients.
  void InitAverage(samptype_t indata);
  void SetCoeff(samptype_t new_coeff);
  void SetAvgBits(uint8_t new_avgbits);
};


// Bank version.

template <class samptype_t, uint8_t coeffbits,
  int bankcount, int chancount>
class nloop_AveragerBank_t
{
protected:
  nloop_Averager_t<samptype_t,coeffbits>
    averagers[bankcount][chancount];

  // Number of channels and banks that are actually being used.
  // This lets us change geometry at run-time rather than compile-time.
  int banks_active;
  int chans_active;

public:
  // This forces a sane state.
  nloop_AveragerBank_t(void);
  // Default destructor is fine.


  // Processing functions.

  // This updates the running average based on the input.
  // The approximate settling time is 2^avgbits samples.
  // The output value is ( average * coeff / 2^coeffbits ).
  // This only operates on active banks/channels.
  void UpdateAverage(
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &indata,
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &outdata );

  // Use InitAverage() to avoid startup transients.
  void InitAverage(
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &indata );


  // Accessors.

  int GetActiveChans(void);
  void SetActiveChans(int new_chans);
  int GetActiveBanks(void);
  void SetActiveBanks(int new_banks);

  void SetCoeffs(
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &new_coeffs );
  void SetBankCoeffs(
    nloop_SampleSlice_t<samptype_t,bankcount,1> &new_coeffs );
  void SetChanCoeffs(
    nloop_SampleSlice_t<samptype_t,1,chancount> &new_coeffs );
  void SetUniformCoeffs(samptype_t new_coeff);
  void SetOneCoeff(int bankidx, int chanidx, samptype_t new_coeff);

  void SetAvgBits(
    nloop_SampleSlice_t<uint8_t,bankcount,chancount> &new_avgbits );
  void SetBankAvgBits(
    nloop_SampleSlice_t<uint8_t,bankcount,1> &new_avgbits );
  void SetChanAvgBits(
    nloop_SampleSlice_t<uint8_t,1,chancount> &new_avgbits );
  void SetUniformAvgBits(uint8_t new_avgbits);
  void SetOneAvgBits(int bankidx, int chanidx, uint8_t new_avgbits);
};



//
// Boolean De-Glitcher.

// This delays rising and falling edges by specified amounts, removing
// spurious brief pulses or drop-outs, but adding delay.


// Individual version.

template <class indextype_t>
class nloop_DeGlitcher_t
{
protected:
  indextype_t rise_delay, fall_delay;
  indextype_t rise_countdown, fall_countdown;
  bool last_output;

public:
  // This processes one input sample.
  bool ProcessSample(bool indata);

  // This sets the delays and resets the countdowns.
  void SetDelays(indextype_t new_rise_delay, indextype_t new_fall_delay);
};


// Bank version.

template <class indextype_t, int bankcount, int chancount>
class nloop_DeGlitcherBank_t
{
protected:
  nloop_DeGlitcher_t<indextype_t> deglitchers[bankcount][chancount];

public:
  // This processes one input slice.
  void ProcessSample( nloop_SampleSlice_t<bool,bankcount,chancount> &indata,
    nloop_SampleSlice_t<bool,bankcount,chancount> &outdata );

  // These set the delays and reset the countdowns.
  void SetDelays(
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &new_rise_delays,
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &new_fall_delays );
  void SetBankDelays(
    nloop_SampleSlice_t<indextype_t,bankcount,1> &new_rise_delays,
    nloop_SampleSlice_t<indextype_t,bankcount,1> &new_fall_delays );
  void SetChanDelays(
    nloop_SampleSlice_t<indextype_t,1,chancount> &new_rise_delays,
    nloop_SampleSlice_t<indextype_t,1,chancount> &new_fall_delays );
  void SetUniformDelays(
    indextype_t new_rise_delay, indextype_t new_fall_delay );
  void SetOneDelays( int bankidx, int chanidx,
    indextype_t new_rise_delay, indextype_t new_fall_delay );
};



//
// Thresholding detectors.


// Single-threshold detector.
// Events happen when the signal rises above a specified threshold.
// This is a trivial element but it's used to build more complex elements.

template <class samptype_t, int bankcount, int chancount>
class nloop_ThresholdSingleBank_t
{
public:
  // This returns true if and only if the sample is at or above the threshold.
  void TestSamples(
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &indata,
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &thresholds,
    nloop_SampleSlice_t<bool,bankcount,chancount> &outflag );
};



// Two-threshold detector.
// This combines boolean flags from two single-threshold detectors.
// Events start when the signal rises above a higher threshold, and stop
// when it falls below a lower threshold (i.e. detection has hysteresis).

template <int bankcount, int chancount>
class nloop_ThresholdDualBank_t
{
protected:
  nloop_SampleSlice_t<bool,bankcount,chancount> prev_state;

public:
  // This resets internal state to "no events detected".
  void ResetState(void);

  // This returns true if and only if the sample rose to the turn-on
  // threshold and has not yet fallen below the turn-off threshold.
  void TestDual(
    nloop_SampleSlice_t<bool,bankcount,chancount> &flag_activate,
    nloop_SampleSlice_t<bool,bankcount,chancount> &flag_sustain,
    nloop_SampleSlice_t<bool,bankcount,chancount> &outflag );
};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-threshold-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
