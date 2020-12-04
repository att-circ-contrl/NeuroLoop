// Attention Circuits Control Laboratory - NeuroLoop project
// File I/O helper functions - Templated functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.


//
// IIR Biquad Functions

// NOTE - File I/O functions will work with any stream-type objects, not just
// file streams.

// These read and write IIR biquad filter bank coefficients.
// When reading, columns unrelated to coefficients are ignored.
// When writing, extra columns with constant values may be added.
// See notes/BIQUADCOEFFS.txt for file format information.


// This sets all named bank and stage coefficients, active or not.
// This version treats all table rows as applying to this filter.
// Bank numbers are copied as-is.

template<class samptype_t, class filtbanktype_t>
void nloop_ReadBiquadCoeffs(istream &infile, filtbanktype_t &filtbank)
{
  multimap<string,string> criteria;
  map<int,int> bankremap;

  // Wrap the criteria-filtered bank-remapped version.

  criteria.clear();
  bankremap.clear();
  nloop_ReadBiquadCoeffs<samptype_t,filtbanktype_t>(infile,
    filtbank, criteria, bankremap);
}



// This sets all named bank and stage coefficients, active or not.
// This version does criteria checking and bank remapping.
// If match criteria are supplied, only table rows that match all of the
// specified criteria are used.
// Match criteria are (column name, cell value) tuples.
// Bank numbers in the remap table are remapped ( k -> bankremap[k] ).

template<class samptype_t, class filtbanktype_t>
void nloop_ReadBiquadCoeffs(istream &infile, filtbanktype_t &filtbank,
  multimap<string,string> matchcriteria, map<int,int> bankremap)
{
  map<string,vector<string>> tabdata;
  map<string,vector<string>>::iterator cidx;
  multimap<string,string>::iterator midx, midxend, tidx;
  size_t row_count, ridx;
  map<string,string> thisrow;
  string thiskey, thisval;
  bool match_ok, criterion_ok;
  vector<string> thisseries;
  int banknum, stagenum;
  uint8_t den0bits;
  samptype_t num0, num1, num2;
  samptype_t den0, den1, den2;


  // Read the raw CSV data.
  tabdata = nloop_ReadCSV(infile);

  // Get the number of rows.
  row_count = nloop_GetCSVRowCount(tabdata);


  // Iterate through rows.
  for (ridx = 0; ridx < row_count; ridx++)
  {
    // Fetch this row.
    thisrow = nloop_GetCSVRowCells(tabdata, ridx);


    // Check match criteria.

    match_ok = matchcriteria.empty();

    if (!match_ok)
    {
      match_ok = true;

      // To increment spans, look for the next entry with a different key.
      for ( midx = matchcriteria.begin();
        midx != matchcriteria.end();
        midx = matchcriteria.upper_bound(midx->first) )
      {
        thiskey = midx->first;

        // Find the end of this span.
        midxend = matchcriteria.upper_bound(thiskey);

        // Make sure we satisfy at least one of the allowed values for
        // this column name.
        criterion_ok = false;
        if (thisrow.count(thiskey))
        {
          thisval = thisrow[thiskey];
          for (tidx = midx; tidx != midxend; tidx++)
            if (tidx->second == thisval)
              criterion_ok = true;
        }

        match_ok = match_ok && criterion_ok;
      }
    }


    // Proceed if the row matches.
    if (match_ok)
    {
      // NOTE - Entries that aren't present will get created with "".
      // Running stoi on "" gives 0, which we can live with.

      banknum = stoi(thisrow["bank"]);
      stagenum = stoi(thisrow["stage"]);

      if (bankremap.count(banknum))
        banknum = bankremap[banknum];

      // FIXME - Assuming signed 64-bit is larger than samptype_t.
      // If the user is using uint64_t, this will lose half the range.

      num0 = nloop_LLToSample<samptype_t>( stoll(thisrow["num0"]) );
      num1 = nloop_LLToSample<samptype_t>( stoll(thisrow["num1"]) );
      num2 = nloop_LLToSample<samptype_t>( stoll(thisrow["num2"]) );

      den0 = nloop_LLToSample<samptype_t>( stoll(thisrow["den0"]) );
      den1 = nloop_LLToSample<samptype_t>( stoll(thisrow["den1"]) );
      den2 = nloop_LLToSample<samptype_t>( stoll(thisrow["den2"]) );

      // Get the bit-shift value.
      // This tolerates negative den0.
      den0bits = 0; // den0 = 1.
      while (den0 > 1)
      {
        den0 >>= 1;
        den0bits++;
      }

      // Set this coefficient.
      filtbank.SetCoefficients( stagenum, banknum,
        den0bits, den1, den2, num0, num1, num2 );
    }
  }
}



