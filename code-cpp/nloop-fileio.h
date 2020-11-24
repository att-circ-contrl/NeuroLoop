// Attention Circuits Control Laboratory - NeuroLoop project
// File I/O helper functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// NOTE - The implementation of these needs <regex> from C++11.


//
// Includes

// Standard library includes.
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <regex>


//
// Namespaces

// Things get very verbose very quickly without this.
using namespace std;



//
// Functions

// NOTE - File I/O functions will work with any stream-type objects, not just
// file streams.


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



// These read and write IIR biquad filter bank coefficients.
// When reading, columns unrelated to coefficients are ignored.
// When writing, extra columns with constant values may be added.
// See notes/BIQUADCOEFFS.txt for file format information.

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
  multimap<string,string> matchcriteria, map<int,int> bankremap);

// No extra output columns.
template<class samptype_t, class filtbanktype_t>
void nloop_WriteBiquadCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header);

// With extra output columns (written before the coefficients).
template<class samptype_t, class filtbanktype_t>
void nloop_WriteBiquadCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values);



//
// Code Inclusion

// C++ compiles templated classes and functions on-demand. The source code
// has to be included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies
// get pruned at link-time.

#include "nloop-fileio-inc.cpp"


//
// This is the end of the file.
