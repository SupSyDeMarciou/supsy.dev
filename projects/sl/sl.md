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

# Structure of the library
## Files
The library is divided into three main modules :
- **Math**: structures and functions which are useful for linear algebra, 3D graphics and basic arithmetic. It contains `math.h`, `vector.h`, `quaternion.h`, `matrix.h`, `block_matrix.h`, `noise.h` and `algorithm.h`
- **Struct**: useful structures, some of which are *generic* and can thus be reused with any user-defined type. It contains `allocator.h`, `array.h`, `list.h`, `dict.h`, `arena.h` and `string.h`.
- **Misc**: miscellaneous utility files which are only bundled together because they didn't fit the above two categories. It contains `async.h`, `io.h`, `log.h`, `tui.h`

Additionnaly `base.h` defines shared definitions accross most headers.

## Synthax overview
Everything in the library is prefixed by either `SL_` (functions and macros) or `sl_` (types). This prefix, serving as a namespace, can be removed by defining `SL_STRIP_PREFIX` before including the library. This system is heavily inspired by Tsoding's *nob.h*.

The code base is written with the following synthax :
- Types are `snake_case`
- Constants and global static variables are `SNAKE_CASE`
- Functions are by default `camelCase`, and their "*overloads*" are separated using an underscore (ex: `functionName_overload(...)`). This allows me to synthaxically group similar functions together. 
- Functions operating on a specific type are prefixed by the type name (example: `fv2add(...)` does an addition (`add`) between two 2D vectors of floats (`fv2`))
- Some types have contructors which are either called `type {type}_(...)` (may have an overload letter in place of the `_`) or `type {typeButInCamelCase}Create(...)` depending on wether the initialization is done at compile-time or during the run-time.
- Declarative macros used to define new compound types like dynamic arrays or linked lists are called `SL_DEF_{compound_type}(...)`.

In addition, some functions and macros are only intended to be used by the **SL** and are thus prefixed with a double underscore (`__SL_...`) to signify this. These don't get their prefix removed by defining `SL_STRIP_PREFIX`.  

# Content overview
### *base.h*
This file serves as the lowest level include. It includes the standard library headers and defines constructs shared by the other headers.

