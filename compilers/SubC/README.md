This is a port of Nils M Holm's SubC (small subset of C) compiler for Videocore IV.
see: http://www.t3x.org/subc/ for the original compiler.

Notes:
- The run time has not yet been ported.
- No optimization.
- Untested.
- Generates a rather err 'unique' :) source format (C function & macros calls to generate binary) rather than 
the more traditional assembly source code.

Build:
- 'make scc0', will generate the compiler.
- 'scc0 -S test.c', to compile test.c to test.s.
- 'test.s' will need to be built using a tinyasm project like the ones at from https://github.com/hermanhermitage/videocoreiv.

Stay tuned for updates...
