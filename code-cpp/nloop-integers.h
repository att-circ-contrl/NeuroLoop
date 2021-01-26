// Attention Circuits Control Laboratory - NeuroLoop project
// Integer type tests.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

// Wrapper.
#ifndef NLOOP_INTEGERS_H
#define NLOOP_INTEGERS_H


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


// End of wrapper.
#endif


//
// This is the end of the file.
