// Attention Circuits Control Laboratory - NeuroLoop project
// Biquad filter implementation.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instatiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// nloop_IIRBiquad_t Class


// Process linear buffers.
// NOTE - Elements [0], [-1], and [-2] are read/written.

template <class samptype_t, class indextype_t>
void nloop_IIRBiquad_t<samptype_t, indextype_t>::ApplyBiquadOnceLinear(
  samptype_t *inbuf, samptype_t *outbuf)
{
  samptype_t innow, inprev1, inprev2, outprev1, outprev2;
  samptype_t outnow;

  innow = *inbuf;
  inprev1 = *(inbuf - 1);
  inprev2 = *(inbuf - 2);
  outprev1 = *(outbuf - 1);
  outprev2 = *(outbuf - 2);

  outnow = num0 * innow;
  outnow += num1 * inprev1;
  outnow += num2 * inprev2;
  outnow -= den1 * outprev1;
  outnow -= den2 * outprev2;

  // FIXME - This will be very slow for unsigned operands!
  // Use pointer tricks to treat it as signed instead.
  if (NLOOP_ISSIGNED(samptype_t))
  { NLOOP_ARITHSHR(outnow, den0_bits); }
  else
  { NLOOP_ARITHSHR_UNSIGNED(outnow, den0_bits); }

  *outbuf = outnow;
}



// Process circular buffers.
// NOTE - Buffer size must be a power of two! The mask is used for wrapping.
// Elements [n], [n-1], and [n-2] are read/written.

template <class samptype_t, class indextype_t>
void nloop_IIRBiquad_t<samptype_t, indextype_t>::ApplyBiquadOnceCircular(
  samptype_t *inbuf, indextype_t inptr, indextype_t inbufmask,
  samptype_t *outbuf, indextype_t outptr, indextype_t outbufmask )
{
  samptype_t innow, inprev1, inprev2, outprev1, outprev2;
  samptype_t outnow;
  indextype_t saved_outptr;

  // Remember that adding the mask value is equivalent to adding -1.

  innow = inbuf[inptr];
  inptr = (inptr + inbufmask) & inbufmask;
  inprev1 = inbuf[inptr];
  inptr = (inptr + inbufmask) & inbufmask;
  inprev2 = inbuf[inptr];

  saved_outptr = outptr;

  outptr = (outptr + outbufmask) & outbufmask;
  outprev1 = outbuf[outptr];
  outptr = (outptr + outbufmask) & outbufmask;
  outprev2 = outbuf[outptr];


  outnow = num0 * innow;
  outnow += num1 * inprev1;
  outnow += num2 * inprev2;
  outnow -= den1 * outprev1;
  outnow -= den2 * outprev2;

  // FIXME - This will be very slow for unsigned operands!
  // Use pointer tricks to treat it as signed instead.
  if (NLOOP_ISSIGNED(samptype_t))
  { NLOOP_ARITHSHR(outnow, den0_bits); }
  else
  { NLOOP_ARITHSHR_UNSIGNED(outnow, den0_bits); }


  outbuf[saved_outptr] = outnow;
}



// Read accessor.

template <class samptype_t, class indextype_t>
void nloop_IIRBiquad_t<samptype_t, indextype_t>::GetCoefficients(
  uint8_t &old_den0bits, samptype_t &old_den1, samptype_t &old_den2,
  samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2 )
{
  old_den0bits = den0_bits;
  old_den1 = den1;
  old_den2 = den2;

  old_num0 = num0;
  old_num1 = num1;
  old_num2 = num2;
}



// Write accessor.

template <class samptype_t, class indextype_t>
void nloop_IIRBiquad_t<samptype_t, indextype_t>::SetCoefficients(
  uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
  samptype_t new_num0, samptype_t new_num1, samptype_t new_num2 )
{
  den0_bits = new_den0bits;
  den1 = new_den1;
  den2 = new_den2;

  num0 = new_num0;
  num1 = new_num1;
  num2 = new_num2;
}



//
// nloop_IIRBiquadChain_t Class


// Process buffers.
// This will still work with zero active stages (copying input to output).
// NOTE - This only reads the addressed elements. History is kept in internal
// buffers. As a result, this takes time to stabilize.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain_t<samptype_t, indextype_t, stagecount>::
  ApplyChainOnce(samptype_t &indata, samptype_t &outdata)
{
  int sidx;

  // Copy the latest sample.
  buffers[0][bufptr] = indata;

  // Run the filter chain.
  for (sidx = 0; sidx < stages_active; sidx++)
    biquads[sidx].ApplyBiquadOnceCircular(
      buffers[sidx], bufptr, NLOOP_IIRBIQUADCHAIN_BUFSIZE - 1,
      buffers[sidx+1], bufptr, NLOOP_IIRBIQUADCHAIN_BUFSIZE - 1
    );

  // Copy the output.
  outdata = buffers[stages_active][bufptr];

  // Increment the buffer pointer.
  bufptr++;
  bufptr &= (NLOOP_IIRBIQUADCHAIN_BUFSIZE - 1);
}



// Read the number of active stages.

template <class samptype_t, class indextype_t, int stagecount>
int nloop_IIRBiquadChain_t<samptype_t, indextype_t, stagecount>::
  GetActiveStages(void)
{
  return stages_active;
}



// Set the number of active stages.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain_t<samptype_t, indextype_t, stagecount>::
  SetActiveStages(int new_stages)
{
  if (new_stages < 0)
    new_stages = 0;
  else if (new_stages > stagecount)
    new_stages = stagecount;

  stages_active = new_stages;
}