It defines many aliases to builtin types (`int{n}_t -> i{n}`, `uint{n}_t -> u{n}`, `float -> f32`, `double -> f64`, *etc...*), and their pointer types (the generic structures like dynamic arrays heavely rely on having *typedef-ed* names so this is a compromise to have). It defines :
- `usize`, `ssize`: aliases for `size_t` and `ssize_t`. 
- `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `uint`: aliases for builtin sized integer types.
- `f16`, `f32`, `f64`, `f128`: aliases for floating-point number types.
- `ch8`, `ch16`, `ch32`: aliases for sized character types.

It also defines the error management constructs (`SL_ERROR`, `SL_strerr`) and memory-related constructs like the macro `SL_new(...)` which allocates and fills the allocation with its input or `SL_memclone` (on which `SL_new` is based).

## *Math*

The math module defines vector, matrice and quaternion structures and operations, as well as utilities for basic number manipulation and noise generation.

A convention is used troughout the module :
- `s` and `v` suffix: overload of a binary operation (in the send *"operations with two inputs"*) on which the second operand is now either a scalar (`s`) or a vector (`v`).
> Example: `fm2x2mul` does matrix multiplication of two 2x2 float matrices, whereas `fv2muls` scales the matrix by a float and `fv2mulv` does matrix-vector multiplication with right operand being a 2D vector of floats.

- `S`, `M` and `T` prefix or suffix : the first or second operand is scaled (`S`), multiplied (`M`) and/or transposed (`T`).  
> Example: `iv3sub(lhs, rhs)` does `lhs - rhs`, whereas `iv3subS(lhs, rhs, s)` does `lhs * s - rhs`, `iv3subSM(lhs, rhs, s, m)` does `lhs - rhs * s * m` (with `m` a `iv3`) and `iv3Ssub(lhs, s, rhs)` does `lhs * s - rhs`.

- `_u` suffix : at least one operand is assumed to be of unit length (specified within the function's description. Used to reduce redundant normalisation operations).

### *math.h*

This file defines constants and operations on core mathematical C types like `u32` and `double` (abreviated as `d`).

It defines:
- Constants: 
    - Mathematical constants: `PI`, `TAU` *(2 pi)*, `E`, `PHI` *(golden ratio)*.
    - Conversion constants: `DEG_TO_RAD` *(conversion from degrees to radians)*, `RAD_TO_DEG` *(conversion from radians to degrees)*.

- Operations:
    - Min and max: `umin`, `u64min`, `imin`, `i64min`, `fmin`, `dmin`, `umax`, `u64max`, `imax`, `i64max`, `fmax`, `dmax`.
    - Sign: `isign`, `i64sign`, `fsign`, `dsign`.
    - Bit reinterpretation: `inplaceU32ToFloat`, `inplaceFloatToU32`, `inplaceU64ToDouble`, `inplaceDoubleToU64`, `inplaceU32ToFloat01` *(only uses bits in the mantissa to keep output normalized)*, `inplaceU64ToDouble01`.
    - Random: `u32hash_PCG`, `u32srand`, `u32rand`, `u32rand_in`, `frand`, `drand`, `frand_in`, `drand_in`.
    - Stepping (rounding): `fstep`, `dstep`.
    - Linear interpolation: `flerp`, `dlerp`.
    - Clamping: `fclamp`, `dclamp`.

### *vector.h*
This file defines vectors of *2*, *3*, *4* and an *arbitrary number* of components for the types `bool` (`b`), `i8`, `u8`, `i16`, `u16`, `i32` (or `i`), `u32` (or `u`), `i64` (or `li`), `u64` (or `lu`), `float` (`f`) and `double` (`d`). 
> For example:
> - `fv2` is a vector of two floats.
> - `bv4` is a vector of four booleans.
> - `i64v` is a vector of i64 of arbitrary component count.

For a vector of type `T` and arbitrary dimension, the structure simply is:
```C
typedef struct {
    const usize count; 
    T *data;
} Tv;
```
For a vector of type `T` and dimension 4, the structure is more complex:
```C
typedef union {
    T data[4];
    struct {
        union { T x, r, u; };
        union { T y, g, v; };
        union { T z, b, s; };
        union { T w, a, t; };
    };
    struct {
        union { T __x0, __r0, __u0; };
        union { Tv2 yz, gb, vs; };
        union { T __w0, __a0, __t0; };
    };
    struct {
        union { Tv2 xy, rb, uv; };
        union { Tv2 zw, ba, st; };
    };
    struct {
        union { Tv3 xyz, rgb, uvs; };
        union { T __w1, __a1, __t1; };
    };
    struct {
        union { T __x1, __r1, __u1; };
        union { Tv3 yzw, gba, vst; };
    };
} Tv4;
```

The ***SL*** makes heavy use of `union` to create aliases in arithmetic types, so that a `fv4` may be used as an extended 3D coordinate (`.xyzw`), a color (`.rgba`) or a 4D UV coordinate (`.uvst`). In addition, you can extract sub-vectors from largers ones using this `union` trick (`.yz` extracts the middle `fv2` of a `fv4`) which acts as a weak form of GLSL's swizzling.

Though vectors are very diverse, they have some common function definitions:
- Addition and subtraction: `add` *(addition)*, `addS`, `addSM`, `Sadd`, `sub` *(subtraction)*, `subS`, `subSM`, `Ssub`.
- Component-wise multiplication and division: `mul` *(multiplication)*, `muls`, `div` *(division)*, `divs`.  
    **/!\ `mul` and `div` are component-wise operations, as vectors don't usually define multiplication and division**.
- Vector arithmetic: `equ` *(equality)*, `dot` *(dot product)*, `len` *(euclidean length)*, `len_sqr` *(squared euclidean length)*, `len_max` *(infinite length, aka. max component)*, `len_manh` *(manhattan or taxicab length)*, `dist` *(length of the difference of two vectors)*.
- Combination: `min` *(component-wise minimum)*, `max` *(component-wise maximum)*, `mix` *(addition of vectors, each scaled by a scalar)*.  

In addition, some functions are only defined for specific vectors types:
- Non-boolean types: `mod` *(component-wise modulus)*, `mods`.
- Signed types: `neg` *(negation)*, `abs` *(component-wise absolute value)*, `refl{_u}` *(reflection around axis)*, `align{_u}` *(projection on axis)*, `proj{_u}` *(projection on plane orthogonal to axis)*.
- Real types:
    - Normalisation: `norm` *(normalization)*.
    - Rounding: `floor`, `ceil`, `frac` *(fractional part)*. **These are component-wise operations**.
    - Interpolation: `lerp` *(linear interpolation)*, `serp` *(spherical interpolation)*.
- Integer and boolean types - binary operations: `and`, `or`, `xor`, `not`. **These are component-wise operations**.
- Integer types - shifting: `lshft` *(left shift)*, `lshfts`, `rshft` *(right shift)*, `rshfts`. **These are component-wise operations**.
- 2D and 3D signed types - cross product `cross` *(cross product)*. **In 2D, returns the *sin* of the angle formed by the two vectors**.
- 2D and 3D real types:
    - `fv2`, `dv2`: `angle` *(atan2(v.y, v.x))*, `from_angle` *(get cos/sin pair)*, `rot` *(rotate by angle)*, `rot_sc` *(rotate by sin/cos pair)*, `rot_cs`, `orth` *(rotate by +90°)*
    - `fv3`, `dv3`: `from_yawPitch`

### *quaternion.h*
This file defines quaternions of floats (`fq`) and doubles (`dq`). These are mainly used for their interesting properties regarding 3D rotations.  
Here is the definition of `fq` (`dq` is equivalent with `double` in place of `float`): 
```C
typedef union {
    float data[4];
    struct { float a, b, c, d; };
    struct { float w, x, y, z; };
    struct { float r; union { fv3 iv; struct { float i, j, k; }; }; };
    fv4 fv4;
} fq;
```
`union`s are once again used for aliasing as it can be useful to consider quaternions either as a 4D vector or as a real component and an imaginary 3D vector.

Quaternions implement the following operations:
- Common operations: `equ`, `add`, `sub`, `mul`, `muls`, `div`, `len_sqr`, `len`.
- Transformations: `neg` *(negation)*, `trsp` *(transposition)*, `inv` *(inversion)*, `norm` *(normalization)*. 
- Exponetial: `exp` *(***e** *raised to the operand)*, `ln` *(natural logarithm of the operand)*, `ln_u`, `pow` *(first operand to the power of the second operand)*, `pow_u`.
- Rotation: `rot` *(rotation of a 3D vector)*.
- Interpolation: `serp` *(spherical interpolation)*, `serp_u`.

The following conversions are also defined for quaternions and 3D rotation representations:  
- Euler angles: `{from|to}_euler` *(euler angles in roll-pitch-yaw order)*.
- Vectors: `{from|to}_axisAngle` *(rotation around 3D axis)*, `from_fromTo` *(rotation to align starting vector to destination vector)*

### *matrix.h*

I can't be bothered for now...

## *Struct*
This module defines generic structure types like dynamic arrays, linked lists and dictionnaries, as well as custom allocators and sized strings.

All of the generic types can be configured to use a custom allocator when they need to expand/free their memory footprint. This allocator is generaly assigned by using a type's constructor with suffix `A` (example: `listCreateA(...)` creates an empty list with custom allocator).

### *allocator.h*
This file defines the custom allocator type recognized throughout the **SL** (`allocator`), as well as some macros to incorporate it with the standard library's allocator (refered to as the `std_allocator` or simply `NULL` in the **SL**).

An allocator can be created using the `allocator_(...)` constructor, and is simply a set of functions to use when menipulating memory. These internal functions have signatures equivalent to their `stdlib.h` counterparts so they may be used as direct replacements.

The following macros are defined to create allocator-independent code: 
- `aalloc(allocator, size)`: allocate a chunk of data.
- `azalloc(allocator, size)`: allocate a chunk of data which is zero-initialized.
- `arealloc(allocator, ptr, newsize)`: reallocate an already allocated chunk of data.
- `afree(allocator, ptr)`: free an allocated chunk of data.
- `aclone(allocator, ptr, size)`: clone an allocated chunk of data.

### *arena.h*
This file defines a simple implementation of an arena allocator using the **SL** allocator representation. Being a simple arena allocator, it only supports allocating new memory and not freeing parts of memory, however it can grow like a dynamic array by a `page_size` (define as a constructor parameter) everytime an allocation does not fit the current sector.

The arena implements the following functions:
- Constructor: `arenaCreate`. 
- Dstructor: `arenaDestroy`.

### *array.h*

This file defines the dynamic array type (`array(T)`) and the slice type (`slice(T)`), as well as some functions to manipulate them in a type independent manner. All read-only functions can be used with both arrays or slices indiscriminetly, but memory manipulating functions should only be used with dynamic arrays (as slices don't contain the information necessary for memory resizing).  
To define a new array, use the declarative macro:
```C
// Define an array and a slice of type T
SL_DEF_ARRAY(T);

