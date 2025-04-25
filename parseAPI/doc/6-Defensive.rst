.. _sec:defmode:

Defensive Mode Parsing
======================

Binary code that defends itself against analysis may violate the
assumptions made by the the ParseAPI’s standard parsing algorithm.
Enabling defensive mode parsing activates more conservative assumptions
that substantially reduce the percentage of code that is analyzed by the
ParseAPI. For this reason, defensive mode parsing is best-suited for use
of ParseAPI in conjunction with dynamic analysis techniques that can
compensate for its limited coverage of the binary code.
