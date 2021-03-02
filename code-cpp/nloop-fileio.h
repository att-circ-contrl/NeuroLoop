// Attention Circuits Control Laboratory - NeuroLoop project
// File I/O helper functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// NOTE - The implementation of these needs <regex> from C++11.

//
// Wrapper.
#ifndef NLOOP_FILEIO_H
#define NLOOP_FILEIO_H



// NOTE - File I/O functions will work with any stream-type objects, not just
// file streams.


//
// Functions


// I/O helper functions.

// This converts samptype_t to signed long long, interpreting samptype_t
// as signed.
template<class samptype_t> long long nloop_SampleToLL(samptype_t data);

// This converts signed long long to samptype_t appropriately.
template<class samptype_t> samptype_t nloop_LLToSample(long long data);

// These check a row's contents against match criteria.
// Match criteria are (column name, cell value) tuples.
// An empty criteria list always matches.
bool nloop_CSVRowMatchesAnyCriteria( map<string,string> &thisrow,
  multimap<string,string> &matchcriteria );
bool nloop_CSVRowMatchesAllCriteria( map<string,string> &thisrow,
  multimap<string,string> &matchcriteria );


// CSV I/O functions.

// This reads all columns from a CSV file, discarding order information.
// A header line must exist, containing column names.
// Cell values within quotes have the outermost quotes stripped.
map<string,vector<string>> nloop_ReadCSV(istream &infile);

// This writes data series to a CSV file using the specified column order.
void nloop_WriteCSV(ostream &outfile, list<string> colnames,
  map<string,vector<string>> &dataseries, bool want_header);

// This returns the number of rows in a CSV table.
size_t nloop_GetCSVRowCount(map<string,vector<string>> &datatable);

// This returns a specific row from a CSV table.
// Nonexistent cells contain the empty string.
map<string,string> nloop_GetCSVRowCells(
  map<string,vector<string>> &datatable, size_t ridx);


// Biquad I/O functions.

// These read and write IIR biquad filter bank coefficients.
// When reading, columns unrelated to coefficients are ignored.
// When writing, extra columns with constant values may be added.
// See notes/BIQUADCOEFFS.txt for file format information.

// CSV biquad coefficients may be negative even with unsigned samptype_t.

// Reading.

// Treat all table rows as applying to this filter.
// Don't remap bank numbers.
template<class samptype_t, class filtbanktype_t>
void nloop_ReadBiquadCoeffs(istream &infile, filtbanktype_t &filtbank);

// If match criteria are supplied, only table rows that match all of the
// specified criteria are used.
// Match criteria are (column name, cell value) tuples.
// Bank numbers in the remap table are remapped ( k -> bankremap[k] ).
template<class samptype_t, class filtbanktype_t>
void nloop_ReadBiquadCoeffs(istream &infile, filtbanktype_t &filtbank,
  multimap<string,string> &matchcriteria, map<int,int> &bankremap);

// Writing.
// NOTE - These only write active banks and stages.

// No extra output columns.
template<class samptype_t, class filtbanktype_t>
void nloop_WriteBiquadCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header);

// With extra output columns (written before the coefficients).
template<class samptype_t, class filtbanktype_t>
void nloop_WriteBiquadCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values);


// Lookup table I/O functions.

// These read and write lookup table tuples.

// When reading, columns unrelated to tuple address and data are ignored.
// NOTE - Only the LUT entries listed in the CSV file are modified. Other
// entries in the LUT are left as-is.

// When writing, extra columns with constant values may be added.
// See notes/LUTVALUES.txt for file format information.

// Reading.

// Reading a single lookup table.
// Treat all CSV rows as being part of this lookup table.
template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTableSingle( istream &infile, luttype_t &lut,
  string infield, string outfield );

// Reading a single lookup table.
// If match criteria are supplied, only table rows that match all of the
// specified criteria are used.
// Match criteria are (column name, cell value) tuples.
template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTableSingle( istream &infile, luttype_t &lut,
  string infield, string outfield, multimap<string,string> &matchcriteria );

// Reading a class with per-bank lookup tables.
// Treat all CSV rows as being part of this lookup table.
// Don't remap bank numbers.
template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTablePerBank( istream &infile, luttype_t &lut,
  string infield, string outfield );

// Reading a class with per-bank lookup tables.
// If match criteria are supplied, only table rows that match all of the
// specified criteria are used.
// Match criteria are (column name, cell value) tuples.
// Bank numbers in the remap table are remapped ( k -> bankremap[k] ).
template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTablePerBank( istream &infile, luttype_t &lut,
  string infield, string outfield,
  multimap<string,string> &matchcriteria, map<int,int> &bankremap );

// Writing.
// NOTE - These only write active banks and rows.

// Writing a single lookup table.
// No extra output columns.
template <class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTableSingle( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header );

// Writing a single lookup table.
// With extra output columns (written before the tuple contents).
template <class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTableSingle( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values );

// Writing a class with per-bank lookup tables.
// No extra output columns.
template <class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTablePerBank( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header );

// Writing a class with per-bank lookup tables.
// With extra output columns (written before the tuple contents).
template <class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTablePerBank( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values );


//
// Code Inclusion

// C++ compiles templated classes and functions on-demand. The source code
// has to be included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies
// get pruned at link-time.

#include "nloop-fileio-inc.cpp"


// End of wrapper.
#endif

//
// This is the end of the file.
