// Attention Circuits Control Laboratory - NeuroLoop project
// File I/O helper functions - Templated functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// NOTE - Because this implements template code, it has to be included by
// every file that instantiates templates.
// Only one copy of each instantiated variant will actually be compiled;
// extra copies get pruned at link-time.



// NOTE - File I/O functions will work with any stream-type objects, not just
// file streams.


//
// I/O Helper Functions


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
// IIR Biquad Functions

// These read and write IIR biquad filter bank coefficients.
// When reading, columns unrelated to coefficients are ignored.
// When writing, extra columns with constant values may be added.
// See BIQUADCOEFFS.txt for file format information.


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
  multimap<string,string> &matchcriteria, map<int,int> &bankremap)
{
  map<string,vector<string>> tabdata;
  size_t row_count, ridx;
  map<string,string> thisrow;
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
    // Proceed if the row matches.
    if ( nloop_CSVRowMatchesAllCriteria(thisrow, matchcriteria) )
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
// This only writes active banks and stages.

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
// This only writes active banks and stages.

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



//
// FIR I/O functions.

// These read and write FIR filter bank coefficients.
// When reading, columns unrelated to coefficients are ignored.
// When writing, extra columns with constant values may be added.
// See FIRCOEFFS.txt for file format information.

// CSV FIR coefficients may be negative even with unsigned samptype_t.


// This reads a FIR filter from a stream.
// Treat all table rows as applying to this filter.
// Don't remap bank numbers.

template<class samptype_t, class indextype_t, class filtbanktype_t>
void nloop_ReadFIRCoeffs(istream &infile, filtbanktype_t &filtbank,
  uint8_t fracbits)
{
  multimap<string,string> criteria;
  map<int,int> bankremap;

  // Wrap the criteria-filtered bank-remapped version.
  criteria.clear();
  bankremap.clear();
  nloop_ReadFIRCoeffs<samptype_t,indextype_t,filtbanktype_t>(
    infile, filtbank, fracbits, criteria, bankremap);
}



// This reads a FIR filter from a stream.
// If match criteria are supplied, only table rows that match all of the
// specified criteria are used.
// Match criteria are (column name, cell value) tuples.
// Bank numbers in the remap table are remapped ( k -> bankremap[k] ).

template<class samptype_t, class indextype_t, class filtbanktype_t>
void nloop_ReadFIRCoeffs(istream &infile, filtbanktype_t &filtbank,
  uint8_t fracbits,
  multimap<string,string> &matchcriteria, map<int,int> &bankremap)
{
  map<string,vector<string>> tabdata;
  map<string,vector<string>>::iterator tabcolidx;
  map<int,string> banknames;
  map<int,string>::iterator banknameidx;
  regex thisregex;
  smatch thismatchlist;
  string thiscolname;
  int bankidx;
  size_t rowcount, ridx;
  map<string,string> thisrow;
  indextype_t coeffcount;


  // Read the raw CSV data.
  tabdata = nloop_ReadCSV(infile);


  // First pass: Get column names for remapped banks.

  banknames.clear();
  thisregex = "bank\\s+(\\d+)";
  for (tabcolidx = tabdata.begin(); tabdata.end() != tabcolidx; tabcolidx++)
  {
    thiscolname = tabcolidx->first;

    // NOTE - "regex_match" looks for an exact match (whole line matches).
    // "regex_search" looks for a substring that matches.
    if (regex_match(thiscolname, thismatchlist, thisregex))
    {
      bankidx = stoi(thismatchlist[1]);

      if (bankremap.count(bankidx))
        bankidx = bankremap[bankidx];

      banknames[bankidx] = thiscolname;
    }
  }


  // Second pass: For each _column_, walk through table _rows_. Using rows
  // that match the criteria, build a FIR.

  rowcount = nloop_GetCSVRowCount(tabdata);

  for (banknameidx = banknames.begin();
    banknames.end() != banknameidx;
    banknameidx++)
  {
    // Get the column name and remapped bank index for this bank.

    bankidx = banknameidx->first;
    thiscolname = banknameidx->second;


    // Iterate through the rows, building this FIR.

    filtbank.BlankOneFilter(bankidx);
    coeffcount = 0;

    for (ridx = 0; ridx < rowcount; ridx++)
    {
      // This fills in "" for any missing column cells.
      // Running stoi on "" gives 0, which we can live with.
      thisrow = nloop_GetCSVRowCells(tabdata, ridx);

      if ( nloop_CSVRowMatchesAllCriteria(thisrow, matchcriteria) )
      {
        // FIXME - Assume 64-bit is larger than samptype_t.
        // If the user is using uint64_t, this will lose have the range.

        // This does bounds checking for us, so don't check it here.
        filtbank.SetOneCoefficient( bankidx, coeffcount,
          nloop_LLToSample<samptype_t>( stoll(thisrow[thiscolname]) ) );
        coeffcount++;
      }

    } // Row iteration.

    // This does bounds checking on coeffcount, so it's safe.
    filtbank.SetOneGeometry(bankidx, fracbits, coeffcount);

  } // Bank iteration.
}



// This writes a FIR filter to a stream.
// NOTE - This only writes active banks.
// NOTE - This doesn't save "fracbits"! The caller has to keep track of that.
// No extra output columns.

template<class samptype_t, class indextype_t, class filtbanktype_t>
void nloop_WriteFIRCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header)
{
  list<string> col_order;
  map<string,string> col_values;

  // Wrap the extra-columns version.

  col_order.clear();
  col_values.clear();
  nloop_WriteFIRCoeffs<samptype_t,indextype_t,filtbanktype_t>(
    outfile, filtbank, want_header, col_order, col_values );
}



