# 2026q1 Homework1 (warmup)

## constant vs immutable in C99

Verify that C99 `const` means **immutable** (cannot modify through its name),
not **constant** (compile-time evaluable).

All tests live in a single `constant_immutable.c`.  Compile-time tests are
guarded by `#ifdef TEST_*` flags and driven by the Makefile; runtime experiments
build and execute the default `main()`.

### Build & Run

```
make constant_immutable_report
```

Builds with all GCC optimization levels (`-O0`, `-Og`, `-O1`, `-Os`, `-O2`,
`-O3`, `-Ofast`) and prints a grouped summary.

### Sample Output

```
constant vs immutable in C99 -- gcc 13

Compile-time: constant expression in case label (C99 6.6, 6.8.4.2)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     case_define: compiled
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     case_enum: compiled
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     case_sizeof: compiled
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     case_cast_float: compiled
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     case_char: compiled
  -O0                                      case_const_int: compile error
  -Og, -O1, -Os, -O2, -O3, -Ofast          case_const_int: compiled (gcc extension)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     arr[const_int]: VLA warning

Compile-time: mutability and addressability
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     enum_assign: compile error (expected)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     enum_addr: compile error (expected)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     const_addr: compiled (expected)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     define_redefine: compiled (expected)

Experiment 1: modify const local via cast (UB: C99 6.7.3.5)
  -O0                                      local_name=999 local_ptr=999
  -Og, -O1, -Os, -O2, -O3, -Ofast          local_name=100 local_ptr=999

Experiment 2: memory permissions (C99 6.7.3, footnote 114)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     g_const      perms=r--p writable=no
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     g_mutable    perms=rw-p writable=yes
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     local_const  perms=rw-p writable=yes
```

### Findings

- **Case labels**: `#define`, `enum`, `sizeof`, character constants, and float
  casts are constant expressions (C99 §6.6) — accepted in `case` labels at all
  optimization levels.  `const int` is **not** a constant expression; GCC
  rejects it at `-O0` but accepts it at `-Og+` due to constant folding (a GCC
  extension, not standard C99).
- **VLA**: `int a[n]` with `const int n` is a VLA — confirmed by
  `-Wvla -Werror=vla` triggering at all levels.
- **Mutability**: `enum` constants cannot be assigned or addressed (no memory
  location).  `const int` occupies memory and is addressable.  `#define` can be
  redefined via `#undef`.
- **Experiment 1**: Modifying a `const` local via pointer cast is undefined
  behavior (C99 §6.7.3.5).  Under `-O0` the compiler reads from memory and
  sees 999; from `-Og` onward, constant propagation replaces the read with the
  original value 100.
- **Experiment 2**: Global `const` is placed in a read-only page (`r--p`),
  while global mutable and stack variables are in read-write pages (`rw-p`).
  This confirms C99 §6.7.3 footnote 114.

## Function designator conversion test

Verify that C99 §6.3.2.1¶4 causes function designators to auto-convert to
function pointers, making `puts`, `*puts`, `&puts`, and `********puts` all
yield the same address — while `&fp` (a function pointer variable) yields
the variable's stack address.

### Build & Run

```
make func_designator_report
```

### Sample Output

```
Function designator conversion test -- gcc 13

puts        = 0x7e111696abe0
*puts       = 0x7e111696abe0
&puts       = 0x7e111696abe0
********puts= 0x7e111696abe0

fp          = 0x7e111696abe0
*fp         = 0x7e111696abe0
&fp         = 0x7fff51d55e10

call: (********puts)("Hello")
Hello
return value: 6
```

## Incomplete type (opaque pointer) test

Verify that C99 §6.2.5 incomplete types can only be used through pointers:
declaring objects, arrays, or using `sizeof` on an incomplete type are all
compile errors.

### Build & Run

```
make opaque_report
```

### Sample Output

```
Incomplete type (opaque pointer) test -- gcc 13

Compile-time: what works with incomplete type (C99 6.2.5)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     struct opaque *p: compiled
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     struct opaque x: compile error
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     struct opaque a[3]: compile error
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     sizeof(struct opaque): compile error

Runtime: sizeof pointer to incomplete type
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     sizeof(struct opaque *) = 8
```

## Object lifetime UB test (dangling pointer)

Demonstrate that returning the address of an automatic variable is UB per
C99 §6.2.4: the object's lifetime ends when the function returns, even though
the storage may not yet be reused and the pointer still holds the original
address.

Two versions contrast compiler behavior:
- `foo()` returns `&x` directly — GCC replaces with `return NULL` at all
  optimization levels (exploiting UB).
- `bar()` smuggles `&x` via an output parameter — at `-O0` the dangling
  read "works" (`*p2 = 42`), but at `-O1+` the value is garbage because
  the optimizer inlines `bar` and never writes `x` to the stack.

### Build & Run

```
make lifetime_ub_report
```

### Sample Output

```
Object lifetime UB test -- gcc 13

v1: foo() direct return (GCC replaces with NULL)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     foo(42) returned: p1 = (nil)

v2: bar() smuggled via output parameter
  -O0, -Og                                   p2 = 0x..., *p2 = 42
  -O1, -Os, -O2, -O3, -Ofast                 p2 = 0x..., *p2 = <garbage>

v3: after second bar(99) — storage reuse
  -O0, -Og                                   *p2 = 99  (storage reused)
  -O1, -Os, -O2, -O3, -Ofast                 *p2 = <garbage>
```
