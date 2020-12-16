// Attention Circuits Control Laboratory - NeuroLoop project
// Threshold-based detectors.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// nloop_Averager_t Class

// This computes a running average of a signal using a first-order
// exponential filter, and multiplies it by a coefficient.
// For higher filter orders, use an IIR biquad instead of this.
// This uses bit-shifting instead of division for speed.
// NOTE - You need at least max(avgbits,coeffbits) bits of headroom.
// NOTE - This assumes that unsigned samptype_t contains signed data (as is
// the case after band-pass filtering).


// This updates the running average based on the input.
// The approximate settling time is 2^avgbits samples.
// The output value is ( average * coeff / 2^coeffbits ).

template <class samptype_t, uint8_t coeffbits>
samptype_t nloop_Averager_t<samptype_t,coeffbits>::
UpdateAverage(samptype_t indata)
{
  samptype_t outval;

  // Subtract the previous average value and add the new sample.

  outval = running_sum;

  if (NLOOP_ISSIGNED(samptype_t))
  { NLOOP_ARITHSHR(outval, avgbits); }
  else
  { NLOOP_ARITHSHR_UNSIGNED(outval, avgbits); }

  running_sum -= outval;
  running_sum += indata;


  // Compute and return the current average value, multiplied by the
  // coefficient.

  outval = running_sum;

  if (NLOOP_ISSIGNED(samptype_t))
  { NLOOP_ARITHSHR(outval, avgbits); }
  else
  { NLOOP_ARITHSHR_UNSIGNED(outval, avgbits); }

  outval *= coeff;

  if (NLOOP_ISSIGNED(samptype_t))
  { NLOOP_ARITHSHR(outval, coeffbits); }
  else
  { NLOOP_ARITHSHR_UNSIGNED(outval, coeffbits); }

  return outval;
}



// Use InitAverage() to avoid startup transients.

template <class samptype_t, uint8_t coeffbits>
void nloop_Averager_t<samptype_t,coeffbits>::
InitAverage(samptype_t indata)
{
  running_sum = indata << avgbits;
}



template <class samptype_t, uint8_t coeffbits>
void nloop_Averager_t<samptype_t,coeffbits>::
SetCoeff(samptype_t new_coeff)
{
  coeff = new_coeff;
}



template <class samptype_t, uint8_t coeffbits>
void nloop_Averager_t<samptype_t,coeffbits>::
SetAvgBits(uint8_t new_avgbits)
{
  avgbits = new_avgbits;
}



//
// nloop_AveragerBank_t Class

// Bank version of nloop_Averager_t.


template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
nloop_AveragerBank_t(void)
{
  // Force a sane initial configuration.

  chans_active = chancount;
  banks_active = bankcount;

  // This will output 0.
  SetUniformCoeffs(0);
  // This will track the input with no low-pass filtering.
  SetUniformAvgBits(0);
}



// This only operates on active banks/channels.

