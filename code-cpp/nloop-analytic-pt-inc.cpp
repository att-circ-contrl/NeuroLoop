// Attention Circuits Control Laboratory - NeuroLoop project
// Analytic function proxy implementations - Peak and trough based.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// Peak, trough, and zero-crossing analytic estimator.


// Constructor.

template <class samptype_t, class indextype_t>
nloop_Analytic_PTZC_t<samptype_t,indextype_t>::
nloop_Analytic_PTZC_t(void)
{
  ResetState();
}



// This resets the feature identification state.
// NOTE - This also resets zero level (to 0) and minimum period (to maxval).

template <class samptype_t, class indextype_t>
void nloop_Analytic_PTZC_t<samptype_t,indextype_t>::
ResetState(void)
{
  // Configuration.
  // NOTE - With unsigned samptype_t, negative values wrap around.
  zero_level = 0;
  // NOTE - This initial value means we'll never detect zero-crossings.
  min_zc_gap = NLOOP_MAXVAL(indextype_t);

  // State.
  max_mag_seen = 0;
  last_mag = 0;
  since_rise_count = 0;
  since_fall_count = 0;
  last_period = 0;
}



// This sets the minimum period.
// This is used to prevent spurious ZC detections from high-frequency noise.
// NOTE - This should be substantially smaller than the input signal's
// actual minimum period.

template <class samptype_t, class indextype_t>
void nloop_Analytic_PTZC_t<samptype_t,indextype_t>::
SetMinPeriod(indextype_t newminperiod)
{
  min_zc_gap = newminperiod >> 1;
}



// This sets the zero level.

template <class samptype_t, class indextype_t>
void nloop_Analytic_PTZC_t<samptype_t,indextype_t>::
SetZeroLevel(samptype_t newzero)
{
  zero_level = newzero;
}



// This steps processing for one sample.

template <class samptype_t, class indextype_t>
void nloop_Analytic_PTZC_t<samptype_t,indextype_t>::
HandleSample(samptype_t sampval)
{
  bool is_negative;
  samptype_t thismag;

  // Increment counters.
  since_rise_count++;
  since_fall_count++;

  // Level-shift to be zero-averaged.
  // NOTE - Unsigned samptype_t is expected to wrap around.
  sampval -= zero_level;

  // Compute sign and magnitude.
  if (NLOOP_ISSIGNED(samptype_t))
  {
    is_negative = (sampval < 0);
    thismag = (is_negative ? -sampval : sampval);
  }
  else
  {
    is_negative = NLOOP_UNSIGNED_ISNEG(sampval);
    thismag = sampval;
    if (is_negative)
    { NLOOP_UNSIGNED_NEGATE(thismag); }
  }

  // Update maximum magnitude tracking.
  // FIXME - Sudden loud high-frequency noise may perturb this, due to samples
  // after the ZC but before min_zc_gap contributing.
  if (thismag > max_mag_seen)
    max_mag_seen = thismag;


  // Check to see if we've found a new zero-crossing.

  if (since_rise_count > since_fall_count)
  {
    // We're in the negative lobe, looking for a rising crossing.
    if ( (!is_negative) && (since_fall_count >= min_zc_gap) )
    {
      // This is a rising crossing.

      // Update the period estimate.
      last_period = since_rise_count - since_fall_count;
      last_period <<= 1;

      // Record the magnitude and reset magnitude tracking.
      last_mag = max_mag_seen;
      max_mag_seen = thismag;

      // Record this edge.
      since_rise_count = 0;
    }
  }
  else
  {
    // We're in the positive lobe, looking for a falling crossing.
    if ( (is_negative) && (since_rise_count >= min_zc_gap) )
    {
      // This is a falling crossing.

      // Update the period estimate.
      last_period = since_fall_count - since_rise_count;
      last_period <<= 1;

      // Record the magnitude and reset magnitude tracking.
      last_mag = max_mag_seen;
      max_mag_seen = thismag;

      // Record this edge.
      since_fall_count = 0;
    }
  }
}



// This returns the current running estimate of analytic signal parameters.
// All durations are in samples.

