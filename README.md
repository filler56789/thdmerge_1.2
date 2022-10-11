# thdmerge 1.2
TrueHD AC-3 interleaver

thdmerge (version 1.2) is a program that interleaves TrueHD streams with AC-3 frames for playback-compatibility with hardware Blu-Ray players.

Author = Donald Graft (<donald.graft@cantab.net>).

USAGE:

thdmerge input.thd input.ac3 output.thd+ac3 [-t]

-t: truncate longer stream to length of shorter stream

NOTICE:

input.ac3 can be a silent and mono stream;

the ac3 bitrate must be equal to or greater than 64 kbps.
