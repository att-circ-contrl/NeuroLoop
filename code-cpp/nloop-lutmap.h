// Attention Circuits Control Laboratory - NeuroLoop project
// Monotonic map lookup tables.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_LUTMAP_H
#define NLOOP_LUTMAP_H


//
// Classes


//
// Stepwise monotonic lookup table.

// This maps input to the first matching table row's output.
// This is done in a stepwise manner (no interpolation).
// Matching either searches for the first row entry <= the input in a
// descending monotonic table, or the first row entry >= the input in an
// ascending monotonic table.


// Individual version.

template <class intype_t, class outtype_t, int rowcount>
class nloop_LookupMonoStep_t
{
protected:
  intype_t input_lut[rowcount];
  outtype_t output_lut[rowcount];

  int rows_active;

public:
  // This forces consistent values (blanked table, zero size).
  nloop_LookupMonoStep_t(void);
  // Default destructor is fine.


  // Processing functions.

  // This searches a monotonic descending table for the first entry
  // less than or equal to the input argument.
  outtype_t Lookup_LE(intype_t inval);
  // This searches a monotonic ascending table for the first entry
  // greater than or equal to the input argument.
  outtype_t Lookup_GE(intype_t inval);


  // Accessors.

  void BlankTable(void);

  void SetEntry(int rowidx, intype_t inval, outtype_t outval);
  void GetEntry(int rowidx, intype_t &inval, outtype_t &outval);

  void SetActiveRows(int new_rows);
  int GetActiveRows(void);
};


// Parallel version - Per-bank lookup tables.
// NOTE - This accepts BAxCH input, but only has BAx1 lookup tables.

template <class intype_t, class outtype_t,
  int rowcount, int bankcount, int chancount>
class nloop_LookupMonoStepPerBank_t
{
protected:
  nloop_LookupMonoStep_t<intype_t,outtype_t,rowcount> lut[bankcount];

  int banks_active;
  int chans_active;
  int rows_active;

public:
  // This forces consistent values (blanked tables, zero size).
  nloop_LookupMonoStepPerBank_t(void);
  // Default destructor is fine.


  // Processing functions.

  // These perform lookups on a single element.
  outtype_t LookupOne_LE(intype_t inval, int bankidx);
  outtype_t LookupOne_GE(intype_t inval, int bankidx);

  // These perform lookups on all elements of a slice in parallel.
  void LookupAll_LE(
    nloop_SampleSlice_t<intype_t,bankcount,chancount> &invals,
    nloop_SampleSlice_t<outtype_t,bankcount,chancount> &outvals
  );
  void LookupAll_GE(
    nloop_SampleSlice_t<intype_t,bankcount,chancount> &invals,
    nloop_SampleSlice_t<outtype_t,bankcount,chancount> &outvals
  );


  // Accessors.

  void BlankTables(void);

  void SetAllLUTs(
    nloop_SampleSlice_t<intype_t,bankcount,rowcount> &invals,
    nloop_SampleSlice_t<outtype_t,bankcount,rowcount> &outvals );

  void SetOneLUT( int bankidx,
    nloop_SampleSlice_t<intype_t,1,rowcount> &invals,
    nloop_SampleSlice_t<outtype_t,1,rowcount> &outvals );

  void SetOneEntry(int bankidx, int rowidx,
    intype_t inval, outtype_t outval);


  void GetAllLUTs(
    nloop_SampleSlice_t<intype_t,bankcount,rowcount> &invals,
    nloop_SampleSlice_t<outtype_t,bankcount,rowcount> &outvals );

  void GetOneLUT( int bankidx,
    nloop_SampleSlice_t<intype_t,1,rowcount> &invals,
    nloop_SampleSlice_t<outtype_t,1,rowcount> &outvals );

  void GetOneEntry(int bankidx, int rowidx,
    intype_t &inval, outtype_t &outval);


  void SetActiveBanks(int new_banks);
  void SetActiveChans(int new_chans);
  void SetActiveRows(int new_rows);

  int GetActiveBanks(void);
  int GetActiveChans(void);
  int GetActiveRows(void);
};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-lutmap-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
