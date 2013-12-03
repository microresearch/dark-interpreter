/* main buffer processing */

/* buffer outline:

2 or 3 buffers assigned which can be swapped (not copied but as pointers to processes)|:

1- 16 bit audio 96k (not be be swapped) always as 3 blocks SRAM : 112KB + 16KB main block (assigned or not?)

2- datagen output/working buffer - or are these separate buffers(3-) guess the same

as 64KB (64 is ccmdata see delay code)

#include "mainbuffer.h"
