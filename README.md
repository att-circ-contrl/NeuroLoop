# NeuroLoop Module Library v1

## Overview

This is a modular set of libraries for performing real-time detection and
characterization of transient oscillations in the local field potential
("LFP bursts") and for producing stimulation triggers that are phase-aligned
to oscillations.

Implementations are provided in C++, Matlab, and VHDL and/or Verilog, to
support rapid development of detection/trigger systems suitable for
closed-loop experiments using desktop computers, embedded computers, or
FPGA-based ephys equipment.

The NeuroLoop project is copyright (c) 2020 by Vanderbilt University, and
is released under the Creative Commons Attribution 4.0 International
License.


## Discussion

The purpose of this project is to allow rapid development of embedded
C++ or FPGA code for use with closed-loop neural stimulation experiments
and other experiments that require on-line real-time detection of LFP
oscillation events. Each of the three library implementations supports that
goal in a different way:

* The Matlab implementation operates offline, processing the entire time
series at once. This is the most computationally efficient way to perform
module processing, as it leverages Matlab's optimized vector-processing
functions. This is intended for use during the design stage, allowing
different system configurations to be tested with minimum development effort.

* The C++ implementation operates sample-by-sample, and so can be used to
implement on-line real-time detection systems either on embedded hardware
or on desktop computers. It is furthermore written to have module geometries
and interfaces that are almost identical to the HDL implementations, so that
C++ prototypes may be used to validate or emulate HDL designs.

* The HDL implementation operates sample-by-sample, and serializes parallel
operations through pipelined execution units to conserve hardware (which is
presumed to be more scarce than clock cycles, as the logic clock rate
typially greatly exceeds the signal sampling rate). This can be used to
implement on-line real-time detection systems using FPGA resources provided
by ephys systems. The idea is to provide known-good "canned" code for as
much of such a system as possible, to reduce the development and debugging
effort needed.


## Documentation

The following directories contain documentation:

* manual -- LaTeX build directory for project documentation.
Use `make -C manual` to build it.


## Libraries

The following directories contain library code:

* code-cpp --
C++ implementation of NeuroLoop modules.
* (FIXME - Matlab modules go here.)
* (FIXME - VHDL and/or Verilog modules go here.)


## Sample Code

(FIXME -- Use the Burst Box or a stripped-down variant as sample code.)


This is the end of the file.