// This writes a FIR filter to a stream.
// NOTE - This only writes active banks.
// NOTE - This doesn't save "fracbits"! The caller has to keep track of that.
// With extra output columns (written before the coefficients).

template<class samptype_t, class indextype_t, class filtbanktype_t>
void nloop_WriteFIRCoeffs(ostream &outfile, filtbanktype_t &filtbank,
  bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values)
{
  int bankcount, bidx;
  list<string> colnames;
  list<string>::iterator nidx;
  map<string,vector<string>> colseries;
  vector<string> thiscolseries;
  string thiscolname, thisval;
  indextype_t coeffcount, maxcoeffcount, sidx;
  uint8_t fracbits;

  // Initialize column names.

  colnames.clear();
  for (nidx = extra_col_order.begin(); nidx != extra_col_order.end(); nidx++)
    colnames.push_back(*nidx);


  // Build per-bank data series.

  bankcount = filtbank.GetActiveBanks();
  colseries.clear();
  maxcoeffcount = 0;

  for (bidx = 0; bidx < bankcount; bidx++)
  {
    thiscolname = "bank ";
    thiscolname += to_string(bidx);
    colnames.push_back(thiscolname);

    thiscolseries.clear();
    filtbank.GetOneGeometry(bidx, fracbits, coeffcount);

    if (coeffcount > maxcoeffcount)
      maxcoeffcount = coeffcount;

    for (sidx = 0; sidx < coeffcount; sidx++)
      thiscolseries.push_back(
        to_string( filtbank.GetOneCoefficient(bidx, sidx) ) );

    colseries[thiscolname] = thiscolseries;
  }

  // Add the extra columns.

  for (nidx = extra_col_order.begin(); nidx != extra_col_order.end(); nidx++)
  {
    thiscolname = *nidx;
    thisval = extra_col_values[thiscolname];

    thiscolseries.clear();
    for (sidx = 0; sidx < maxcoeffcount; sidx++)
      thiscolseries.push_back(thisval);

    colseries[thiscolname] = thiscolseries;
  }

  // Write the table.
  nloop_WriteCSV(outfile, colnames, colseries, want_header);
}



//
// Lookup Table I/O Functions

// These read and write lookup table tuples.

// When reading, columns unrelated to tuple address and data are ignored.
// NOTE - only the LUT entries listed in the CSV file are modified. Other
// entries in the LUT are left as-is.

// When writing, extra columns with constant values may be added.
// See LUTVALUES.txt for file format information.


// Reading a single lookup table.
// Treat all CSV rows as being part of this lookup table.

template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTableSingle( istream &infile, luttype_t &lut,
  string infield, string outfield )
{
  multimap<string,string> criteria;

  // Wrap the criteria-filtered version.

  criteria.clear();

  nloop_ReadLookupTableSingle(infile, lut, infield, outfield, criteria);
}