// The array type and slice type can now be accessed using
SL_array(T) array = {0}; // Empty array is a valid array
SL_array(T) array = SL_arrayCreate(T, ...); // Preallocate capacity
SL_slice(T) slice = SL_slicev(T, ...); 

// The `SL_array(T)` and `SL_slice(T)` types have the following definitions :
typedef struct SL_array(T) { 
    T *data;
    usize count;
    usize capa; 
    sl_allocator *alloc;
} SL_array(T);

typedef struct SL_slice(T) { 
    T *data;
    usize count;
} SL_slice(T);
```
As you can see, the array keeps track of its capacity and its allocator to efficiently grow (it doubles in size whenever it's full to allow for amortised *O(1)* insertion).

The array implements the following functions:
- Slice constructors: `slice_` *(wrap C array)*, `slicev` *(wrap arbitrary arguments)*, `slicea` *(take a sectionn of a dynamic array)*.
- Array constructors: `arrayCreate`, `arrayCreateA`, `arrayDestroy`, `arrayClone`, `arrayCloneA`, `arrayWrap`, `arrayWrap_var`, `arrayFrom`, `arrayFromA`, `arrayFrom_var`, `arrayFromA_var`. **The `wrap` functions don't allocate any data but directly use the provided array, the resulting array is thus not necessarily safe when used with memory manipulating functions (especially in the case of wrapped static array)**.
- Accessors: `arrayFirst` *(pointer to first element)*, `arrayLast` *(pointer to last element)*, `arrayAt` *(pointer to element at index)*.
- Data insertion/deletion: `arrayInsert_range` *(insert within the array)*, `arrayInsert`, `arrayInsert_var`, `arrayAdd_range` *(insert at end of array)*, `arrayAdd`, `arrayAdd_var`, `arrayCat` *(concatenation)*, `arrayRemove_range` *(remove within the array)*, `arrayRemove`, `arrayRemove_unordered` *(remove without keeping order (faster than ordered remove))*, `arrayPop` *(remove and return value)*.
- Data rearanging: `arraySort` *(using quicksort)*, `arrayFill` *(set all elements to same value)*, `arrayReserve` *(pre-allocate certain capacity)*.
- Printing and putting: `arrayPrintf_full`, `arrayPrintf`, `putArray_full`, `putArray`.  

In addition, the `aforeach` loop is introduced as a simpler replacement for a `for` loop across every element:
```C
// Create a static array 
SL_slice(iv2) vectors = SL_slicev(iv2, iv2_right, iv2_up, iv2_one);

