// Attention Circuits Control Laboratory - NeuroLoop project
// Biquad filter implementation.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instatiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at compile-time.


//
// nloop_IIRBiquad Class


// Process linear buffers.
// NOTE - Elements [0], [-1], and [-2] are read/written.

template <class samptype_t, class indextype_t>
void nloop_IIRBiquad<samptype_t, indextype_t>::ApplyBiquadOnceLinear(
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
  // NOTE - If you're using a signed type, you'd better have set compiler
  // flags to force arithmetic shifting!
  outnow >>= den0_bits;

  *outbuf = outnow;
}



// Process circular buffers.
// NOTE - Buffer size must be a power of two! The mask is used for wrapping.
// Elements [n], [n-1], and [n-2] are read/written.

template <class samptype_t, class indextype_t>
void nloop_IIRBiquad<samptype_t, indextype_t>::ApplyBiquadOnceCircular(
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
  // NOTE - If you're using a signed type, you'd better have set compiler
  // flags to force arithmetic shifting!
  outnow >>= den0_bits;


  outbuf[saved_outptr] = outnow;
}



// Read accessor.

template <class samptype_t, class indextype_t>
void nloop_IIRBiquad<samptype_t, indextype_t>::GetCoefficients(
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
void nloop_IIRBiquad<samptype_t, indextype_t>::SetCoefficients(
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
// nloop_IIRBiquadChain Class


// Process buffers.
// This will still work with zero active stages (copying input to output).
// NOTE - This only reads the addressed elements. History is kept in internal
// buffers. As a result, this takes time to stabilize.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain<samptype_t, indextype_t, stagecount>::
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
int nloop_IIRBiquadChain<samptype_t, indextype_t, stagecount>::
  GetActiveStages(void)
{
  return stages_active;
}



// Set the number of active stages.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain<samptype_t, indextype_t, stagecount>::
  SetActiveStages(int new_stages)
{
  stages_active = new_stages;

  if (0 > new_stages)
    stages_active = 0;
  else if (stagecount < new_stages)
    stages_active = stagecount;
}




// Read biquad coefficients for one stage.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain<samptype_t, indextype_t, stagecount>::
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
void nloop_IIRBiquadChain<samptype_t, indextype_t, stagecount>::
  SetCoefficients( int stagenum,
  uint8_t new_den0bits, samptype_t new_den1, samptype_t new_den2,
  samptype_t new_num0, samptype_t new_num1, samptype_t new_num2 )
{
  if ((0 <= stagenum) && (stagecount > stagenum))
    biquads[stagenum].SetCoefficients( new_den0bits, new_den1, new_den2,
      new_num0, new_num1, new_num2 );
}



// Stuff one layer of the internal buffers.
// The idea is to initialize them to values that will settle quickly.

template <class samptype_t, class indextype_t, int stagecount>
void nloop_IIRBiquadChain<samptype_t, indextype_t, stagecount>::
  StuffBufferStage(int stagenum, samptype_t value)
{
  int sidx;

  // Valid stages are 0..stagecount.
  if ((0 <= stagenum) && (stagecount >= stagenum))
    for (sidx = 0; sidx < NLOOP_IIRBIQUADCHAIN_BUFSIZE; sidx++)
      buffers[stagenum][sidx] = value;
}



//
// This is the end of the file.
