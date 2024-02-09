// Attention Circuits Control Laboratory - NeuroLoop project
// Voting logic.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// Functions


// Voting selection multiplexer among channels.
// For each channel, this picks data from the specified bank.

template <class samptype_t, int bankcount, int chancount>
void nloop_SelectWinningBanks(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &source,
  nloop_SampleSlice_t<samptype_t,1,chancount> &dest,
  nloop_SampleSlice_t<int,1,chancount> &selections
)
{
  int bidx, cidx;

  for (cidx = 0; cidx < chancount; cidx++)
  {
    // NOTE - Invalid selection IDs default to 0.
    bidx = selections.data[0][cidx];
    if ((bidx < 0) || (bidx >= bankcount))
      bidx = 0;

    dest.data[0][cidx] = source.data[bidx][cidx];
  }
}



// Keep-vs-replace latching.
// If "latchflags" matches "replaceflag", new values are latched.

template <class samptype_t, int bankcount, int chancount, bool replaceflag>
void nloop_ConditionallyLatchNew(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &target,
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &new_values,
  nloop_SampleSlice_t<bool,bankcount,chancount> &latchflags
)
{
  int bidx, cidx;
  bool thisflag;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
    {
      thisflag = latchflags.data[bidx][cidx];
      // NOTE - In C++, "==" on booleans is supposed to be XNOR, but do this
      // explicitly just in case of shenanigans.
      // The compiler will optimize this properly.
      if ( (thisflag && replaceflag) ||
        ((!thisflag) && (!replaceflag)) )
        target.data[bidx][cidx] = new_values.data[bidx][cidx];
    }
}



// Winner-take-all voting among banks.

// The "was_local_winner" flag is true if the winner was a local maximum,
// and false if the first or last bank won (edge of the distribution).

template <class samptype_t, int bankcount, int chancount>
void nloop_IdentifyWinningBanks(
  nloop_SampleSlice_t<samptype_t,bankcount,chancount> &source,
  int active_banks, int active_chans,
  nloop_SampleSlice_t<int,1,chancount> &selections,
  nloop_SampleSlice_t<bool,1,chancount> &was_local_winner
)
{
  int bidx, cidx;
  samptype_t thisval, maxval;
  int maxidx;
  bool was_local;

  if (active_banks > bankcount)
    active_banks = bankcount;
  if (active_chans > chancount)
    active_chans = chancount;

  selections.SetUniformValue(0);
  was_local_winner.SetUniformValue(false);

  for (cidx = 0; cidx < active_chans; cidx++)
  {
    maxval = source.data[0][cidx];
    maxidx = 0;

    for (bidx = 1; bidx < active_banks; bidx++)
    {
      thisval = source.data[bidx][cidx];
      if (thisval > maxval)
      {
        maxval = thisval;
        maxidx = bidx;
      }
    }

    was_local = true;
    if ( (0 == maxidx) || ((active_banks-1) == maxidx) )
      was_local = false;

    selections.data[0][cidx] = maxidx;
    was_local_winner.data[0][cidx] = was_local;
  }
}



//
// This is the end of the file.
