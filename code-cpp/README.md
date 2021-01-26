# NeuroLoop Module Library - C++ Notes

Various design decisions that were made for reasons that might not be
obvious:

* I've avoided using inheritance where practical. This is to avoid vtable
lookups, which will slow down embedded code.

This results in certain oddities; the most obvious is passing what might
normally be parent classes as template arguments instead, and duplicating
code for setting the number of active channels and banks.

* I've avoided operator overloading, which would have simplified the
threshold comparison code (among other things). This is to avoid allocating
r-values for the boolean result on the stack, as embedded systems have
strong constraints on memory.

* I've implemented most functions to accept references and do in-place
modification of data for the same reason. The idea is to avoid duplication
and copying where possible (to minimize memory footprint).

* I've tried to keep a strict separation between signal processing
implementations and array (bank) implementations. This is to keep a 1:1
mapping between C++ and FPGA implementations. The idea is to have HDL code
for single-channel operations, and to implement bank versions by either
replicating in space or multiplexing in time or both, without duplicating
code.


_This is the end of the file._
