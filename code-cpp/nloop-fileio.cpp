// Attention Circuits Control Laboratory - NeuroLoop project
// File I/O helper functions - non-template functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

//
// Includes

#include "nloop-includes-workstation.h"



// NOTE - File I/O functions will work with any stream-type objects, not just
// file streams.


//
// I/O Helper Functions


// These check a row's contents against match criteria.
// Match criteria are (column name, cell value) tuples.

// Private implementation.

void nloop_CSVRowMatchesCriteria_helper( map<string,string> &thisrow,
  multimap<string,string> &matchcriteria,
  bool &matches_any, bool &matches_all )
{
  multimap<string,string>::iterator midx, tidx, tidxend;
  string thiskey, thisval;
  bool criterion_ok;

  matches_any = false;
  matches_all = true;

  // Special-case the empty list.
  if (matchcriteria.empty())
  {
    matches_any = true;
    matches_all = true;
  }

  // We're stepping through spans that share the same key.
  // To increment, move to the nearest entry with a different key.
  for ( midx = matchcriteria.begin();
    midx != matchcriteria.end();
    midx = matchcriteria.upper_bound(midx->first) )
  {
    thiskey = midx->first;

    // Find the end of this span.
    tidxend = matchcriteria.upper_bound(thiskey);

    // See if we satisfy at least one of the allowed values for this
    // key value (column name).
    criterion_ok = false;
    if (thisrow.count(thiskey))
    {
      thisval = thisrow[thiskey];
      for (tidx = midx; tidx != tidxend; tidx++)
        if (tidx->second == thisval)
          criterion_ok = true;
    }

    // Update match flags.
    matches_any = matches_any || criterion_ok;
    matches_all = matches_all && criterion_ok;
  }
}


// Wrappers for "any"/"all" public versions.

bool nloop_CSVRowMatchesAnyCriteria( map<string,string> &thisrow,
  multimap<string,string> &matchcriteria )
{
  bool matches_any, matches_all;

  matches_any = false;
  matches_all = false;

  nloop_CSVRowMatchesCriteria_helper( thisrow, matchcriteria,
    matches_any, matches_all );

  return matches_any;
}


bool nloop_CSVRowMatchesAllCriteria( map<string,string> &thisrow,
  multimap<string,string> &matchcriteria )
{
  bool matches_any, matches_all;

  matches_any = false;
  matches_all = false;

  nloop_CSVRowMatchesCriteria_helper( thisrow, matchcriteria,
    matches_any, matches_all );

  return matches_all;
}



//
// CSV I/O Functions


// This reads all columns from a CSV file, discarding order information.
// A header line must exist, containing column names.
// Cell values within quotes have the outermost quotes stripped.

