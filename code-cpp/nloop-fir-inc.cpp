// Attention Circuits Control Laboratory - NeuroLoop project
// FIR filter implementations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// nloop_FIRFilter_t Class


// Constructor.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
nloop_FIRFilter_t(void)
{
  BlankCoefficients();
}



// Process a linear buffer.
// NOTE - Elements [0]..[n-1] are read. y[0] is returned.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
samptype_t nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
ApplyFIROnceLinear(samptype_t *inbuf)
{
  samptype_t running_total;
  indextype_t cidx;

  running_total = 0;
  for (cidx = 0; cidx < coeffcount; cidx++)
    running_total += inbuf[cidx] * coeffs[cidx];

  NLOOP_ARITHSHR(running_total, fracbits);

  return running_total;
}



// Process a circular buffer.
// NOTE - Buffer size must be a power of two! The mask is used for
// wrapping. Elements [0]..[n-1] (modulo buffer length) are read.
// y[0] is returned.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
samptype_t nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
ApplyFIROnceCircular(
  samptype_t *inbuf, indextype_t inptr, indextype_t inbufmask)
{
  samptype_t running_total;
  indextype_t cidx;

  running_total = 0;
  for (cidx = 0; cidx < coeffcount; cidx++)
  {
    inptr &= inbufmask;
    running_total += inbuf[inptr] * coeffs[cidx];
    inptr++;
  }

  NLOOP_ARITHSHR(running_total, fracbits);

  return running_total;
}



// Blanking accessor.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
void nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
BlankCoefficients(void)
{
  indextype_t cidx;

  fracbits = 0;
  coeffcount = 0;

  for (cidx = 0; cidx < maxcoeffs; cidx++)
    coeffs[cidx] = 0;
}



// Fixed-point bit depth configuration.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
void nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
SetFracBits(uint8_t newbits)
{
  fracbits = newbits;
}



// Fixed-point bit depth accessor.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
uint8_t nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
GetFracBits(void)
{
  return fracbits;
}



// Coefficient count configuration.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
void nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
SetCoeffCount(indextype_t newcount)
{
  if (newcount < 0)
    newcount = 0;
  if (newcount > maxcoeffs)
    newcount = maxcoeffs;

  coeffcount = newcount;
}



// Coefficient count accessor.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
indextype_t nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
GetCoeffCount(void)
{
  return coeffcount;
}



