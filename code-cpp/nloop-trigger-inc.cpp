// Attention Circuits Control Laboratory - NeuroLoop project
// Trigger logic.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.



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
void nloop_TargetBankZC_SelectInputs
(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<bool,1,trigcount> &want_falling,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &rise_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &fall_delays,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &signals_out
)
{
  int bidx, cidx, tidx;

  for (tidx = 0; tidx < trigcount; tidx++)
  {
    bidx = src_banks.data[0][tidx];
    cidx = src_chans.data[0][tidx];

    if ( (bidx >= 0) && (bidx < bankcount)
      && (cidx >= 0) && (cidx < chancount) )
    {
      if (want_falling.data[0][tidx])
        signals_out.data[0][tidx] = fall_delays.data[bidx][cidx];
      else
        signals_out.data[0][tidx] = rise_delays.data[bidx][cidx];
    }
  }
}


//
// Phase target logic.

// This converts the nominal target phase fraction (0..255) into a
// delay in samples (frac * period / 256), for each trigger.

template <class indextype_t, int bankcount, int chancount, int trigcount>
void nloop_TargetBankPhase_SelectTargets
(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &periods,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &nominal_targets,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &targets_out
)
{
  int bidx, cidx, tidx;
  indextype_t thisval;

  for (tidx = 0; tidx < trigcount; tidx++)
  {
    bidx = src_banks.data[0][tidx];
    cidx = src_chans.data[0][tidx];

    if ( (bidx >= 0) && (bidx < bankcount)
      && (cidx >= 0) && (cidx < chancount) )
    {
      thisval = nominal_targets.data[0][tidx];
      thisval *= periods.data[bidx][cidx];
      thisval >>= 8;
      targets_out.data[0][tidx] = thisval;
    }
  }
}


//
// Zero-crossing plus phase target logic.

// This selects the rising delay, falling delay, or delay since phase 0
// from the specified bank and channel, and either copies the target
// delay or converts a nominal target phase fraction (0..255) into a
// delay in samples (frac * period / 256), for each trigger.
// "want_phase" takes priority over "want_falling".

template <class indextype_t, int bankcount, int chancount, int trigcount>
void nloop_TargetBankZCPhase_SelectInputsAndTargets
(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<bool,1,trigcount> &want_phase,
  nloop_SampleSlice_t<bool,1,trigcount> &want_falling,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &rise_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &fall_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &phases,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &periods,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &signals_out,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &nominal_targets,
  nloop_SampleSlice_t<indextype_t,1,trigcount> &targets_out
)
{
  int bidx, cidx, tidx;
  indextype_t thisval;

  for (tidx = 0; tidx < trigcount; tidx++)
  {
    bidx = src_banks.data[0][tidx];
    cidx = src_chans.data[0][tidx];

    if ( (bidx >= 0) && (bidx < bankcount)
      && (cidx >= 0) && (cidx < chancount) )
    {
      if (want_phase.data[0][tidx])
      {
        signals_out.data[0][tidx] = phases.data[bidx][cidx];

        thisval = nominal_targets.data[0][tidx];
        thisval *= periods.data[bidx][cidx];
        thisval >>= 8;
        targets_out.data[0][tidx] = thisval;
      }
      else
      {
        if (want_falling.data[0][tidx])
          signals_out.data[0][tidx] = fall_delays.data[bidx][cidx];
        else
          signals_out.data[0][tidx] = rise_delays.data[bidx][cidx];

        targets_out.data[0][tidx] = nominal_targets.data[0][tidx];
      }
    }
  }
}


//
// Conditional flag logic.

