// Attention Circuits Control Laboratory - NeuroLoop project
// Preprocessing modules - Templated functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// Auto-ranging module.
//
// This monitors the range of the input and computes an attenuation bit-shift
// and offset to make it fit in a user-specified range.
// The mapping used is:  outval = (inval >> attenbits) + oset
// Offset may be positive or negative.


//
// Helper functions.


// This recalculates the running attenuation and offset values.
// The FPGA-based version would do this for every sample. The C++ version
// only calls this when the result is needed.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
RecalcAttenOffset(void)
{
  int cidx;
  samptype_t thismin, thismax;
  samptype_t thismiddle, thishalfspan;
  uint8_t thisatten;

  for (cidx = 0; cidx < chancount; cidx++)
  {
    thismin = minvals[cidx];
    thismax = maxvals[cidx];

    // Force sanity.
    // If we reset tracking and haven't yet seen samples, this case happens.
    if (thismax < thismin)
      thismax = thismin;

    // FIXME - We have to handle the case where the signal bounds approach
    // the type's minimum and maximum values.
    // Do this by dividing the measured limits by 2.
    // The calculated offset may be off by 1; this is acceptable.

    NLOOP_ARITHSHR(thismin, 1);
    NLOOP_ARITHSHR(thismax, 1);

    // Middle calculation is fine as-is. (A/2 + B/2) = (A + B)/2.
    thismiddle = thismin + thismax;
    // This is half the real range, to guarantee fitting in samptype_t.
    thishalfspan = thismax - thismin;


    // Calculate and store the attenuation.

    thisatten = 0;

    while (thishalfspan > halfspan_wanted)
    {
      thisatten++;
      // This should always be a positive value, so logical shift is fine.
      thishalfspan >>= 1;
    }

    running_attens[cidx] = thisatten;


    // Calculate and store the offset.

    NLOOP_ARITHSHR(thismiddle, thisatten);

    // Subtracting is always fine. With unsigned arguments, we wrap around.
    running_offsets[cidx] = middle_wanted - thismiddle;
  }
}



// This computes the attenuated and shifted value of the input.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::CalcOutput(
    nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    nloop_SampleSlice_t<samptype_t, 1, chancount> &outdata,
    bool use_latched)
{
  int cidx;
  samptype_t thisval, thisoset;
  uint8_t thisatten, groupatten;


  // If we want running output, recalculate the attenuation and offset.
  // The FPGA implementation does this for every sample. The C++ version
  // does this only when attenuation and offset are needed, as it's slow.
  if (!use_latched)
    RecalcAttenOffset();


  // Calculate the "tied" attenuation. This is the maximum of per-channel
  // attenuation values.

  groupatten = 0;
  for (cidx = 0; cidx < chancount; cidx++)
  {
    if (use_latched)
      thisatten = latched_attens[cidx];
    else
      thisatten = running_attens[cidx];

    if (thisatten > groupatten)
      groupatten = thisatten;
  }


  // Walk through channels, performing attenuation and shifting.

  for (cidx = 0; cidx < chancount; cidx++)
  {
    // Figure out our attenuation and offset.

    if (use_latched)
    {
      thisoset = latched_offsets[cidx];
      thisatten = latched_attens[cidx];
    }
    else
    {
      thisoset = running_offsets[cidx];
      thisatten = running_attens[cidx];
    }

    if (atten_tied)
      thisatten = groupatten;


    // Perform attenuation and shifting.

    thisval = indata.data[0][cidx];

    thisval >>= thisatten;
    // For unsigned values, this will wrap around to implement negative
    // offsets.
    thisval += thisoset;

    outdata.data[0][cidx] = thisval;
  }


  // Done.
}



//
// Constructor.


template<class samptype_t, class indextype_t, int chancount>
nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
nloop_AutoRanger_t(void)
{
  countdown_active = false;
  latch_countdown = 0;

  atten_tied = false;

  SetDesiredRange( NLOOP_MINVAL(samptype_t), NLOOP_MAXVAL(samptype_t) );

  // This resets the observed maximum/minimum values.
  ResetTracking(atten_tied);

  // This sets latched attenuation and offset to "do nothing" values.
  ResetLatched();
}


// Default destructor is fine.



//
// Processing functions.


// This updates the internal state used to calculate attenuation and
// offset.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
UpdateFromSample(nloop_SampleSlice_t<samptype_t, 1, chancount> &data)
{
  int cidx;
  samptype_t thisval;

  // Update observed minimum and maximum.
  for (cidx = 0; cidx < chancount; cidx++)
  {
    thisval = data.data[0][cidx];

    if (thisval < minvals[cidx])
      minvals[cidx] = thisval;

    if (thisval > maxvals[cidx])
      maxvals[cidx] = thisval;
  }

  // Update latching state.
  if (countdown_active)
  {
    if (latch_countdown > 0)
      latch_countdown--;
    else
    {
      countdown_active = false;
      latch_countdown = 0;

      RecalcAttenOffset();
      for (cidx = 0; cidx < chancount; cidx++)
      {
        latched_offsets[cidx] = running_offsets[cidx];
        latched_attens[cidx] = running_attens[cidx];
      }
    }
  }

  // Done.
}



