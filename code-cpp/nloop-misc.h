// Attention Circuits Control Laboratory - NeuroLoop project
// Miscellaneous declarations.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_MISC_H
#define NLOOP_MISC_H


//
// Macros


// Sign-safe "arithemetic shift-right" operation.
// NOTE - Compilers generally default to arithmetic shift-right for signed
// types, but we can explicitly force that with NLOOP_SIGN_SAFE_SHIFT.

#ifdef NLOOP_SIGN_SAFE_SHIFT
// Sign-safe but slower macro.
#define NLOOP_ARITHSHR(X,Y) {	\
if ((X) < 0) {	\
  (X) = -(X);	\
  (X) >>= (Y);	\
  (X) = -(X);	\
} else { (X) >>= (Y); }	\
}
#else
// Fast but potentially sign-unsafe code. Requires arithmetic shift-right.
#define NLOOP_ARITHSHR(X,Y) { (X) >>= (Y); }
#endif


// Explicit "arithmetic shift-right" operation on an unsigned operand.
// This preserves the sign bit when shifting.
// FIXME - This is much slower than a true arithmetic shift! Re-cast to
// a signed type instead.

#define NLOOP_ARITHSHR_UNSIGNED(X,Y) {	\
if (NLOOP_UNSIGNED_ISNEG(X)) {	\
  NLOOP_UNSIGNED_NEGATE(X);	\
  (X) >>= (Y);			\
  NLOOP_UNSIGNED_NEGATE(X);	\
} else {			\
  (X) >>= (Y);			\
} }


// Macros for interpreting unsigned values as signed.

#define NLOOP_UNSIGNED_ISNEG(X) ( (~(X)) < (X) )

#define NLOOP_UNSIGNED_NEGATE(X) { (X) = ~(X); (X)++; }


// Wrappers for checking various things about integer types.
// NOTE - We'll need C++11 to handle 64-bit types properly with <limits>.

// FIXME - Add support for kludging this if we're using a compiler that
// doesn't have <limits>, such as 2014 avr-gcc.

#ifdef NLOOP_KLUDGE_LIMITS

// The kludge way: Bit twiddling to figure out what we're dealing with.
// This assumes two's complement and that logical operations work normally.
// NOTE - We're getting promotion to int32_t after ~ for certain cases.
// Extra casts can fix this.

#define NLOOP_ISSIGNED(T) ( ((T) 0) > ((T) (~((T) 0))) )

#define NLOOP_MAXVAL(T) ( NLOOP_ISSIGNED(T) ? \
  (~( nloop_SearchMinSignedHelper<T>() )) : ((T) (~((T) 0))) )
#define NLOOP_MINVAL(T) ( NLOOP_ISSIGNED(T) ? \
  nloop_SearchMinSignedHelper<T>() : ((T) 0) )

#else

// The right way: use the "numeric_limits" template from <limits>.

// This evaluates to a constant, so no parentheses after "is_signed".
#define NLOOP_ISSIGNED(T) std::numeric_limits<T>::is_signed
// These are evaluated as functions, but the compiler will hopefully
// optimize them to constants.
#define NLOOP_MAXVAL(T) std::numeric_limits<T>::max()
#define NLOOP_MINVAL(T) std::numeric_limits<T>::min()

#endif



//
// Functions


#ifdef NLOOP_KLUDGE_LIMITS

// Inline helper function for type min/max values.

// We don't have a good way to search for this, so do it the brute force way.
// NOTE - Use "-Wno-shift-count-overflow" to disable gcc's warnings for this.
// FIXME - As a ( ? : ) expression this is getting promoted to int32_t in
// testing, so use explicit variable typing instead. It's more readable, too.

template<class T> inline T nloop_SearchMinSignedHelper(void)
{
  T result;

  result = 1;
  result <<= 63;

  if (0 == result)
  {
    result = 1;
    result <<= 31;
  }

  if (0 == result)
  {
    result = 1;
    result <<= 15;
  }

  if (0 == result)
  {
    result = 1;
    result <<= 7;
  }

  return result;
}

#endif



//
// Data Buffer Class


// FIXME - There's no such thing as a templated typedef, so these have to
// be classes even for relatively simple constructs.

// This represents one "slice" of sample data across all channels and
// filter banks within a signal processing pipeline.

template <class samptype_t, int bankcount, int chancount>
class nloop_SampleSlice_t
{
public:
  samptype_t data[bankcount][chancount];

  void SetUniformValue(samptype_t newval);
};



//
// Code Inclusion

// C++ compiles templated classes on-demand. The source code has to be
// included so that the compiler can do this.
// Only one copy of each variant will actually be compiled; extra copies get
// pruned at link-time.


// FIXME - This should be in an "-inc.cpp" file.
template <class samptype_t, int bankcount, int chancount>
void nloop_SampleSlice_t<samptype_t,bankcount,chancount>::
SetUniformValue(samptype_t newval)
{
  int bidx, cidx;

  for (bidx = 0; bidx < bankcount; bidx++)
    for (cidx = 0; cidx < chancount; cidx++)
      data[bidx][cidx] = newval;
}


// End of wrapper.
#endif


//
// This is the end of the file.