// This triggers depending on the states of two flags.
// Per-trigger configuration options are "A", "A and B", "A and not B".
template <int bankcount, int chancount, int trigcount>
void nloop_ConditionalFlagDual_SelectFlags
(
  nloop_SampleSlice_t<int,1,trigcount> &src_banks,
  nloop_SampleSlice_t<int,1,trigcount> &src_chans,
  nloop_SampleSlice_t<bool,1,trigcount> &want_secondary,
  nloop_SampleSlice_t<bool,1,trigcount> &negate_secondary,
  nloop_SampleSlice_t<bool,bankcount,chancount> &input_primary,
  nloop_SampleSlice_t<bool,bankcount,chancount> &input_secondary,
  nloop_SampleSlice_t<bool,1,trigcount> &output_flags
)
{
  int bidx, cidx, tidx;
  bool thisval, secondval;

  for (tidx = 0; tidx < trigcount; tidx++)
  {
    bidx = src_banks.data[0][tidx];
    cidx = src_chans.data[0][tidx];

    if ( (bidx >= 0) && (bidx < bankcount)
      && (cidx >= 0) && (cidx < chancount) )
    {
      thisval = input_primary.data[bidx][cidx];

      if (want_secondary.data[0][tidx])
      {
        secondval = input_secondary.data[bidx][cidx];

        if (negate_secondary.data[0][tidx])
          secondval = !secondval;

        thisval = thisval && secondval;
      }

      output_flags.data[0][tidx] = thisval;
    }
    else
      output_flags.data[0][tidx] = false;
  }
}



//
// Classes


//
// Individual trigger generator.


// Constructor.

template<class indextype_t>
nloop_Trigger_t<indextype_t>::nloop_Trigger_t(void)
{
  ResetState();
}



// This initializes state to sane values.

template<class indextype_t>
void nloop_Trigger_t<indextype_t>::ResetState(void)
{
  // Configuration state.

  // Duration and cooldown time are in samples.
  trig_duration = 1;
  trig_cooldown_time = 50;
  reraise_ok = false;


  // Transient state.
  ForceIdle();
}



// This forces state to "idle" and resets transient state to sane values.
// Configuration state is left intact.

template<class indextype_t>
void nloop_Trigger_t<indextype_t>::ForceIdle(void)
{
  state = NLOOP_TSTATE_IDLE;

  timeout_left = 0;
  saved_target = 0;
  prev_signal = 0;
  unwrap_offset = 0;
}



// This accepts a delay/phase sample and returns the current output state.
// This checks trigger_count_left before triggering and decrements it if
// a new trigger event is generated.

template<class indextype_t>
bool nloop_Trigger_t<indextype_t>::
ProcessSample(indextype_t thisval, indextype_t thistarget,
  indextype_t thisperiod, bool thisdetectflag,
  indextype_t &trigger_count_left)
{
  switch (state)
  {
    case NLOOP_TSTATE_WAITRISE:
      // Pulse has been triggered but is not yet active.
      // Wait until the signal reaches the saved target value.

      // Unwrap if necessary. Threshold is half the period.
      // Period is known to be non-negative, so shifting is safe.

      // Add the unwrapping offset here, to save repeating (thisval + offset).
      thisval += unwrap_offset;

      if ( (thisval + (thisperiod >> 1)) < prev_signal )
      {
        unwrap_offset += thisperiod;
        thisval += thisperiod;
      }

      // Update unwrap tracking.
      prev_signal = thisval;

      // Compare the unwrapped signal with the saved target value.
      if ( thisval >= saved_target )
      {
        timeout_left = trig_duration;
        state = NLOOP_TSTATE_WAITFALL;
      }
      break;

    case NLOOP_TSTATE_WAITFALL:
      // Pulse is active.
      if (timeout_left > 0)
        timeout_left--;
      if (timeout_left <= 0)
      {
        timeout_left = trig_cooldown_time;
        state = NLOOP_TSTATE_WAITCOOL;
      }
      break;

    case NLOOP_TSTATE_WAITCOOL:
      // Cooldown period after a pulse.
      if (timeout_left > 0)
        timeout_left--;

      // If we've cooled down look for a new rising edge on the trigger flag.
      if (timeout_left <= 0)
        // Only do this if detection is de-asserted or re-raising is okay.
        if ( (!thisdetectflag) || reraise_ok )
          state = NLOOP_TSTATE_IDLE;
      break;

    default:
      // Assume NLOOP_TSTATE_IDLE.

      // If the detection flag is asserted and we have quota left, queue
      // a pulse.
      if ( thisdetectflag && (trigger_count_left > 0) )
      {
        trigger_count_left--;
        state = NLOOP_TSTATE_WAITRISE;

        // Figure out what we're looking for to trigger the pulse.
        saved_target = thistarget;
        // If we've passed the target, advance the target by one period.
        if (thisval >= saved_target)
          saved_target += thisperiod;
        // Check and advance a second time if necessary.
        // This can happen if calibration or other detector details push the
        // detected time to a value greater than the period.
        if (thisval >= saved_target)
          saved_target += thisperiod;

        // Reinitialize input unwrapping.
        unwrap_offset = 0;
        prev_signal = thisval;
      }
      break;
  }


  // Return true if a pulse is being asserted.
  return (NLOOP_TSTATE_WAITFALL == state);
}