// This computes transformed output using the running attenuation and offset.
// This does NOT update the internal state.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
GetRunningOutput(nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
  nloop_SampleSlice_t<samptype_t, 1, chancount> &outdata)
{
  // Wrap the helper function.
  CalcOutput(indata, outdata, false);
}



// This computes transformed output using the latched attenuation and offset.
// This does NOT update the internal state.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
GetLatchedOutput(nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
  nloop_SampleSlice_t<samptype_t, 1, chancount> &outdata)
{
  // Wrap the helper function.
  CalcOutput(indata, outdata, true);
}



//
// Accessors.


// This reinitializes maximum and minimum tracking.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
ResetTracking(bool want_shared_atten)
{
  int cidx;

  for (cidx = 0; cidx < chancount; cidx++)
  {
    // Set these to values that any sample will modify.
    // Observed minimum is maxval, and vice versa.
    minvals[cidx] = NLOOP_MAXVAL(samptype_t);
    maxvals[cidx] = NLOOP_MINVAL(samptype_t);
  }
}



// This resets the latched attenuation and offset to "do nothing" values.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
ResetLatched(void)
{
  int cidx;

  for (cidx = 0; cidx < chancount; cidx++)
  {
    latched_offsets[cidx] = 0;
    latched_attens[cidx] = 0;
  }
}



// This queues a latching operation.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
LatchAfter(indextype_t sampcount)
{
  latch_countdown = sampcount;
  countdown_active = true;
}



// This indicates whether a future latch operation is queued.

template<class samptype_t, class indextype_t, int chancount>
bool nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
IsAutoRangeRunning(void)
{
  return countdown_active;
}



// This updates the user-specified target range.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
SetDesiredRange(samptype_t newmin, samptype_t newmax)
{
  samptype_t scratchmin, scratchmax;

  // FIXME - We have to handle the case where the user specifies the
  // type minimum and maximum as goal values.
  // We do this ourselves in the constructor.

  // Do this by dividing the supplied limits by 2.
  // The calculated offset may be off by 1; this is acceptable.

  scratchmin = newmin;
  NLOOP_ARITHSHR(scratchmin, 1);
  scratchmax = newmax;
  NLOOP_ARITHSHR(scratchmax, 1);

  // Force sanity.
  if (scratchmax < scratchmin)
    scratchmax = scratchmin;

  // This is fine as-is. (A/2 + B/2) = (A + B)/2.
  middle_wanted = scratchmin + scratchmax;

  // Store half the span, rather than the full span, to guarantee that it
  // fits in range.
  halfspan_wanted = scratchmax - scratchmin;
}



//
// Debugging accessors.


// This queries the minimum values seen in the input.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
GetMinValuesSeen(nloop_SampleSlice_t<samptype_t, 1, chancount> &data)
{
  int cidx;

  for (cidx = 0; cidx < chancount; cidx++)
    data.data[0][cidx] = minvals[cidx];
}



// This queries the maximum values seen in the input.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
GetMaxValuesSeen(nloop_SampleSlice_t<samptype_t, 1, chancount> &data)
{
  int cidx;

  for (cidx = 0; cidx < chancount; cidx++)
    data.data[0][cidx] = maxvals[cidx];
}



// This returns the running attenuation and offset values.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
GetRunningAttenOffset(
  nloop_SampleSlice_t<samptype_t,1,chancount> &bitshifts,
  nloop_SampleSlice_t<samptype_t,1,chancount> &offsets)
{
  int cidx;

  // Update the running values.
  RecalcAttenOffset();

  // Report the running values.
  for (cidx = 0; cidx < chancount; cidx++)
  {
    offsets.data[0][cidx] = running_offsets[cidx];
    // Use an explicit cast in case we're turning uint8_t into int8_t.
    bitshifts.data[0][cidx] = (samptype_t) (running_attens[cidx]);
  }
}



// This returns the latched attenuation and offset values.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
GetLatchedAttenOffset(
  nloop_SampleSlice_t<samptype_t,1,chancount> &bitshifts,
  nloop_SampleSlice_t<samptype_t,1,chancount> &offsets)
{
  int cidx;

  // Report the latched values.
  for (cidx = 0; cidx < chancount; cidx++)
  {
    offsets.data[0][cidx] = latched_offsets[cidx];
    // Use an explicit cast in case we're turning uint8_t into int8_t.
    bitshifts.data[0][cidx] = (samptype_t) (latched_attens[cidx]);
  }
}



// This manually latches the specified attenuation and offset values.

template<class samptype_t, class indextype_t, int chancount>
void nloop_AutoRanger_t<samptype_t,indextype_t,chancount>::
SetAttenOffset(
  nloop_SampleSlice_t<samptype_t,1,chancount> &bitshifts,
  nloop_SampleSlice_t<samptype_t,1,chancount> &offsets)
{
  int cidx;

  // Modify the latched values.
  for (cidx = 0; cidx < chancount; cidx++)
  {
    latched_offsets[cidx] = offsets.data[0][cidx];
    // Blithely assume that the input value is in a sensible range.
    latched_attens[cidx] = (uint8_t) (bitshifts.data[0][cidx]);
  }
}



//
// This is the end of the file.
