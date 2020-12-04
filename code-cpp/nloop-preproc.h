// Attention Circuits Control Laboratory - NeuroLoop project
// Preprocessing modules - Declarations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_PREPROC_H
#define NLOOP_PREPROC_H


//
// Auto-Ranging Classes


// Auto-ranging module.
// This monitors the range of the input and computes an attenuation bit-shift
// and offset to make it fit in a user-specified range.
// The mapping used is:  outval = (inval >> attenbits) + oset
// Offset may be positive or negative.

template<class samptype_t, class indextype_t, int chancount>
class nloop_AutoRanger_t
{
protected:
  // Internal state.

  samptype_t minvals[chancount];
  samptype_t maxvals[chancount];

  indextype_t latch_countdown;
  bool countdown_active;

  samptype_t middle_wanted, halfspan_wanted;

  // Attenuation can be per-channel or tied together.
  // Offsets are always per-channel.
  bool atten_tied;

  // NOTE - We aren't actually doing a running calculation.
  // An FPGA-based vesion would, but it'd slow down an embedded version.
  // Instead, we just compute it when we need it.
  samptype_t running_offsets[chancount];
  uint8_t running_attens[chancount];

  // This gets latched from the running values when the countdown times out.
  samptype_t latched_offsets[chancount];
  uint8_t latched_attens[chancount];


  // Helper functions.

  // This recalculates the running attenuation and offset values.
  void RecalcAttenOffset(void);
  // This computes the attenuated and shifted value of the input.
  void CalcOutput(nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    nloop_SampleSlice_t<samptype_t, 1, chancount> &outdata,
    bool use_latched);

public:
  // Constructor.
  nloop_AutoRanger_t(void);
  // Default destructor is fine.

  // Processing functions.

  // An FPGA-based implementation would do the equivalent of calling
  // UpdateFromSample(), then calling _both_ "GetXOutput()" functions,
  // Then using a mux to select from raw input, latched output, or running
  // output.

  // This updates the internal state used to calculate attenuation and
  // offset.
  void UpdateFromSample(nloop_SampleSlice_t<samptype_t, 1, chancount> &data);

  // This computes transformed output using either the running attenuation
  // and offset or the latched attenuation and offset.
  // This does NOT update the internal state.
  void GetRunningOutput(nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    nloop_SampleSlice_t<samptype_t, 1, chancount> &outdata);
  void GetLatchedOutput(nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    nloop_SampleSlice_t<samptype_t, 1, chancount> &outdata);

  // Accessors.

  void ResetTracking(bool want_shared_atten);
  void ResetLatched(void);
  void LatchAfter(indextype_t sampcount);
  bool IsAutoRangeRunning(void);
  void SetDesiredRange(samptype_t newmin, samptype_t newmax);

  // Debugging accessors.

  // These return the minimum and maximum sample values seen since reset.
  void GetMinValuesSeen(nloop_SampleSlice_t<samptype_t, 1, chancount> &data);
  void GetMaxValuesSeen(nloop_SampleSlice_t<samptype_t, 1, chancount> &data);
  // These return the active offsets and attenuations.
  void GetRunningAttenOffset(
    nloop_SampleSlice_t<samptype_t,1,chancount> &bitshifts,
    nloop_SampleSlice_t<samptype_t,1,chancount> &offsets);
  void GetLatchedAttenOffset(
    nloop_SampleSlice_t<samptype_t,1,chancount> &bitshifts,
    nloop_SampleSlice_t<samptype_t,1,chancount> &offsets);
  // This manually latches the specified attenuation and offset values.
  void SetAttenOffset(
    nloop_SampleSlice_t<samptype_t,1,chancount> &bitshifts,
    nloop_SampleSlice_t<samptype_t,1,chancount> &offsets);
};



//
// Artifact-Rejection Class

// FIXME - Artifact-rejection NYI.



//
// Code Inclusion

// C++ compiles templated classes and functions on-demand. The source code
// has to be included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies
// get pruned at link-time.

#include "nloop-preproc-autorange-inc.cpp"
// FIXME - Artifact-rejection goes here.


// End of wrapper.
#endif

//
// This is the end of the file.