// Reading a single lookup table.
// If match criteria are supplied, only table rows that match all of the
// specified criteria are used.
// Match criteria are (column name, cell value) tuples.

template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTableSingle( istream &infile, luttype_t &lut,
  string infield, string outfield, multimap<string,string> &matchcriteria )
{
  map<string,vector<string>> tabdata;
  size_t line_count, lidx;
  map<string,string> thisline;
  int lutridx;
  intype_t inval;
  outtype_t outval;


  // Read the raw CSV data.
  tabdata = nloop_ReadCSV(infile);

  // Get the number of CSV rows.
  line_count = nloop_GetCSVRowCount(tabdata);


  // Iterate through CSV rows.
  for (lidx = 0; lidx < line_count; lidx++)
  {
    // Fetch this CSV row.
    thisline = nloop_GetCSVRowCells(tabdata, lidx);

    // Check match criteria.
    // Proceed if the CSV row matches.
    if ( nloop_CSVRowMatchesAllCriteria(thisline, matchcriteria) )
    {
      // NOTE - Entries that aren't present will get created with "".
      // Running stoi on "" gives 0, which we can live with.

      lutridx = stoi(thisline["row"]);

      // FIXME - Assuming signed 64-bit is larger than intype_t/outtype_t.
      // If the user is using uint64_t, this will lose half the range.

      inval = nloop_LLToSample<intype_t>( stoll(thisline[infield]) );
      outval = nloop_LLToSample<outtype_t>( stoll(thisline[outfield]) );

      // Set this LUT entry.
      lut.SetEntry(lutridx, inval, outval);
    }
  }
}



// Reading a class with per-bank lookup tables.
// Treat all CSV rows as being part of this lookup table.
// Don't remap bank numbers.

template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTablePerBank( istream &infile, luttype_t &lut,
  string infield, string outfield )
{
  multimap<string,string> criteria;
  map<int,int> bankremap;

  // Wrap the criteria-filtered bank-remapped version.

  criteria.clear();
  bankremap.clear();

  nloop_ReadLookupTableSingle( infile, lut, infield, outfield,
    criteria, bankremap );
}



// Reading a class with per-bank lookup tables.
// If match criteria are supplied, only table rows that match all of the
// specified criteria are used.
// Match criteria are (column name, cell value) tuples.
// Bank numbers in the remap table are remapped ( k -> bankremap[k] ).

template <class intype_t, class outtype_t, class luttype_t>
void nloop_ReadLookupTablePerBank( istream &infile, luttype_t &lut,
  string infield, string outfield,
  multimap<string,string> &matchcriteria, map<int,int> &bankremap )
{
  map<string,vector<string>> tabdata;
  size_t line_count, lidx;
  map<string,string> thisline;
  int bankidx, lutridx;
  intype_t inval;
  outtype_t outval;


  // Read the raw CSV data.
  tabdata = nloop_ReadCSV(infile);

  // Get the number of CSV rows.
  line_count = nloop_GetCSVRowCount(tabdata);


  // Iterate through CSV rows.
  for (lidx = 0; lidx < line_count; lidx++)
  {
    // Fetch this CSV row.
    thisline = nloop_GetCSVRowCells(tabdata, lidx);

    // Check match criteria.
    // Proceed if the CSV row matches.
    if ( nloop_CSVRowMatchesAllCriteria(thisline, matchcriteria) )
    {
      // NOTE - Entries that aren't present will get created with "".
      // Running stoi on "" gives 0, which we can live with.

      lutridx = stoi(thisline["row"]);
      bankidx = stoi(thisline["bank"]);

      if (bankremap.count(bankidx))
        bankidx = bankremap[bankidx];

      // FIXME - Assuming signed 64-bit is larger than intype_t/outtype_t.
      // If the user is using uint64_t, this will lose half the range.

      inval = nloop_LLToSample<intype_t>( stoll(thisline[infield]) );
      outval = nloop_LLToSample<outtype_t>( stoll(thisline[outfield]) );

      // Set this LUT entry.
      lut.SetOneEntry(bankidx, lutridx, inval, outval);
    }
  }
}



// Writing a single lookup table.
// No extra columns.
// This only writes active rows.

template<class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTableSingle( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header)
{
  list<string> col_order;
  map<string,string> col_values;

  // Wrap the extra-columns version.

  col_order.clear();
  col_values.clear();

  nloop_WriteLookupTableSingle<intype_t,outtype_t,luttype_t>(
    outfile, lut, infield, outfield, want_header, col_order, col_values );
}