// Coefficient accessors.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
void nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
SetOneCoefficient(indextype_t coeffidx, samptype_t coeffval)
{
  if ( (coeffidx >= 0) && (coeffidx < maxcoeffs) )
    coeffs[coeffidx] = coeffval;
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
samptype_t nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
GetOneCoefficient(indextype_t coeffidx)
{
  samptype_t result;

  result = 0;

  if ( (coeffidx >= 0) && (coeffidx < maxcoeffs) )
    result = coeffs[coeffidx];

  return result;
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
void nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
SetAllCoefficients(uint8_t newbits, indextype_t newcoeffcount,
  nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &newcoeffs)
{
  indextype_t cidx;

  // Copy everything, not just the used coefficients.
  for (cidx = 0; cidx < maxcoeffs; cidx++)
    coeffs[cidx] = newcoeffs.data[0][cidx];

  // Wrap the accessors to set configuration safely.
  SetFracBits(newbits);
  SetCoeffCount(newcoeffcount);
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs>
void nloop_FIRFilter_t<samptype_t, indextype_t, maxcoeffs>::
GetAllCoefficients(uint8_t &oldbits, indextype_t &oldcoeffcount,
  nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &oldcoeffs)
{
  indextype_t cidx;

  // Copy everything, not just the used coefficients.
  for (cidx = 0; cidx < maxcoeffs; cidx++)
    oldcoeffs.data[0][cidx] = coeffs[cidx];

  // Copy configuration.
  oldbits = fracbits;
  oldcoeffcount = coeffcount;
}



//
// nloop_FIRFilterBank_t Class


// Constructor.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
nloop_FIRFilterBank_t(void)
{
  chans_active = 0;
  banks_active = 0;

  bufptr = 0;

  BlankAllFilters();
}



// This processes one sample.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
ApplyBankOnce( nloop_SampleSlice_t<samptype_t,1,chancount> &indata,
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &outdata )
{
  int bidx, cidx;
  int readidx;
  indextype_t bufmask;


  // Blank the entire output (active or inactive).

  outdata.SetUniformValue(0);


  // Copy this input sample.

  bufmask = buflen - 1;
  bufptr &= bufmask; // Shouldn't be needed but do it anyways.

  for (cidx = 0; cidx < chans_active; cidx++)
    inbufs[cidx][bufptr] = indata.data[0][cidx];

  bufptr++;
  bufptr &= bufmask;


  // Only process active channels/banks.

  for (bidx = 0; bidx < banks_active; bidx++)
  {
    readidx = bufptr;
    readidx -= firs[bidx].GetCoeffCount(); // Underflow is fine.
    readidx &= bufmask; // This wraps underflow around to a valid value.

    for (cidx = 0; cidx < chans_active; cidx++)
      outdata.data[bidx][cidx] =
        firs[bidx].ApplyFIROnceCircular(&(inbufs[cidx][0]), readidx, bufmask);
  }
}



// Channel and bank geometry accessors.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
int nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
GetActiveChans(void)
{
  return chans_active;
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
SetActiveChans(int new_chans)
{
  if (new_chans < 0)
    new_chans = 0;
  if (new_chans > chancount)
    new_chans = chancount;

  chans_active = new_chans;
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
int nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
GetActiveBanks(void)
{
  return banks_active;
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
SetActiveBanks(int new_banks)
{
  if (new_banks < 0)
    new_banks = 0;
  if (new_banks > bankcount)
    new_banks = bankcount;

  banks_active = new_banks;
}



// Filter blanking accessors.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
BlankAllFilters(void)
{
  int bidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    firs[bidx].BlankCoefficients();
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
BlankOneFilter(int banknum)
{
  if ( (banknum >= 0) && (banknum < bankcount) )
    firs[banknum].BlankCoefficients();
}



// Individual coefficient accessors.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
SetOneCoefficient(int banknum, indextype_t coeffidx, samptype_t coeffval)
{
  if ( (banknum >= 0) && (banknum < bankcount) )
    // Coeffidx is checked by the FIR's accessor.
    firs[banknum].SetOneCoefficient(coeffidx, coeffval);
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
samptype_t nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
GetOneCoefficient(int banknum, indextype_t coeffidx)
{
  samptype_t result;

  result = 0;

  if ( (banknum >= 0) && (banknum < bankcount) )
    // Coeffidx is checked by the FIR's accessor.
    result = firs[banknum].GetOneCoefficient(coeffidx);

  return result;
}



// Filter geometry accessors.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
SetOneGeometry(int banknum, uint8_t newfracbits, indextype_t newcoeffcount)
{
  if ( (banknum >= 0) && (banknum < bankcount) )
  {
    // FIR accessors check the other arguments.
    firs[banknum].SetFracBits(newfracbits);
    firs[banknum].SetCoeffCount(newcoeffcount);
  }
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
GetOneGeometry(int banknum, uint8_t &oldfracbits, indextype_t &oldcoeffcount)
{
  oldfracbits = 0;
  oldcoeffcount = 0;

  if ( (banknum >= 0) && (banknum < bankcount) )
  {
    oldfracbits = firs[banknum].GetFracBits();
    oldcoeffcount = firs[banknum].GetCoeffCount();
  }
}



// Whole-filter configuration accessors.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
SetBankCoefficients(int banknum, int newbits, indextype_t newcoeffcount,
  nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &newcoeffs)
{
  if ( (banknum >= 0) && (banknum < bankcount) )
    // FIR accessor checks the other arguments.
    firs[banknum].SetAllCoefficients(newbits, newcoeffcount, newcoeffs);
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
GetBankCoefficients(int banknum, int &oldbits, indextype_t &oldcoeffcount,
    nloop_SampleSlice_t<samptype_t,1,maxcoeffs> &oldcoeffs)
{
  oldbits = 0;
  oldcoeffcount = 0;
  oldcoeffs.SetUniformValue(0);

  if ( (banknum >= 0) && (banknum < bankcount) )
    firs[banknum].GetAllCoefficients(oldbits, oldcoeffcount, oldcoeffs);
}



// Input buffer accessors.

template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
BlankAllInputBuffers(void)
{
  int cidx;
  indextype_t sidx;

  bufptr = 0;

  for (cidx = 0; cidx < chancount; cidx++)
    for (sidx = 0; sidx < buflen; sidx++)
      inbufs[cidx][sidx] = 0;
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
BlankOneInputBuffer(int channum)
{
  indextype_t sidx;

  if ( (channum >= 0) && (channum < chancount) )
  {
    // Leave the buffer read/write pointer where it is.

    for (sidx = 0; sidx < buflen; sidx++)
      inbufs[channum][sidx] = 0;
  }
}



template <class samptype_t, class indextype_t, indextype_t maxcoeffs,
  indextype_t buflen, int bankcount, int chancount>
void nloop_FIRFilterBank_t
<samptype_t, indextype_t, maxcoeffs, buflen, bankcount, chancount>::
FastSettleBuffers(nloop_SampleSlice_t<samptype_t,1,chancount> &indata)
{
  int cidx;
  indextype_t sidx;
  samptype_t thisval;

  // Copy all channels, active or not.

  bufptr = 0;

  for (cidx = 0; cidx < chancount; cidx++)
  {
    thisval = indata.data[0][cidx];
    for (sidx = 0; sidx < buflen; sidx++)
      inbufs[cidx][sidx] = thisval;
  }
}



//
// This is the end of the file.
