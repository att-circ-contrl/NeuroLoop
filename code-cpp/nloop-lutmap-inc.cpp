// Attention Circuits Control Laboratory - NeuroLoop project
// Monotonic map lookup tables.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// Classes


//
// Stepwise monotonic lookup table.

// This maps input to the first matching table row's output.
// This is done in a stepwise manner (no interpolation).
// Matching either searches for the first row entry <= the input in a
// descending monotonic table, or the first row entry >= the input in an
// ascending monotonic table.


//
// Individual version.


// Constructor.
// This forces consistent values (blanked table, zero size).

template <class intype_t, class outtype_t, int rowcount>
nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
nloop_LookupMonoStep_t(void)
{
  BlankTable();
  rows_active = 0;
}


// Default destructor is fine.



// Processing functions.

// This searches a monotonic descending table for the first entry
// less than or equal to the input argument.

template <class intype_t, class outtype_t, int rowcount>
outtype_t nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
Lookup_LE(intype_t inval)
{
  outtype_t outval;
  int ridx, ridxmax;

  // FIXME - Linear search. This should be a binary search.
  // Using linear as a placeholder for ease of debugging.

  // FIXME - Take pessimal time (checking all entries), so that embedded
  // applications are easier to debug.
  // Otherwise time taken would vary with data.

  ridxmax = rows_active;
  if (ridxmax > rowcount)
    ridxmax = rowcount;

  // Force sane output.
  outval = 0;

  for (ridx = (ridxmax - 1); ridx >= 0; ridx--)
    if (input_lut[ridx] <= inval)
      outval = output_lut[ridx];

  return outval;
}



// This searches a monotonic ascending table for the first entry
// greater than or equal to the input argument.

template <class intype_t, class outtype_t, int rowcount>
outtype_t nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
Lookup_GE(intype_t inval)
{
  outtype_t outval;
  int ridx, ridxmax;

  // FIXME - Linear search. This should be a binary search.
  // Using linear as a placeholder for ease of debugging.

  // FIXME - Take pessimal time (checking all entries), so that embedded
  // applications are easier to debug.
  // Otherwise time taken would vary with data.

  ridxmax = rows_active;
  if (ridxmax > rowcount)
    ridxmax = rowcount;

  // Force sane output.
  outval = 0;

  for (ridx = (ridxmax - 1); ridx >= 0; ridx--)
    if (input_lut[ridx] >= inval)
      outval = output_lut[ridx];

  return outval;
}



// Accessors.

template <class intype_t, class outtype_t, int rowcount>
void nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
BlankTable(void)
{
  int ridx;

  for (ridx = 0; ridx < rowcount; ridx++)
  {
    // Zero can always be cast to an appropriate type.
    input_lut[ridx] = 0;
    output_lut[ridx] = 0;
  }
}



template <class intype_t, class outtype_t, int rowcount>
void nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
SetEntry(int rowidx, intype_t inval, outtype_t outval)
{
  if ( (rowidx >= 0) && (rowidx < rowcount) )
  {
    input_lut[rowidx] = inval;
    output_lut[rowidx] = outval;
  }
}



template <class intype_t, class outtype_t, int rowcount>
void nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
GetEntry(int rowidx, intype_t &inval, outtype_t &outval)
{
  inval = 0;
  outval = 0;

  if ( (rowidx >= 0) && (rowidx < rowcount) )
  {
    inval = input_lut[rowidx];
    outval = output_lut[rowidx];
  }
}



template <class intype_t, class outtype_t, int rowcount>
void nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
SetActiveRows(int new_rows)
{
  rows_active = new_rows;

  if (rows_active < 0)
    rows_active = 0;
  else if (rows_active > rowcount)
    rows_active = rowcount;
}



template <class intype_t, class outtype_t, int rowcount>
int nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount>::
GetActiveRows(void)
{
  return rows_active;
}



//
// Parallel version - Per-bank lookup tables.
// NOTE - This accepts BAxCH input, but only has BAx1 lookup tables.


// Constructor.
// This forces consistent values (blanked table, zero size).

template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
nloop_LookupMonoStepPerBank_t(void)
{
  BlankTables();

  banks_active = 0;
  chans_active = 0;

  // Call the helper for this, since we have to propagate it to the lookup
  // tables.
  SetActiveRows(0);
}


// Default destructor is fine.



// Processing functions.


// Single-element lookup, LE.

template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
outtype_t nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
LookupOne_LE(intype_t inval, int bankidx)
{
  outtype_t outval;

  outval = 0;

  if ( (0 <= bankidx) && (bankidx < bankcount) )
    outval = lut[bankidx].Lookup_LE(inval);

  return outval;
}



// Single-element lookup, GE.

template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
outtype_t nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
LookupOne_GE(intype_t inval, int bankidx)
{
  outtype_t outval;

  outval = 0;

  if ( (0 <= bankidx) && (bankidx < bankcount) )
    outval = lut[bankidx].Lookup_GE(inval);

  return outval;
}



// Full slice lookup, LE.

