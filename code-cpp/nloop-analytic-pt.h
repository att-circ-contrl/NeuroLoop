// Attention Circuits Control Laboratory - NeuroLoop project
// Analytic function proxy implementations - Peak and trough based.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.

#ifndef NLOOP_ANALYTIC_PT_H
#define NLOOP_ANALYTIC_PT_H


//
// Classes


//
// Peak, trough, and zero-crossing analytic estimator.

// The "full period" flag indicates whether period is to be estimated from
// the most recent lobe (short delay, more variation) or from the most
// recent two lobes (longer delay, less variation).
// NOTE - The default zero level is 0 for both signed and unsigned samptype_t.
// This means unsigned values wrap around (default band-pass output behavior).

template <class samptype_t, class indextype_t>
class nloop_Analytic_PTZC_t
{
protected:
  // Configuration.
  samptype_t zero_level;
  indextype_t min_zc_gap;

  // Internal state.
  samptype_t max_mag_seen;
  samptype_t last_mag;
  indextype_t since_rise_count, since_fall_count;
  indextype_t last_period;

public:
  nloop_Analytic_PTZC_t(void);
  // Default destructor is fine.
  // NOTE - This also resets zero level (to 0) and minimum period (to maxval).
  void ResetState(void);
  // NOTE - This should be substantially smaller than the input signal's
  // actual minimum period.
  void SetMinPeriod(indextype_t newminperiod);
  void SetZeroLevel(samptype_t newzero);
  void HandleSample(samptype_t sampval);
  void GetEstimatedAnalytic(samptype_t &magnitude, indextype_t &period,
    indextype_t &since_rise_zc, indextype_t &since_fall_zc);
};



// FIXME - Flank-crossing estimator NYI.



// Array of peak-and-trough analytic signal approximators, indexed by channel
// and bank.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
class nloop_AnalyticBank_PT_t
{
protected:
  // Analytic approximator instances.
  estimator_t estimators[bankcount][chancount];

  // Number of channels and banks that are actually being used.
  // This lets us change geometry at run-time rather than compile-time.
  int chans_active;
  int banks_active;

public:
  // Default constructor and destructor are fine.
  // Estimators are required to handle their own first-time initialization.


  // Processing functions.

  // This also resets zero levels and active bank/channel counts.
  void ResetState(void);

  // These only operate on active banks/channels.
  void HandleSamples(
    nloop_SampleSlice_t<samptype_t, bankcount, chancount> &indata
  );
  void GetEstimatedAnalytic(
    nloop_SampleSlice_t<samptype_t, bankcount, chancount> &outmagnitude,
    nloop_SampleSlice_t<indextype_t, bankcount, chancount> &outperiod,
    nloop_SampleSlice_t<indextype_t, bankcount, chancount> &since_rise_zc,
    nloop_SampleSlice_t<indextype_t, bankcount, chancount> &since_fall_zc
  );


  // Accessors.

  int GetActiveChans(void);
  void SetActiveChans(int new_chans);
  int GetActiveBanks(void);
  void SetActiveBanks(int new_banks);

  void SetMinPeriods(
    nloop_SampleSlice_t<indextype_t,bankcount,1> &newminperiods
  );
  void SetOneMinPeriod(int bankidx, indextype_t newminperiod);

  void SetZeroLevels(
    nloop_SampleSlice_t<samptype_t,bankcount,chancount> &newzeros
  );
  void SetOneZeroLevel(int bankidx, int chanidx, samptype_t newzero);
};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-analytic-pt-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
