// Attention Circuits Control Laboratory - NeuroLoop project
// Voting logic.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_VOTING_H
#define NLOOP_VOTING_H


//
// Functions

// Voting selection multiplexer among channels.
// For each channel, this picks data from the specified bank.

template <class samptype_t, int bankcount, int chancount>
void nloop_SelectWinningBanks(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &source,
  nloop_SampleSlice_t<samptype_t,1,chancount> &dest,
  nloop_SampleSlice_t<int,1,chancount> &selections
);


// Keep-vs-replace latching.
// If "latchflags" matches "replaceflag", new values are latched.

template <class samptype_t, int bankcount, int chancount, bool replaceflag>
void nloop_ConditionallyLatchNew(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &target,
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &new_values,
  nloop_SampleSlice_t<bool,bankcount,chancount> &latchflags
);


// Winner-take-all voting among banks.

// The "was_local_winner" flag is true if the winner was a local maximum,
// and false if the first or last bank won (edge of the distribution).

template <class samptype_t, int bankcount, int chancount>
void nloop_IdentifyWinningBanks(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &source,
  int active_banks, int active_chans,
  nloop_SampleSlice_t<int,1,chancount> &selections,
  nloop_SampleSlice_t<bool,1,chancount> &was_local_winner
);


//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.

#include "nloop-voting-inc.cpp"


// End of wrapper.
#endif


//
// This is the end of the file.