template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
UpdateAverage(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &indata,
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &outdata )
{
  int bidx, cidx;

  for (bidx = 0; bidx < banks_active; bidx++)
    for (cidx = 0; cidx < chans_active; cidx++)
      outdata.data[bidx][cidx] =
        averagers[bidx][cidx].UpdateAverage( indata.data[bidx][cidx] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
InitAverage( nloop_SampleSlice_t<samptype_t,bankcount,chancount> &indata )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].InitAverage( indata.data[bidx][cidx] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
int nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
GetActiveChans(void)
{
  return chans_active;
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetActiveChans(int new_chans)
{
  if (new_chans < 0)
    new_chans = 0;
  else if (new_chans > chancount)
    new_chans = chancount;

  chans_active = new_chans;
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
int nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
GetActiveBanks(void)
{
  return banks_active;
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetActiveBanks(int new_banks)
{
  if (new_banks < 0)
    new_banks = 0;
  else if (new_banks > bankcount)
    new_banks = bankcount;

  banks_active = new_banks;
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetCoeffs( nloop_SampleSlice_t<samptype_t,bankcount,chancount> &new_coeffs )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetCoeff( new_coeffs.data[bidx][cidx] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetBankCoeffs( nloop_SampleSlice_t<samptype_t,bankcount,1> &new_coeffs )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetCoeff( new_coeffs.data[bidx][0] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetChanCoeffs( nloop_SampleSlice_t<samptype_t,1,chancount> &new_coeffs )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetCoeff( new_coeffs.data[0][cidx] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetUniformCoeffs(samptype_t new_coeff)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetCoeff( new_coeff );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetOneCoeff(int bankidx, int chanidx, samptype_t new_coeff)
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    averagers[bankidx][chanidx].SetCoeff( new_coeff );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetAvgBits( nloop_SampleSlice_t<uint8_t,bankcount,chancount> &new_avgbits )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetAvgBits( new_avgbits.data[bidx][cidx] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetBankAvgBits( nloop_SampleSlice_t<uint8_t,bankcount,1> &new_avgbits )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetAvgBits( new_avgbits.data[bidx][0] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetChanAvgBits( nloop_SampleSlice_t<uint8_t,1,chancount> &new_avgbits )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetAvgBits( new_avgbits.data[0][cidx] );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetUniformAvgBits(uint8_t new_avgbits)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      averagers[bidx][cidx].SetAvgBits( new_avgbits );
}



template <class samptype_t, uint8_t coeffbits, int bankcount, int chancount>
void nloop_AveragerBank_t<samptype_t,coeffbits,bankcount,chancount>::
SetOneAvgBits(int bankidx, int chanidx, uint8_t new_avgbits)
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
    averagers[bankidx][chanidx].SetAvgBits( new_avgbits );
}



//
// nloop_DeGlitcher_t Class

// This delays rising and falling edges by specified amounts, removing
// spurious brief pulses or drop-outs, but adding delay.


// This processes one input sample.

template <class indextype_t>
bool nloop_DeGlitcher_t<indextype_t>::ProcessSample(bool indata)
{
  if (last_output)
  {
    // Output was high.

    if (indata)
      // Still high.
      fall_countdown = fall_delay;
    else if (0 >= fall_countdown)
    {
      // Low and we're past the delay.
      last_output = false;
      rise_countdown = rise_delay;
    }
    else
      // Low but we can't report the change yet.
      fall_countdown--;
  }
  else
  {
    // Output was low.

    if (!indata)
      // Still low.
      rise_countdown = rise_delay;
    else if (0 >= rise_countdown)
    {
      // High and we're past the delay.
      last_output = true;
      fall_countdown = fall_delay;
    }
    else
      // High but we can't report the change yet.
      rise_countdown--;
  }

  return last_output;
}



// This sets the delays and resets the countdowns.

template <class indextype_t>
void nloop_DeGlitcher_t<indextype_t>::
SetDelays(indextype_t new_rise_delay, indextype_t new_fall_delay)
{
  rise_delay = new_rise_delay;
  fall_delay = new_fall_delay;
  rise_countdown = 0;
  fall_countdown = 0;
  last_output = false;
}



//
// nloop_DeGlitcherBank_t Class

// Bank version of nloop_DeGlitcher_t.


// This processes one input slice.

template <class indextype_t, int bankcount, int chancount>
void nloop_DeGlitcherBank_t<indextype_t,bankcount,chancount>::
ProcessSample( nloop_SampleSlice_t<bool,bankcount,chancount> &indata,
  nloop_SampleSlice_t<bool,bankcount,chancount> &outdata )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      outdata.data[bidx][cidx] =
        deglitchers[bidx][cidx].ProcessSample( indata.data[bidx][cidx] );
}



template <class indextype_t, int bankcount, int chancount>
void nloop_DeGlitcherBank_t<indextype_t,bankcount,chancount>::
SetDelays(
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &new_rise_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,chancount> &new_fall_delays )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      deglitchers[bidx][cidx].SetDelays( new_rise_delays.data[bidx][cidx],
        new_fall_delays.data[bidx][cidx] );
}



template <class indextype_t, int bankcount, int chancount>
void nloop_DeGlitcherBank_t<indextype_t,bankcount,chancount>::
SetBankDelays(
  nloop_SampleSlice_t<indextype_t,bankcount,1> &new_rise_delays,
  nloop_SampleSlice_t<indextype_t,bankcount,1> &new_fall_delays )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      deglitchers[bidx][cidx].SetDelays( new_rise_delays.data[bidx][0],
        new_fall_delays.data[bidx][0] );
}



template <class indextype_t, int bankcount, int chancount>
void nloop_DeGlitcherBank_t<indextype_t,bankcount,chancount>::
SetChanDelays(
  nloop_SampleSlice_t<indextype_t,1,chancount> &new_rise_delays,
  nloop_SampleSlice_t<indextype_t,1,chancount> &new_fall_delays )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      deglitchers[bidx][cidx].SetDelays( new_rise_delays.data[0][cidx],
        new_fall_delays.data[0][cidx] );
}



template <class indextype_t, int bankcount, int chancount>
void nloop_DeGlitcherBank_t<indextype_t,bankcount,chancount>::
SetUniformDelays( indextype_t new_rise_delay, indextype_t new_fall_delay )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      deglitchers[bidx][cidx].SetDelays( new_rise_delay, new_fall_delay );
}



template <class indextype_t, int bankcount, int chancount>
void nloop_DeGlitcherBank_t<indextype_t,bankcount,chancount>::
SetOneDelays( int bankidx, int chanidx,
  indextype_t new_rise_delay, indextype_t new_fall_delay )
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
      deglitchers[bankidx][chanidx].SetDelays( new_rise_delay,
        new_fall_delay );
}



//
// nloop_ThresholdSingleBank_t Class

// Single-threshold detector.
// Events happen when the signal rises above a specified threshold.
// This is a trivial element but it's used to build more complex elements.


// This returns true if and only if the sample is at or above the threshold.

template <class samptype_t, int bankcount, int chancount>
void nloop_ThresholdSingleBank_t<samptype_t,bankcount,chancount>::
TestSamples(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &indata,
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &thresholds,
  nloop_SampleSlice_t<bool,bankcount,chancount> &outflag )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      outflag.data[bidx][cidx] =
        ( indata.data[bidx][cidx] >= thresholds.data[bidx][cidx] );
}



//
// nloop_ThresholdDualBank_t Class

// Two-threshold detector.
// This combines boolean flags from two single-threshold detectors.
// Events start when the signal rises above a higher threshold, and stop
// when it falls below a lower threshold (i.e. detection has hysteresis).


// This resets internal state to "no events detected".

template <int bankcount, int chancount>
void nloop_ThresholdDualBank_t<bankcount,chancount>::ResetState(void)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      prev_state.data[bidx][cidx] = false;
}



// This returns true if and only if the sample rose to the turn-on
// threshold and has not yet fallen below the turn-off threshold.

template <int bankcount, int chancount>
void nloop_ThresholdDualBank_t<bankcount,chancount>::
TestDual(
  nloop_SampleSlice_t<bool,bankcount,chancount> &flag_activate,
  nloop_SampleSlice_t<bool,bankcount,chancount> &flag_sustain,
  nloop_SampleSlice_t<bool,bankcount,chancount> &outflag )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
    {
      outflag.data[bidx][cidx] = flag_activate.data[bidx][cidx]
        || ( prev_state.data[bidx][cidx] && flag_sustain.data[bidx][cidx] );
      prev_state.data[bidx][cidx] = outflag.data[bidx][cidx];
    }
}



//
// This is the end of the file.
