# SupSyLibraries
This C library acts as a bundle of multiple useful libraries for helping with linear arithmetics, generic structures and others.

You can find the repo here: [SupSyLibraries](https://github.com/SupSyDeMarciou/SL).

# Dependencies
This library is entirely written by myself, no dependencies to be found !

However, depending on the C standard and wether POSIX is used, some functions may not be defined and some macros may not be used. Indeed, a very limited number of functions make use of `_Generic` *(a C23 extension)*, `nanosleep` *(a POSIX standard function)* and some others.

The library has been compiled using **Cland** and **GCC** in projects using `-std=c99`, `-std=c11`, `-std=c23`, `-std=gnu99` and `-std=gnu23`. 

# How to use
This library is header only in the vain of the *STB libraries*. Therefore, you can simply do the following :
```C
#define SL_IMPLEMENTATION       // Define this marco to enable the definitions of every function in the SL. Define it only once in your project to avoid redefinition conflicts.
#define SL_STRIP_PREFIX         // Define this macro to remove almost every `SL_` or `sl_` prefix (these prefixes act like a namespace)

#include "SL/sl.h"              // Includes everything in the SL as recursive includes to every header
// or
#include "SL/sl_all.h"          // Includes everything in the SL from this single header (which is a concatenation of every file in the library generated automaticaly)
// or
#include "SL/struct/array.h"    // Only includes the relevant headers to make this header work.
```

> ### /!\ Be careful, some files are proceduraly generated and can get quite big so they might slow down your text editor if you open them (particularily `sl_all.h`, `vector.h` and `matrix.h`) /!\

> ### /!\ TODO: CONTINUE /!\