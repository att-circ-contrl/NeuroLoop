// Attention Circuits Control Laboratory - NeuroLoop project
// Trigger logic.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_TRIGGER_H
#define NLOOP_TRIGGER_H


//
// Functions

// We're keeping the different target logic cases distinct so that each
// can be implemented using the minimum needed resources.
// Likewise with the flag logic.

// These have no internal state, so they don't have to be classes.


//
// Zero-crossing target logic.

// This selects either the rising or falling delay from the specified
// bank and channel, for each trigger.
template <class indextype_t, int bankcount, int chancount, int trigcount>
void nloop_TargetBankZC_SelectInputs(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<bool,1,trigcount> &want_falling,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &rise_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &fall_delays,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &signals_out
);


//
// Phase target logic.

// This converts the nominal target phase fraction (0..255) into a
// delay in samples (frac * period / 256), for each trigger.
template <class indextype_t, int bankcount, int chancount, int trigcount>
void nloop_TargetBankPhase_SelectTargets(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &periods,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &nominal_targets,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &targets_out
);


//
// Zero-crossing plus phase target logic.

// This selects the rising delay, falling delay, or delay since phase 0
// from the specified bank and channel, and either copies the target
// delay or converts a nominal target phase fraction (0..255) into a
// delay in samples (frac * period / 256), for each trigger.
// "want_phase" takes priority over "want_falling".
template <class indextype_t, int bankcount, int chancount, int trigcount>
void nloop_TargetBankZCPhase_SelectInputsAndTargets(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<bool,1,trigcount> &want_phase,
  nloop_SampleSlice_t<bool,1,trigcount> &want_falling,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &rise_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &fall_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &periods,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &signals_out,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &nominal_targets,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &targets_out
);


//
// Conditional flag logic.

// This triggers depending on the states of two flags.
// Per-trigger configuration options are "A", "A and B", "A and not B".
template <int bankcount, int chancount, int trigcount>
void nloop_ConditionalFlagDual_SelectFlags(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<bool,1,trigcount> &want_secondary,
  nloop_SampleSlice_t<bool,1,trigcount> &negate_secondary,
  nloop_SampleSlice_t<bool,bankcount,chancount> &input_primary,
  nloop_SampleSlice_t<bool,bankcount,chancount> &input_secondary,
  nloop_SampleSlice_t<bool,1,trigcount> &output_flags
);



//
// Classes


//
// Trigger generator.

// Individual version.

template<class indextype_t>
class nloop_Trigger_t
{
protected:
  // Configuration.
  indextype_t trig_duration, trig_cooldown_time;
  bool reraise_ok;

  // Trigger state private enum.
  enum trigstate_t
  {
    NLOOP_TSTATE_IDLE = 0,
    NLOOP_TSTATE_WAITRISE,
    NLOOP_TSTATE_WAITFALL,
    NLOOP_TSTATE_WAITCOOL
  };

  // Trigger state.
  trigstate_t state;
  indextype_t timeout_left;
  indextype_t saved_target;
  indextype_t prev_signal;
  indextype_t unwrap_offset;

public:
  nloop_Trigger_t(void);
  // Default destructor is fine.

  void ResetState(void);
  void ForceIdle(void);

  // Processing functions.

  // This accepts a delay/phase sample and returns the current output state.
  // This checks trigger_count_left before triggering and decrements it if
  // a new trigger event is generated.
  bool ProcessSample(indextype_t thisval, indextype_t thistarget,
    indextype_t thisperiod, bool thisdetectflag,
    indextype_t &trigger_count_left);

  // Accessors.

  void SetPulseDuration(indextype_t new_duration_samps);
  void SetPulseCooldown(indextype_t new_cooldown_samps);
  void SetReRaise(bool want_reraise);

  indextype_t GetPulseDuration(void);
  indextype_t GetPulseCooldown(void);
  bool GetReRaise(void);
};


// Bank version.

template<class indextype_t, int bankcount, int chancount>
class nloop_TriggerBank_t
{
protected:
  // Priming state.
  indextype_t trigger_count_left, window_time_left;

  // Individual triggers.
  nloop_Trigger_t<indextype_t> triggers[bankcount][chancount];
  // Per-trigger enable flags.
  nloop_SampleSlice_t<bool,bankcount,chancount> enabled;

  // Active geometry.
  int banks_active;
  int chans_active;

public:
  nloop_TriggerBank_t(void);
  // Default destructor is fine.

  void ResetState(void);
  void ForceIdle(void);

  // Processing functions.

  void EnableTriggering( indextype_t active_window_samps,
    indextype_t max_pulses_sent );
  void DisableTriggering(void);
  // This accepts a delay/phase sample and returns the current output state.
  void ProcessSamples(
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &sampvals,
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &targetvals,
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &periods,
    nloop_SampleSlice_t<bool,bankcount,chancount> &detectflags,
    nloop_SampleSlice_t<bool,bankcount,chancount> &trigsout );

  // Accessors.

  void SetActiveBanks(int new_banks);
  void SetActiveChans(int new_chans);

  int GetActiveBanks(void);
  int GetActiveChans(void);


  void SetEnableFlags(
    nloop_SampleSlice_t<bool,bankcount,chancount> &want_enabled );

  void SetPulseDurations(
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &duration_samps );
  void SetPulseCooldowns(
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &cooldown_samps );

  void SetAllReRaises(bool want_reraise);


  void GetEnableFlags(
    nloop_SampleSlice_t<bool,bankcount,chancount> &is_enabled );
  void GetPulseDurations(
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &duration_samps );
  void GetPulseCooldowns(
    nloop_SampleSlice_t<indextype_t,bankcount,chancount> &cooldown_samps );
  void GetReRaises(
    nloop_SampleSlice_t<bool,bankcount,chancount> &reraise_flags );


  void SetOneEnableFlag(int bankidx, int chanidx, bool want_enabled);
  void SetOnePulseDuration(int bankidx, int chanidx,
    indextype_t new_duration_samps);
  void SetOnePulseCooldown(int bankidx, int chanidx,
    indextype_t new_cooldown_samps);
  void SetOneReRaise(int bankidx, int chanidx, bool want_reraise);

  bool GetOneEnableFlag(int bankidx, int chanidx);
  indextype_t GetOnePulseDuration(int bankidx, int chanidx);
  indextype_t GetOnePulseCooldown(int bankidx, int chanidx);
  bool GetOneReRaise(int bankidx, int chanidx);
};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-trigger-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