template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
LookupAll_LE(
  nloop_SampleSlice_t<intype_t,bankcount,chancount> &invals,
  nloop_SampleSlice_t<outtype_t,bankcount,chancount> &outvals
)
{
  int blimit, climit;
  int bidx, cidx;

  blimit = banks_active;
  if (blimit > bankcount)
    blimit = bankcount;

  climit = chans_active;
  if (climit > chancount)
    climit = chancount;

  // Squash all elements.
  outvals.SetUniformValue(0);

  // Only look up active elements.
  for (bidx = 0; bidx < blimit; bidx++)
    for (cidx = 0; cidx < climit; cidx++)
      outvals.data[bidx][cidx] =
        lut[bidx].Lookup_LE( invals.data[bidx][cidx] );
}



// Full slice lookup, GE.

template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
LookupAll_GE(
  nloop_SampleSlice_t<intype_t,bankcount,chancount> &invals,
  nloop_SampleSlice_t<outtype_t,bankcount,chancount> &outvals
)
{
  int blimit, climit;
  int bidx, cidx;

  blimit = banks_active;
  if (blimit > bankcount)
    blimit = bankcount;

  climit = chans_active;
  if (climit > chancount)
    climit = chancount;

  // Squash all elements.
  outvals.SetUniformValue(0);

  // Only look up active elements.
  for (bidx = 0; bidx < blimit; bidx++)
    for (cidx = 0; cidx < climit; cidx++)
      outvals.data[bidx][cidx] =
        lut[bidx].Lookup_GE( invals.data[bidx][cidx] );
}



// Accessors.


template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
BlankTables(void)
{
  int bidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    lut[bidx].BlankTable();
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
SetAllLUTs(
  nloop_SampleSlice_t<intype_t,bankcount,rowcount> &invals,
  nloop_SampleSlice_t<outtype_t,bankcount,rowcount> &outvals
)
{
  int bidx, ridx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (ridx = 0; ridx < rowcount; ridx++)
      lut[bidx].SetEntry( ridx,
        invals.data[bidx][ridx], outvals.data[bidx][ridx] );
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
SetOneLUT( int bankidx,
  nloop_SampleSlice_t<intype_t,1,rowcount> &invals,
  nloop_SampleSlice_t<outtype_t,1,rowcount> &outvals
)
{
  int ridx;

  if ( (bankidx >= 0) && (bankidx < bankcount) )
    for (ridx = 0; ridx < rowcount; ridx++)
      lut[bankidx].SetEntry( ridx,
        invals.data[0][ridx], outvals.data[0][ridx] );
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
SetOneEntry( int bankidx, int rowidx, intype_t inval, outtype_t outval )
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (rowidx >= 0) && (rowidx < rowcount) )
      lut[bankidx].SetEntry( rowidx, inval, outval );
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
GetAllLUTs(
  nloop_SampleSlice_t<intype_t,bankcount,rowcount> &invals,
  nloop_SampleSlice_t<outtype_t,bankcount,rowcount> &outvals
)
{
  int bidx, ridx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (ridx = 0; ridx < rowcount; ridx++)
      lut[bidx].GetEntry( ridx,
        invals.data[bidx][ridx], outvals.data[bidx][ridx] );
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
GetOneLUT( int bankidx,
  nloop_SampleSlice_t<intype_t,1,rowcount> &invals,
  nloop_SampleSlice_t<outtype_t,1,rowcount> &outvals
)
{
  int ridx;

  if ( (bankidx >= 0) && (bankidx < bankcount) )
    for (ridx = 0; ridx < rowcount; ridx++)
      lut[bankidx].GetEntry( ridx,
        invals.data[0][ridx], outvals.data[0][ridx] );
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
GetOneEntry( int bankidx, int rowidx, intype_t &inval, outtype_t &outval )
{
  if ( (bankidx >= 0) && (bankidx < bankcount)
    && (rowidx >= 0) && (rowidx < rowcount) )
      lut[bankidx].GetEntry( rowidx, inval, outval );
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
SetActiveBanks(int new_banks)
{
  banks_active = new_banks;

  if (banks_active < 0)
    banks_active = 0;

  if (banks_active > bankcount)
    banks_active = bankcount;
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
SetActiveChans(int new_chans)
{
  chans_active = new_chans;

  if (chans_active < 0)
    chans_active = 0;

  if (chans_active > chancount)
    chans_active = chancount;
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
void nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
SetActiveRows(int new_rows)
{
  int bidx;

  rows_active = new_rows;

  if (rows_active < 0)
    rows_active = 0;

  if (rows_active > rowcount)
    rows_active = rowcount;

  // Propagate to the individual lookup tables.
  for (bidx = 0; bidx < bankcount; bidx++)
    lut[bidx].SetActiveRows(rows_active);
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
int nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
GetActiveBanks(void)
{
  return banks_active;
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
int nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
GetActiveChans(void)
{
  return chans_active;
}



template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
int nloop_LookupMonoStepPerBank_t<intype_t,outtype_t,
  rowcount,bankcount,chancount>::
GetActiveRows(void)
{
  // Return the cached value, rather than querying the lookup tables.
  return rows_active;
}



//
// This is the end of the file.
