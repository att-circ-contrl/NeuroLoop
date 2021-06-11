// Attention Circuits Control Laboratory - NeuroLoop project
// Miscellaneous math routines.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// Functions


// Single-sample function for fast modulo arithmetic of non-negative integers.
// This tests for quotients up to (2^count)-1.
// Division is expensive, especially in FPGAs, so for situations where we
// know that the quotient is small, shift-and-subtract can be less
// resource-intensive.

// NOTE - We're counting on the compiler being smart enough to inline this.
template <class datatype_t, int subcount>
datatype_t nloop_FastModulo(datatype_t sample, datatype_t modulus)
{
  int bitshift;
  datatype_t testval;

  // This will actually be pretty slow in software. We're using it to get a
  // 1:1 mapping between software and FPGA code.
  for (bitshift = subcount; bitshift > 0; bitshift--)
  {
    testval = modulus << (bitshift - 1);
    if (sample >= testval)
      sample -= testval;
  }

  return sample;
}



// Slice-based function for fast modulo arithmetic of non-negative integers.
// This tests for quotients up to 2^(count-1).
// Division is expensive, especially in FPGAs, so for situations where we
// know that the quotient is small, shift-and-subtract can be less
// resource-intensive.
// Input and output may reference the same object.

template <class datatype_t, int subcount, int bankcount, int chancount>
void nloop_FastModulo_Bank(
  nloop_SampleSlice_t<datatype_t,bankcount,chancount> &indata,
  nloop_SampleSlice_t<datatype_t,bankcount,chancount> &moduli,
  nloop_SampleSlice_t<datatype_t,bankcount,chancount> &outdata
)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      // NOTE - We're counting on the compiler inlining this, for speed.
      outdata.data[bidx][cidx] = nloop_FastModulo<datatype_t,subcount>
        (indata.data[bidx][cidx], moduli.data[bidx][cidx]);
}


//
// This is the end of the file.