// Read biquad coefficients for one stage.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain_t<samptype_t, indextype_t, stagecount>::
  GetCoefficients( int stagenum,
  uint8_t &old_den0bits, samptype_t &old_den1, samptype_t &old_den2,
  samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2 )
{
  old_den0bits = 0;
  old_den1 = 0;
  old_den2 = 0;
  old_num0 = 0;
  old_num1 = 0;
  old_num2 = 0;

  if ((0 <= stagenum) && (stagecount > stagenum))
    biquads[stagenum].GetCoefficients( old_den0bits, old_den1, old_den2,
      old_num0, old_num1, old_num2 );
}



// Write biquad coefficients for one stage.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain_t<samptype_t, indextype_t, stagecount>::
  SetCoefficients( int stagenum,
  uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
  samptype_t new_num0, samptype_t new_num1, samptype_t new_num2 )
{
  if ((0 <= stagenum) && (stagecount > stagenum))
    biquads[stagenum].SetCoefficients( new_den0bits, new_den1, new_den2,
      new_num0, new_num1, new_num2 );
}



// Stuff all layers of the internal buffers with "settled" values.
// Stages are either stuffed with the input value (suitable for low-pass
// stages) or with zero (suitable for high-pass and band-pass stages).

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain_t<samptype_t, indextype_t, stagecount>::
  FastSettleBuffers(samptype_t &indata, bool (&copy_input)[stagecount])
{
  int stidx, vidx;
  samptype_t thisdata;

  // Copy the input into buffer 0 (that's what it normally holds).
  for (vidx = 0; vidx < NLOOP_IIRBIQUADCHAIN_BUFSIZE; vidx++)
    buffers[0][vidx] = indata;


  // For subsequent stages, either copy the input or write zero.
  for (stidx = 0; stidx < stagecount; stidx++)
  {
    thisdata = 0;
    if (copy_input[stidx])
      thisdata = indata;

    // Output buffers are 1..stagecount.
    for (vidx = 0; vidx < NLOOP_IIRBIQUADCHAIN_BUFSIZE; vidx++)
      buffers[stidx + 1][vidx] = thisdata;
  }
}



//
// nloop_IIRFilterBank_t class.


// Process buffers.
// This will still work with zero active stages (copying input to output).
// NOTE - This only manipulates active channels and banks. Unused parts of
// the output array will get stale.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
void nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  ApplyBankOnce(
    nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    nloop_SampleSlice_t<samptype_t, bankcount, chancount> &outdata
  )
{
  int bidx, cidx;

  for (bidx = 0; bidx < banks_active; bidx++)
    for (cidx = 0; cidx < chans_active; cidx++)
      biquads[bidx][cidx].ApplyChainOnce( indata.data[0][cidx],
        outdata.data[bidx][cidx] );
}



// Read the number of active stages.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
int nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  GetActiveStages(void)
{
  // Read from the first biquad chain. They all should be the same.
  return biquads[0][0].GetActiveStages();
}



// Set the number of active stages.
// This is set for all channels and banks, not just active ones.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
void nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  SetActiveStages(int new_stages)
{
  int bidx, cidx;

  if (new_stages < 0)
    new_stages = 0;
  else if (new_stages > stagecount)
    new_stages = stagecount;

  // Adjust all biquad chains.
  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      biquads[bidx][cidx].SetActiveStages(new_stages);
}



// Read the number of active channels.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
int nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  GetActiveChans(void)
{
  return chans_active;
}



// Set the number of active channels.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
void nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  SetActiveChans(int new_chans)
{
  if (new_chans < 0)
    new_chans = 0;
  else if (new_chans > chancount)
    new_chans = chancount;

  chans_active = new_chans;
}



// Read the number of active banks.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
int nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  GetActiveBanks(void)
{
  return banks_active;
}



// Set the number of active banks.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
void nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  SetActiveBanks(int new_banks)
{
  if (new_banks < 0)
    new_banks = 0;
  else if (new_banks > bankcount)
    new_banks = bankcount;

  banks_active = new_banks;
}



// Read filter coefficients for one bank.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
void nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  GetCoefficients( int stagenum, int banknum,
    uint8_t &old_den0bits, samptype_t &old_den1, samptype_t &old_den2,
    samptype_t &old_num0, samptype_t &old_num1, samptype_t &old_num2 )
{
  if ((banknum >= 0) && (banknum < bankcount))
    // Read from the first channel. They should all be the same for this bank.
    biquads[banknum][0].GetCoefficients( stagenum,
      old_den0bits, old_den1, old_den2, old_num0, old_num1, old_num2 );
}



// Set filter coefficients for one bank.
// Coefficients are updated for all channels, not just active channels.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
void nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  SetCoefficients( int stagenum, int banknum,
    uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
    samptype_t new_num0, samptype_t new_num1, samptype_t new_num2 )
{
  int cidx;

  if ((banknum >= 0) && (banknum < bankcount))
  {
    // Set coefficients for all channels in this bank.
    for (cidx = 0; cidx < chancount; cidx++)
      biquads[banknum][0].SetCoefficients( stagenum,
        new_den0bits, new_den1, new_den2, new_num0, new_num1, new_num2 );
  }
}



// Stuff all layers of the internal buffers with "settled" values.
// Stages are either stuffed with the input value (suitable for low-pass
// stages) or with zero (suitable for high-pass and band-pass stages).
// This updates all channels, not just active channels.

template <class samptype_t, class indextype_t,
  int stagecount, int bankcount, int chancount>
void nloop_IIRFilterBank_t<samptype_t, indextype_t,
  stagecount, bankcount, chancount>::
  FastSettleBuffers(
    nloop_SampleSlice_t<samptype_t, 1, chancount> &indata,
    bool (&copy_input)[stagecount]
  )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      biquads[bidx][cidx].FastSettleBuffers( indata.data[0][cidx],
        copy_input );
}



//
// This is the end of the file.
