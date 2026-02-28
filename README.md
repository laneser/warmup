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