// Print the array before the loop
SL_put("[BEFORE LOOP] vectors = ", SL_putArray_full(vectors, FMT_V2("%d"), vector, XPD_V2(*vector)), "\n");
// Expect: "[BEFORE LOOP] vectors = array[v2(1, 0), v2(0, 1), v2(1, 1)]\n"

SL_aforeach(var, vectors) // Litteraly "for each `vector` in `vector` do {...}"
{
    if (aindex(var) == 1 && var->y == 0) *var = iv2_(5, 5);
    //  ~~~~~~~~~~~~~~
    //  Litteraly "index of variable `vector` in the looping array" 
}
// Print the array after the loop
SL_put("[AFTER LOOP] vectors = ", SL_putArray_full(vectors, FMT_V2("%d"), vector, XPD_V2(*vector)), "\n");
// Expect: "[AFTER LOOP] vectors = array[v2(1, 0), v2(5, 5), v2(1, 1)]\n"
//                                                 ~~~~~~~~
```
`aforeach` related additions:  
- `aindex` *(index of loop variable in loop array)*
- `aindex_in` *(index of a variable in an array)*

### *list.h*

This file defines the linked list type (`list(T)`) and the doubly linked list type (`dlist(T)`), as well as some functions to manipulate them in a type independent manner. Every function appart from constructors can be called with linked lists and doubly linked lists indiscriminetly. 
To define a new list, use the declarative macro:
```C
// Define an list and a dlist of type T
SL_DEF_LIST(T);