// No extra columns.
// This only reads out active banks and stages.

template<class samptype_t, class filtbanktype_t>
void nloop_WriteBiquadCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header)
{
  list<string> col_order;
  map<string,string> col_values;

  // Wrap the extra-columns version.

  col_order.clear();
  col_values.clear();
  nloop_WriteBiquadCoeffs<samptype_t,filtbanktype_t>(
    outfile, filtbank, want_header, col_order, col_values );
}



// With extra columns (written before the coefficients).
// This only reads out active banks and stages.

template<class samptype_t, class filtbanktype_t>
void nloop_WriteBiquadCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values)
{
  int chancount, bankcount, stagecount;
  int bidx, sidx;
  list<string> colnames;
  map<string,vector<string>> colseries;
  list<string>::iterator nidx;
  uint8_t den0bits;
  samptype_t num0, num1, num2, den0, den1, den2;


  // Get active geometry.

  chancount = filtbank.GetActiveChans();
  bankcount = filtbank.GetActiveBanks();
  stagecount = filtbank.GetActiveStages();

  // Construct column label list.

  colnames.clear();

  for (nidx = extra_col_order.begin(); nidx != extra_col_order.end(); nidx++)
    colnames.push_back(*nidx);

  colnames.push_back("bank");
  colnames.push_back("stage");
  colnames.push_back("num0");
  colnames.push_back("num1");
  colnames.push_back("num2");
  colnames.push_back("den0");
  colnames.push_back("den1");
  colnames.push_back("den2");


  // Construct data series.

  // Initialize to suppress compiler warnings.
  // FIXME - Maybe set to blatantly bogus values for easier debugging?
  den0bits = 1;
  den1 = 0;
  den2 = 0;
  num0 = 0;
  num1 = 0;
  num2 = 0;

  colseries.clear();

  if (0 < chancount)
    for (bidx = 0; bidx < bankcount; bidx++)
      for (sidx = 0; sidx < stagecount; sidx++)
      {
        // Coefficients.

        filtbank.GetCoefficients(sidx, bidx,
          den0bits, den1, den2, num0, num1, num2);
        den0 = 1;
        den0 <<= den0bits;

        colseries["bank"].push_back(to_string(bidx));
        colseries["stage"].push_back(to_string(sidx));

        colseries["num0"].push_back(to_string(
          nloop_SampleToLL<samptype_t>(num0) ));
        colseries["num1"].push_back(to_string(
          nloop_SampleToLL<samptype_t>(num1) ));
        colseries["num2"].push_back(to_string(
          nloop_SampleToLL<samptype_t>(num2) ));

        colseries["den0"].push_back(to_string(
          nloop_SampleToLL<samptype_t>(den0) ));
        colseries["den1"].push_back(to_string(
          nloop_SampleToLL<samptype_t>(den1) ));
        colseries["den2"].push_back(to_string(
          nloop_SampleToLL<samptype_t>(den2) ));

        // Extra columns.

        for (nidx = extra_col_order.begin();
          nidx != extra_col_order.end();
          nidx++)
          colseries[*nidx].push_back(extra_col_values[*nidx]);
      }


  // Write the table.
  nloop_WriteCSV(outfile, colnames, colseries, want_header);
}



// This converts samptype_t to signed long long, interpreting samptype_t
// as signed.

template<class samptype_t> long long nloop_SampleToLL(samptype_t data)
{
  long long result;
  samptype_t maxval;

  // FIXME - Assume the data value fits.
  // If the user is using uint64_t, we'll lose half the range.
  // NOTE - Don't assume that long long is int64_t.

  result = (long long) data;

  // If we have unsigned samples, figure out if this is a negative value.
  if (!( NLOOP_ISSIGNED(samptype_t) ))
  {
    maxval = NLOOP_MAXVAL(samptype_t);
    if (data > (maxval >> 1))
    {
      // We're operating modulo (maxval+1).
      result -= maxval;
      result--;
    }
  }

  return result;
}



// This converts signed long long to samptype_t appropriately.

template<class samptype_t> samptype_t nloop_LLToSample(long long data)
{
  samptype_t result;
  samptype_t maxval;

  if ( NLOOP_ISSIGNED(samptype_t) )
    result = (samptype_t) data;
  else
  {
    maxval = NLOOP_MAXVAL(samptype_t);
    if (data < 0)
    {
      // We're operating modulo (maxval+1).
      data += maxval;
      data++;
    }

    result = (samptype_t) data;
  }

  return result;
}



//
// This is the end of the file.
