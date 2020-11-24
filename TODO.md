# NeuroLoop module library v1 bug list, feature requests, and changelog.


## To-do list and bug list for version 1:

* Some code isn't sign-agnostic. Reading den0 for biquads from files uses
type int64_t, so it'll get the wrong value for extremely large uint64_t
values.

* Add "is_signed<T>" or "numeric_limits<T>.is_signed" checks where
appropriate for sign-sensitive code.


## Deferred to version 2:


## Abbreviated changelog:

* 24 Nov 2020 --
Biquad IIR filter banks pass initial smoke tests.
Added support for reading/writing biquad coefficients from/to CSV files.

* 23 Oct 2020 --
Repository skeleton and the start of C++ code.


This is the end of the file.