// The list type and dlist type can now be accessed using
SL_list(T)  list  = {0}; // Empty list is a valid list
SL_dlist(T) dlist = SL_dlistCreateA(T, ...); // Create list with allocator assigned

// The `SL_list(T)` and `SL_dlist(T)` types have the following definitions :
typedef struct T_list_node { 
    struct T_list_node *next;
    T data; 
} T_list_node;
typedef struct SL_list(T) { 
    T_list_node *first, *last; 
    usize count; 
    sl_allocator *alloc; 
} SL_list(T);

typedef struct T_dlist_node {
    struct T_dlist_node *next, *prev; 
    type data;
} T_dlist_node;
typedef struct SL_dlist(T) {
    T_dlist_node *first, *last; 
    usize count; 
    sl_allocator *alloc; 
} SL_dlist(T)
```
The list and dlist types have the same structure, only the associated node structures change to either only reference the next item *(list)* or both the previous and next item *(dlist)*.

The array implements the following functions :
- Constructors/Destructors: `listCreateA`, `dlistCreateA`, `listClear`.
- Accessors: `listFirst` *(pointer to first element)*, `listLast` *(pointer to last element)*, `listAt` *(pointer to element at index)*.
- Data insertion/deletion: `listInsert` *(insert within the list)*, `listAdd` *(insert at end of list)*, `listAdd_first` *(insert at start of list)*, `listRemove` *(remove within the list)*, `listRemove_ref`, `listPop` *(remove and return value)*.
- Printing and putting: `listPrintf_full`, `listPrintf`, `putList_full`, `putList`.

In addition, the `lforeach` loop is introduced as a simpler replacement for a `for` loop across every element :
```C
// Create a simple linked list 
SL_list(iv2) vectors = {0};
SL_listAdd(vectors, iv2_right);
SL_listAdd(vectors, iv2_up);
SL_listAdd(vectors, iv2_one);

// Print the list before the loop
SL_put("[BEFORE LOOP] vectors = ", SL_putList_full(vectors, FMT_V2("%d"), vector, XPD_V2(*vector)), "\n");
// Expect: "[BEFORE LOOP] vectors = list[v2(1, 0), v2(0, 1), v2(1, 1)]\n"