// Accessors.

template<class indextype_t>
void nloop_Trigger_t<indextype_t>::
SetPulseDuration(indextype_t new_duration_samps)
{
  trig_duration = new_duration_samps;
  if (trig_duration < 1)
    trig_duration = 1;
}


template<class indextype_t>
void nloop_Trigger_t<indextype_t>::
SetPulseCooldown(indextype_t new_cooldown_samps)
{
  trig_cooldown_time = new_cooldown_samps;
  if (trig_cooldown_time < 1)
    trig_cooldown_time = 1;
}


template<class indextype_t>
void nloop_Trigger_t<indextype_t>::SetReRaise(bool want_reraise)
{
  reraise_ok = want_reraise;
}


template<class indextype_t>
indextype_t nloop_Trigger_t<indextype_t>::GetPulseDuration(void)
{
  return trig_duration;
}


template<class indextype_t>
indextype_t nloop_Trigger_t<indextype_t>::GetPulseCooldown(void)
{
  return trig_cooldown_time;
}


template<class indextype_t>
bool nloop_Trigger_t<indextype_t>::GetReRaise(void)
{
  return reraise_ok;
}



//
// Trigger generator bank.


// Constructor.

template<class indextype_t, int bankcount, int chancount>
nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
nloop_TriggerBank_t(void)
{
  ResetState();
}



// This initializes state to sane values.

template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
ResetState(void)
{
  int bidx, cidx;

  trigger_count_left = 0;
  window_time_left = 0;

  banks_active = 0;
  chans_active = 0;

  enabled.SetUniformValue(false);

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      triggers[bidx][cidx].ResetState();
}



// This forces state to "idle" and resets transient state to sane values.
// Configuration state is left intact.

template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
ForceIdle(void)
{
  int bidx, cidx;

  // Halt all triggering.
  trigger_count_left = 0;
  window_time_left = 0;

  // Reset individual triggers.
  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      triggers[bidx][cidx].ForceIdle();
}



// This resets the active triggering time window and trigger count,
// enabling triggering.

template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
EnableTriggering( indextype_t active_window_samps,
  indextype_t max_pulses_sent )
{
  window_time_left = active_window_samps;
  trigger_count_left = max_pulses_sent;
}



// This disables triggering, clearing the active triggering time window
// and trigger count. Triggers that are in progress will still complete.

template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
DisableTriggering(void)
{
  window_time_left = 0;
  trigger_count_left = 0;
}



// This accepts a delay/phase sample and returns the current output state.

template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
ProcessSamples(
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &sampvals,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &targetvals,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &periods,
  nloop_SampleSlice_t<bool,bankcount,chancount> &detectflags,
  nloop_SampleSlice_t<bool,bankcount,chancount> &trigsout )
{
  int bidx, cidx;
  bool thisout;

  // Reaching the end of the window drops the stimulation quota to zero.
  // We still have to call the update routine to finish pulses that are
  // in progress.

  if (window_time_left > 0)
    window_time_left--;
  else
    trigger_count_left = 0;


  // Process this sample slice.
  // Only scan active banks and channels.

  for (bidx = 0; bidx < banks_active; bidx++)
    for (cidx = 0; cidx < chans_active; cidx++)
    {
      thisout = false;

      if (enabled.data[bidx][cidx])
        // This checks and then updates trigger_count_left.
        thisout = triggers[bidx][cidx].ProcessSample(
          sampvals.data[bidx][cidx], targetvals.data[bidx][cidx],
          periods.data[bidx][cidx], detectflags.data[bidx][cidx],
          trigger_count_left );

      trigsout.data[bidx][cidx] = thisout;
    }
}