map<string,vector<string>> nloop_ReadCSV(istream &infile)
{
  map<string,vector<string>> result;
  list<string> colnames, cellvals;
  list<string>::iterator cidx, vidx;
  string thisline;
  bool first_line;
  regex regexnotnewline, regexnotblank;
  regex regexquoted, regexnoquotes;
  regex regexquotedend, regexnoquotesend;
  smatch thismatchlist;
  bool had_match, have_string;
  string matchval;

  // We need this to trim line delimiters, which don't match "." but do "\S".
  regexnotnewline = "(.*)";
  regexnotblank = "\\S";
  // FIXME - These regexes aren't bulletproof.
  regexquoted = "\\s*\"(.*?)\"\\s*,(.*)";
  regexnoquotes = "\\s*([^\",\\s]*)\\s*,(.*)";
  regexquotedend = "\\s*\"(.*?)\"\\s*";
  // This matches the empty string.
  regexnoquotesend = "\\s*([^\",\\s]*)\\s*";


  first_line = true;
  colnames.clear();

  result.clear();

  while (infile.good())
  {
    getline(infile, thisline);

    if (infile.good())
    {
      // Process this line.

      // Trim delimiters. We can get "\r" at the end reading DOS-format text
      // under Linux.
      if (regex_search(thisline, thismatchlist, regexnotnewline))
        thisline = thismatchlist[1];

      // Proceed if not blank.
      if (regex_search(thisline, regexnotblank))
      {
        // Get this row's cell contents.

        cellvals.clear();

        had_match = true;
        have_string = true;

        while (had_match && have_string)
        {
          matchval = "";

          // Use "match", not "search".
          // We want the full string to have the specified format.
          if (regex_match(thisline, thismatchlist, regexquoted))
          {
            matchval = thismatchlist[1];
            thisline = thismatchlist[2];
          }
          else if (regex_match(thisline, thismatchlist, regexnoquotes))
          {
            matchval = thismatchlist[1];
            thisline = thismatchlist[2];
          }
          else if (regex_match(thisline, thismatchlist, regexquotedend))
          {
            matchval = thismatchlist[1];
            thisline = "";
            have_string = false;
          }
          else if (regex_match(thisline, thismatchlist, regexnoquotesend))
          {
            matchval = thismatchlist[1];
            thisline = "";
            have_string = false;
          }
          else
            // This should only happen on an error condition, as the empty
            // string will match "no quotes end".
            had_match = false;

          // If we found a cell, add it.
          if (had_match)
            cellvals.push_back(matchval);
        }

        // If this was the first line, use the contents as the column names.
        // Otherwise treat it as data.

        if (first_line)
        {
          // Header row. Store the column names.
          // Iterate to make sure we're copying by value and not reference.

          colnames.clear();
          for (cidx = cellvals.begin(); cidx != cellvals.end(); cidx++)
            colnames.push_back(*cidx);

          first_line = false;
        }
        else
        {
          // Data row. Store the column cell values.
          // This will create the column vectors the first time we access
          // them.

          // Add the cells that we have.
          // This tolerates rows that are too long or too short.
          vidx = cellvals.begin();
          for (cidx = colnames.begin(); cidx != colnames.end(); cidx++)
          {
            if (vidx != cellvals.end())
            {
              // Store this cell.
              result[*cidx].push_back(*vidx);
              vidx++;
            }
            else
            {
              // Missing cell; store the empty string.
              result[*cidx].push_back("");
            }
          }
        }


        // Finished with this row.
      }
    }
  }

// FIXME - Diagnostics.
#if 0
{
map<string,vector<string>>::iterator tidx;
vector<string> thiscolumn;
vector<string>::iterator ridx;
cout << "== Table columns:\n";
for (tidx = result.begin(); tidx != result.end(); tidx++)
{
  cout << "-- Column \"" << (tidx->first) << "\":\n";
  thiscolumn = tidx->second;
  for (ridx = thiscolumn.begin(); ridx != thiscolumn.end(); ridx++)
    cout << ".. \"" << (*ridx) << "\"\n";
}
cout << "== End of table columns.\n";
}
#endif

  return result;
}



// This writes data series to a CSV file using the specified column order.

void nloop_WriteCSV(ostream &outfile, list<string> colnames,
  map<string,vector<string>> &dataseries, bool want_header)
{
  list<string>::iterator cidx;
  map<string,string> thisrow;
  bool is_first;
  string thiskey;
  size_t row_count, ridx;

  // Write the column labels.
  if (want_header)
  {
    is_first = true;
    for (cidx = colnames.begin(); colnames.end() != cidx; cidx++)
    {
      if (!is_first)
        outfile << ',';
      is_first = false;
      outfile << '"' << (*cidx) << '"';
    }
    outfile << "\r\n";
  }

  // Get the number of rows (maximum across all columns).
  row_count = nloop_GetCSVRowCount(dataseries);

  // Write data series.
  // Nonexistent cells contain "".
  for (ridx = 0; ridx < row_count; ridx++)
  {
    thisrow = nloop_GetCSVRowCells(dataseries, ridx);

    is_first = true;
    for (cidx = colnames.begin(); colnames.end() != cidx; cidx++)
    {
      if (!is_first)
        outfile << ',';
      is_first = false;
      thiskey = *cidx;
      if (thisrow.count(thiskey))
        outfile << thisrow[thiskey];
    }
    outfile << "\r\n";
  }
}



// This returns the number of rows in a CSV table.

size_t nloop_GetCSVRowCount(map<string,vector<string>> &datatable)
{
  map<string,vector<string>>::iterator cidx;
  size_t row_count;
  string thiskey;

  // Get the number of rows.
  // This doesn't have to be the same for all columns.
  row_count = 0;
  for (cidx = datatable.begin(); datatable.end() != cidx; cidx++)
  {
    thiskey = cidx->first;
    if ( row_count < datatable[thiskey].size() )
      row_count = datatable[thiskey].size();
  }

  return row_count;
}



// This returns a specific row from a CSV table.
// Nonexistent cells contain the empty string.

map<string,string> nloop_GetCSVRowCells(
  map<string,vector<string>> &datatable, size_t ridx)
{
  map<string,vector<string>>::iterator cidx;
  map<string,string> thisrow;
  string thiskey;

  thisrow.clear();

  for (cidx = datatable.begin(); datatable.end() != cidx; cidx++)
  {
    thiskey = cidx->first;

    thisrow[thiskey] = "";
    if ( ridx < datatable[thiskey].size() )
      thisrow[thiskey] = datatable[thiskey][ridx];
  }

  return thisrow;
}



//
// This is the end of the file.