SL_lforeach(var, vectors) // Litteraly "for each `vector` in `vector` do {...}"
{
    if (lindex(var) == 1 && var->y == 0) *var = iv2_(5, 5);
    //  ~~~~~~~~~~~~~~
    //  Litteraly "index of variable `vector` in the looping list" 
}
// Print the list after the loop
SL_put("[AFTER LOOP] vectors = ", SL_putList_full(vectors, FMT_V2("%d"), vector, XPD_V2(*vector)), "\n");
// Expect: "[AFTER LOOP] vectors = list[v2(1, 0), v2(5, 5), v2(1, 1)]\n"
//                                                 ~~~~~~~~
```
`lforeach` related additions:
- `lindex` *(index of loop variable in loop array)*
- `lnext` *(pointer to element after loop variable)*
- `dlprev` *(pointer to element before loop variable)*

### *dict.h*

This file defines the dictionnary type (`dict(K, T)`) as well as some functions to manipulate them in a type independent manner.
To define a new dictionnary, use the declarative macro:
```C
// Define a dictionnary with key of type K and value of type T
SL_DEF_DICT(K, T);

// The dict type can now be accessed using
SL_dict(K, T) dict = SL_dictCreate(K, T, ...); // Create a dictionnary with pre-allocated buckets

// The `SL_dict(K, T)` type has the following definition :
typedef struct SL_dict(K, T)_bucket { 
    struct SL_dict(K, T)_bucket *next; 
    K key;
    T value;
} SL_dict(K, T)_bucket; // Basicaly a linked list node

typedef struct SL_dict(K, T) {
    SL_dict(K, T)_bucket **data;
    usize count, capa;
    SL_allocator *alloc;
    
    usize (*hash)(const K *);
    int   (*cmp)(const K *, const K *);
} SL_dict(K, T)
```
The dictionnary keeps track of a key comparison function and a key hashing function. When using the standard constructors, the functions declared using `DEF_CMP_FUNC` and `DEF_HASH_FUNC` for the considered key type. These can be overriden using `dictCreate_full` :
```C
SL_DEF_CMP_FUNC(K, key_a, key_b) {
    return *key_a == *key_b;
}
SL_DEF_HASH_FUNC(K, key) {
    return (usize)key ^ ((usize)key << 8);
}

SL_dict(K, T) dict_basic = SL_dictCreate(K, T, 256);
// <=>
SL_dict(K, T) dict_specific = SL_dictCreate_full(K, T, K_cmp, K_hash, 256);
```

The array implements the following functions :
- Constructors/Destructors: `dictCreate`, `dictCreate_full` *(specify hash and comparison functions)*, `dictCreateA`, `dictCreateA_full`, `dictClear` *(free buckets)*, `dictDestroy` *(free every resource taken by dict)*.
- Data accessing/insertion/deletion: `dictGet` *(pointer to element associated with a key)*, `dictAdd` *(insert value associated with a key)*, `dictRemove` *(remove value associated with a key)*, `dictKey` *(get key associated with value reference)*.
- Using dictionnary functions: `dictHash` *(use dict's hash function)*, `dictHash2`, `dictCmp` *(use dict's comparison function)*, `dictCmp2`.
- Printing and putting: 
    > ### /!\ TO COMPLETE /!\

In addition, the `dforeach` loop is introduced as a simpler replacement for a `for` loop across every element :
```C
SL_dict(SL_ptr(char), iv2) vectors = SL_dictCreate(SL_ptr(char), iv2, 3);
SL_dictAdd(vectors, "right", iv2_right);
SL_dictAdd(vectors, "up",    iv2_up);
SL_dictAdd(vectors, "one",   iv2_one);

SL_dforeach(pair, vectors) // Litteraly "for each `pair` in `vectors` do {...}"
{
    if (SL_dictCmp(pair->key, "up") && pair->value.y == 0) pair->value = iv2_(5, 5);
}
```

> ### /!\ TODO: CONTINUE /!\