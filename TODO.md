# NeuroLoop module library v1 bug list, feature requests, and changelog.


## To-do list and bug list for version 1:

* (C++) Reading den0 for biquads from files uses type int64_t, so it'll get
the wrong value for extremely large uint64_t biquad coefficients.

* (C++) NLOOP_ARITHSHR_UNSIGNED is much slower than a native arithmetic shift.
This should be replaced with pointer casting to a signed type, but the macro
doesn't actually know the type we should cast to.

* (C++) Add artifact rejection module.

* (C++) Make macro constants for functions/methods that take bool arguments,
to make code that calls them more readable.

* Add "LUTVALUES.txt" format definition file (generalized burst box format).


## Deferred to version 2:


## Abbreviated changelog:

* 25 Mar 2021 --
(C++) Fixed issue reading CSV files with CRLF under Linux.

* 02 Mar 2021 --
(C++) Added lookup tables (presently used for delay calibration).

* 26 Jan 2020 --
(C++) Added trigger logic. Split "misc" into "integers" and "slices".
Moved biquad coefficient notes elsewhere.

* 16 Dec 2020 --
(C++) Added peak/trough analytic estimator. Added threshold-based detector.

* 3 Dec 2020 --
(C++) Added auto-ranging preprocessor.

* 30 Nov 2020 --
(C++) Added type min/max/sign check macros, and added checks to
sign-sensitive code.
Made top-level headers, and added multiple-include wrappers to child headers.

* 24 Nov 2020 --
(C++) Biquad IIR filter banks pass initial smoke tests.
Added support for reading/writing biquad coefficients from/to CSV files.

* 23 Oct 2020 --
Repository skeleton and the start of C++ code.


This is the end of the file.
