# 2026q1 Homework1 (warmup)

contributed by < `laneser` >

## constant vs immutable in C99

Verify that C99 `const` means **immutable** (cannot modify through its name),
not **constant** (compile-time evaluable).

### Experiments

| # | Question | C99 Reference |
|---|----------|---------------|
| 1 | Is a `const` variable a constant expression? | §6.6 |
| 2 | What happens when you cast away `const` and write? | §6.7.3.5 |
| 3 | Does the compiler place `const` objects in read-only memory? | §6.7.3 fn.114 |

### Build & Run

```
make constant_immutable_report
```

Builds with all GCC optimization levels (`-O0`, `-Og`, `-O1`, `-Os`, `-O2`,
`-O3`, `-Ofast`) and prints a grouped summary.

### Sample Output

```
constant vs immutable in C99 -- gcc 13

Experiment 1: constant expression values (C99 6.6)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     define=256 enum=100 sizeof=4 const_var=10

Experiment 2: modify const local via cast (UB: C99 6.7.3.5)
  -O0                                      local_name=999 local_ptr=999
  -Og, -O1, -Os, -O2, -O3, -Ofast          local_name=100 local_ptr=999

Experiment 3: memory permissions (C99 6.7.3, footnote 114)
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     g_const      perms=r--p writable=no
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     g_mutable    perms=rw-p writable=yes
  -O0, -Og, -O1, -Os, -O2, -O3, -Ofast     local_const  perms=rw-p writable=yes
```

### Findings

- **Experiment 1**: `#define`, `enum`, `sizeof` are constant expressions;
  `const int n` is not — `arr[n]` becomes a VLA, `case n:` is illegal.
  Identical across all optimization levels.
- **Experiment 2**: Modifying a `const` local via pointer cast is undefined
  behavior (C99 §6.7.3.5). Under `-O0` the compiler reads from memory and
  sees 999; from `-Og` onward, constant propagation replaces the read with
  the original value 100.
- **Experiment 3**: Global `const` is placed in a read-only page (`r--p`),
  while global mutable and stack variables are in read-write pages (`rw-p`).
  This confirms C99 §6.7.3 footnote 114. Identical across all optimization
  levels — memory layout is determined by the linker, not the optimizer.
