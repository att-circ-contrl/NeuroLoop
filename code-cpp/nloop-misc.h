// Attention Circuits Control Laboratory - NeuroLoop project
// Miscellaneous declarations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Data Buffer Classes


// FIXME - There's no such thing as a templated typedef, so these have to
// be classes even for relatively simple constructs.

// This represents one "slice" of sample data across all channels and
// filter banks within a signal processing pipeline.

template <class samptype_t, int bankcount, int chancount>
class nloop_SampleSlice_t
{
public:
  samptype_t data[bankcount][chancount];
};



//
// This is the end of the file.
