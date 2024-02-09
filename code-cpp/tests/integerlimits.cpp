// Attention Circuits Control Laboratory - NeuroLoop project
// Test program - Integer limit checks.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "testincludes.h"


//
// Macros

// The Right Way to do this is to make a template function, but a macro is
// acceptable for something this short.
// NOTE - To force interpreting uint8_t and int8_t as integers, use "+".
#define REPORT_TEST(S,T)					\
{								\
  cout << S << ":  ";						\
  cout << (NLOOP_ISSIGNED(T) ? "signed" : "unsigned");		\
  cout << "   min: " << +NLOOP_MINVAL(T);			\
  cout << "   max: " << +NLOOP_MAXVAL(T) << "\n";		\
}


//
// Main Program


int main(void)
{
  // Starting banner.
#ifdef NLOOP_KLUDGE_LIMITS
  cout << "\n== Integer limits check (kludged).\n\n";
#else
  cout << "\n== Integer limits check (normal).\n\n";
#endif


  REPORT_TEST("uint8_t", uint8_t);
  REPORT_TEST("uint16_t", uint16_t);
  REPORT_TEST("uint32_t", uint32_t);
  REPORT_TEST("uint64_t", uint64_t);
  REPORT_TEST("int8_t", int8_t);
  REPORT_TEST("int16_t", int16_t);
  REPORT_TEST("int32_t", int32_t);
  REPORT_TEST("int64_t", int64_t);


  // Ending banner.
  cout << "\n== End of integer limits check.\n\n";


  // Report success.
  return 0;
}


//
// This is the end of the file.