// Writing a single lookup table.
// With extra columns (written before the tuple contents).
// This only writes active rows.

template<class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTableSingle( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values)
{
  int rowcount;
  int ridx;
  list<string> colnames;
  map<string,vector<string>> colseries;
  list<string>::iterator nidx;
  intype_t inval;
  outtype_t outval;


  // Get active geometry.

  rowcount = lut.GetActiveRows();

  // Construct column label list.

  colnames.clear();

  for (nidx = extra_col_order.begin(); nidx != extra_col_order.end(); nidx++)
    colnames.push_back(*nidx);

  colnames.push_back("row");
  colnames.push_back(infield);
  colnames.push_back(outfield);


  // Construct data series.

  // Initialize to suppress compiler warnings.
  // It should be possible to cast zero to any type that's used for this.
  inval = 0;
  outval = 0;

  colseries.clear();

  for (ridx = 0; ridx < rowcount; ridx++)
  {
    lut.GetEntry(ridx, inval, outval);

    // LUT index.
    colseries["row"].push_back(to_string(ridx));

    // LUT tuple.
    colseries[infield].push_back(to_string(
      nloop_SampleToLL<intype_t>(inval) ));
    colseries[outfield].push_back(to_string(
      nloop_SampleToLL<outtype_t>(outval) ));

    // Extra columns.
    for (nidx = extra_col_order.begin();
      nidx != extra_col_order.end();
      nidx++)
      colseries[*nidx].push_back(extra_col_values[*nidx]);
  }


  // Write the table.
  nloop_WriteCSV(outfile, colnames, colseries, want_header);
}



// Writing a class with per-bank lookup tables.
// No extra columns.
// This only writes active banks and rows.

template<class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTablePerBank( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header)
{
  list<string> col_order;
  map<string,string> col_values;

  // Wrap the extra-columns version.

  col_order.clear();
  col_values.clear();

  nloop_WriteLookupTablePerBank<intype_t,outtype_t,luttype_t>(
    outfile, lut, infield, outfield, want_header, col_order, col_values );
}



// Writing a class with per-bank lookup tables.
// With extra columns (written before the tuple contents).
// This only writes active banks and rows.

template<class intype_t, class outtype_t, class luttype_t>
void nloop_WriteLookupTablePerBank( ostream &outfile, luttype_t &lut,
  string infield, string outfield, bool want_header,
  list<string> &extra_col_order, map<string,string> &extra_col_values)
{
  int bankcount, rowcount;
  int bidx, ridx;
  list<string> colnames;
  map<string,vector<string>> colseries;
  list<string>::iterator nidx;
  intype_t inval;
  outtype_t outval;


  // Get active geometry.

  bankcount = lut.GetActiveBanks();
  rowcount = lut.GetActiveRows();

  // Construct column label list.

  colnames.clear();

  for (nidx = extra_col_order.begin(); nidx != extra_col_order.end(); nidx++)
    colnames.push_back(*nidx);

  colnames.push_back("bank");
  colnames.push_back("row");
  colnames.push_back(infield);
  colnames.push_back(outfield);


  // Construct data series.

  // Initialize to suppress compiler warnings.
  // It should be possible to cast zero to any type that's used for this.
  inval = 0;
  outval = 0;

  colseries.clear();

  for (bidx = 0; bidx < bankcount; bidx++)
    for (ridx = 0; ridx < rowcount; ridx++)
    {
      lut.GetOneEntry(bidx, ridx, inval, outval);

      // LUT indices.
      colseries["bank"].push_back(to_string(bidx));
      colseries["row"].push_back(to_string(ridx));

      // LUT tuple.
      colseries[infield].push_back(to_string(
        nloop_SampleToLL<intype_t>(inval) ));
      colseries[outfield].push_back(to_string(
        nloop_SampleToLL<outtype_t>(outval) ));

      // Extra columns.
      for (nidx = extra_col_order.begin();
        nidx != extra_col_order.end();
        nidx++)
        colseries[*nidx].push_back(extra_col_values[*nidx]);
    }


  // Write the table.
  nloop_WriteCSV(outfile, colnames, colseries, want_header);
}



//
// This is the end of the file.
