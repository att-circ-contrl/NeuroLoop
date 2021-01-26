// Attention Circuits Control Laboratory - NeuroLoop project
// Data slice implementations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// Data Buffer Class


// Copy-by-value from a different slice.

template <class samptype_t, int bankcount, int chancount>
void nloop_SampleSlice_t<samptype_t,bankcount,chancount>::
CopyFrom( nloop_SampleSlice_t<samptype_t,bankcount,chancount> &source )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      data[bidx][cidx] = source.data[bidx][cidx];
}



// Assign one value to all cells of the slice.

template <class samptype_t, int bankcount, int chancount>
void nloop_SampleSlice_t<samptype_t,bankcount,chancount>::
SetUniformValue( samptype_t newval )
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      data[bidx][cidx] = newval;
}



//
// Functions

template<class samptype_t, int bankcountsrc, int chancountsrc,
  int bankcountdst, int chancountdst>
void nloop_MapSlice(
  nloop_SampleSlice_t<int,bankcountdst,chancountdst> &src_banks,
  nloop_SampleSlice_t<int,bankcountdst,chancountdst> &src_chans,
  nloop_SampleSlice_t<samptype_t,bankcountsrc,chancountsrc> &source,
  nloop_SampleSlice_t<samptype_t,bankcountdst,chancountdst> &target
)
{
  int bidxsrc, cidxsrc, bidxdst, cidxdst;

  for (bidxdst = 0; bidxdst < bankcountdst; bidxdst++)
    for (cidxdst = 0; cidxdst < chancountdst; cidxdst++)
    {
      bidxsrc = src_banks.data[bidxdst][cidxdst];
      cidxsrc = src_chans.data[bidxdst][cidxdst];

      if (bidxsrc < 0)
        bidxsrc = 0;
      else if (bidxsrc >= bankcountsrc)
        bidxsrc = bankcountsrc - 1;

      if (cidxsrc < 0)
        cidxsrc = 0;
      else if (cidxsrc >= chancountsrc)
        cidxsrc = chancountsrc - 1;

      target.data[bidxdst][cidxdst] = source.data[bidxsrc][cidxsrc];
    }
}


//
// This is the end of the file.