template <class samptype_t, class indextype_t>
void nloop_Analytic_PTZC_t<samptype_t,indextype_t>::
GetEstimatedAnalytic(samptype_t &magnitude, indextype_t &period,
  indextype_t &since_rise_zc, indextype_t &since_fall_zc)
{
  // Copy the extracted parameters.
  magnitude = last_mag;
  period = last_period;
  since_rise_zc = since_rise_count;
  since_fall_zc = since_fall_count;

  // Don't compute any derived parameters. Let the caller do that.
  // Doing it here hides which parameters are direct (low error) and which
  // are derived (higher error).
}



//
// FIXME - Flank-crossing estimator NYI.



//
// Array of peak-and-trough analytic signal approximators, indexed by channel
// and bank.


// This resets estimator state (which also resets zero-level and geometry).

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
ResetState(void)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      estimators[bidx][cidx].ResetState();

  banks_active = bankcount;
  chans_active = chancount;
}



// This passes sample data to the estimators.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
HandleSamples(nloop_SampleSlice_t<samptype_t, bankcount, chancount> &indata)
{
  int bidx, cidx;

  for (bidx = 0; bidx < banks_active; bidx++)
    for (cidx = 0; cidx < chans_active; cidx++)
      estimators[bidx][cidx].HandleSample( indata.data[bidx][cidx] );
}



// This queries estimators for analytic signal parameters.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
GetEstimatedAnalytic(
  nloop_SampleSlice_t<samptype_t, bankcount, chancount> &outmagnitude,
  nloop_SampleSlice_t<indextype_t, bankcount, chancount> &outperiod,
  nloop_SampleSlice_t<indextype_t, bankcount, chancount> &since_rise_zc,
  nloop_SampleSlice_t<indextype_t, bankcount, chancount> &since_fall_zc
)
{
  int bidx, cidx;

  for (bidx = 0; bidx < banks_active; bidx++)
    for (cidx = 0; cidx < chans_active; cidx++)
      estimators[bidx][cidx].GetEstimatedAnalytic(
        outmagnitude.data[bidx][cidx], outperiod.data[bidx][cidx],
        since_rise_zc.data[bidx][cidx], since_fall_zc.data[bidx][cidx]
      );
}



// This queries the number of active channels.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
int nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
GetActiveChans(void)
{
  return chans_active;
}



// This sets the number of active channels.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
SetActiveChans(int new_chans)
{
  if (new_chans < 0)
    new_chans = 0;
  else if (new_chans > chancount)
    new_chans = chancount;

  chans_active = new_chans;
}



// This queries the number of active channels.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
int nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
GetActiveBanks(void)
{
  return banks_active;
}



// This sets the number of active banks.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
SetActiveBanks(int new_banks)
{
  if (new_banks < 0)
    new_banks = 0;
  else if (new_banks > bankcount)
    new_banks = bankcount;

  banks_active = new_banks;
}



// This sets the minimum period for the estimators associated with each bank.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
SetMinPeriods(nloop_SampleSlice_t<indextype_t,bankcount,1> &newminperiods)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      estimators[bidx][cidx].SetMinPeriod( newminperiods.data[bidx][0] );
}



// This sets the minimum period for the estimators associated with a single
// bank.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
SetOneMinPeriod(int bankidx, indextype_t newminperiod)
{
  int cidx;

  if ( (bankidx >= 0) && (bankidx < bankcount) )
    for (cidx = 0; cidx < chancount; cidx++)
      estimators[bankidx][cidx].SetMinPeriod( newminperiod );
}



// This sets the zero levels associated with each estimator.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
SetZeroLevels(nloop_SampleSlice_t<samptype_t,bankcount,chancount> &newzeros)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      estimators[bidx][cidx].SetZeroLevel( newzeros.data[bidx][cidx] );
}



// This sets the zero level for one specific estimator.

template <class samptype_t, class indextype_t, class estimator_t,
  int bankcount, int chancount>
void nloop_AnalyticBank_PT_t<samptype_t, indextype_t, estimator_t,
  bankcount, chancount>::
SetOneZeroLevel(int bankidx, int chanidx, samptype_t newzero)
{
  int cidx;

  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (chanidx >= 0) && (chanidx < chancount) )
      estimators[bankidx][chanidx].SetZeroLevel( newzero );
}



//
// This is the end of the file.