// Accessors.

template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetActiveBanks(int new_banks)
{
  if (new_banks < 0)
    new_banks = 0;
  else if (new_banks > bankcount)
    new_banks = bankcount;

  banks_active = new_banks;
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetActiveChans(int new_chans)
{
  if (new_chans < 0)
    new_chans = 0;
  else if (new_chans > chancount)
    new_chans = chancount;

  chans_active = new_chans;
}


template<class indextype_t, int bankcount, int chancount>
int nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetActiveBanks(void)
{
  return banks_active;
}


template<class indextype_t, int bankcount, int chancount>
int nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetActiveChans(void)
{
  return chans_active;
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetEnableFlags(
  nloop_SampleSlice_t<bool,bankcount,chancount> &want_enabled )
{
  enabled.CopyFrom(want_enabled);
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetPulseDurations(
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &duration_samps )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      triggers[bidx][cidx].SetPulseDuration(
        duration_samps.data[bidx][cidx] );
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetPulseCooldowns(
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &cooldown_samps )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      triggers[bidx][cidx].SetPulseCooldown(
        cooldown_samps.data[bidx][cidx] );
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetAllReRaises(bool want_reraise)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      triggers[bidx][cidx].SetReRaise(want_reraise);
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetEnableFlags(
  nloop_SampleSlice_t<bool,bankcount,chancount> &is_enabled )
{
  is_enabled.CopyFrom(enabled);
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetPulseDurations(
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &duration_samps )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      duration_samps.data[bidx][cidx] =
        triggers[bidx][cidx].GetPulseDuration();
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetPulseCooldowns(
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &cooldown_samps )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      cooldown_samps.data[bidx][cidx] =
        triggers[bidx][cidx].GetPulseCooldown();
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetReRaises(
  nloop_SampleSlice_t<bool,bankcount,chancount> &reraise_flags )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      reraise_flags.data[bidx][cidx] =
        triggers[bidx][cidx].GetReRaise();
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetOneEnableFlag(int bankidx, int chanidx, bool want_enabled)
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
      enabled.data[bankidx][chanidx] = want_enabled;
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetOnePulseDuration(int bankidx, int chanidx,
  indextype_t new_duration_samps)
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    triggers[bankidx][chanidx].SetPulseDuration(new_duration_samps);
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetOnePulseCooldown(int bankidx, int chanidx,
  indextype_t new_cooldown_samps)
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    triggers[bankidx][chanidx].SetPulseCooldown(new_cooldown_samps);
}


template<class indextype_t, int bankcount, int chancount>
void nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
SetOneReRaise(int bankidx, int chanidx, bool want_reraise)
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    triggers[bankidx][chanidx].SetReRaise(want_reraise);
}


template<class indextype_t, int bankcount, int chancount>
bool nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetOneEnableFlag(int bankidx, int chanidx)
{
  bool result;

  result = false;

  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
      result = enabled.data[bankidx][chanidx];

  return result;
}


template<class indextype_t, int bankcount, int chancount>
indextype_t nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetOnePulseDuration(int bankidx, int chanidx)
{
  indextype_t result;

  result = 0;

  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    result = triggers[bankidx][chanidx].GetPulseDuration();

  return result;
}


template<class indextype_t, int bankcount, int chancount>
indextype_t nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetOnePulseCooldown(int bankidx, int chanidx)
{
  indextype_t result;

  result = 0;

  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    result = triggers[bankidx][chanidx].GetPulseCooldown();

  return result;
}


template<class indextype_t, int bankcount, int chancount>
bool nloop_TriggerBank_t<indextype_t,bankcount,chancount>::
GetOneReRaise(int bankidx, int chanidx)
{
  bool result;

  result = false;

  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    result = triggers[bankidx][chanidx].GetReRaise();

  return result;
}



//
// This is the end of the file.
