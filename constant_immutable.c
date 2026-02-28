/*
 * constant_immutable.c - Verify that C99 'const' means immutable, not constant.
 *
 * Runtime experiments:
 *   1. Casting away const is UB -- observable via optimization (C99 6.7.3.5)
 *   2. Global const placed in read-only memory (C99 6.7.3, footnote 114)
 *
 * Compile-time tests (enabled via -D flags, see Makefile):
 *   - Case label tests: which constructs qualify as constant expressions (C99 6.6)
 *   - Mutability/addressability: enum vs const int vs #define
 */

#include <stdio.h>

#define BUFSIZE 256

const int g_const = 42;
int g_mutable = 42;

static void check_writable(const char *label, const void *addr)
{
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps)
        return;

    unsigned long target = (unsigned long)addr;
    unsigned long start, end;
    char perms[8], line[256];

    while (fgets(line, sizeof(line), maps)) {
        if (sscanf(line, "%lx-%lx %7s", &start, &end, perms) == 3 &&
            target >= start && target < end) {
            printf("[exp2:%s] %-12s perms=%s writable=%s\n",
                   label, label, perms, perms[1] == 'w' ? "yes" : "no");
            break;
        }
    }
    fclose(maps);
}

int main(void)
{
    /* Experiment 1: casting away const is UB (C99 6.7.3.5). */
    const int local = 100;
    int *p = (int *)&local;
    *p = 999;
    printf("[exp1] local_name=%d local_ptr=%d\n", local, *p);

    /* Experiment 2: memory region permissions. */
    check_writable("g_const", &g_const);
    check_writable("g_mutable", &g_mutable);
    check_writable("local_const", &local);

    return 0;
}

/*
 * Compile-time tests: enabled via -D flags from Makefile.
 * Each block tests whether a specific construct compiles or not.
 */

/* --- Case label tests (C99 6.6, 6.8.4.2) --- */

#ifdef TEST_CASE_DEFINE
/* Expected: compiles -- #define expands to integer literal, a constant expr. */
#define CASE_V 10
void test_case_define(void) { switch (0) { case CASE_V: break; } }
#endif

#ifdef TEST_CASE_ENUM
/* Expected: compiles -- enumeration constant is a constant expression. */
enum { CASE_E = 10 };
void test_case_enum(void) { switch (0) { case CASE_E: break; } }
#endif

#ifdef TEST_CASE_SIZEOF
/* Expected: compiles -- sizeof yields an integer constant expression. */
void test_case_sizeof(void) { switch (0) { case sizeof(int): break; } }
#endif

#ifdef TEST_CASE_CAST_FLOAT
/* Expected: compiles -- float cast to int is a constant expression. */
void test_case_cast_float(void) { switch (0) { case (int)3.14: break; } }
#endif

#ifdef TEST_CASE_CHAR
/* Expected: compiles -- character constant is an integer constant. */
void test_case_char(void) { switch (0) { case 'A': break; } }
#endif

#ifdef TEST_CASE_CONST_INT
/* Expected: compile error in strict C99; GCC accepts at -Og+ (extension). */
void test_case_const_int(void)
{
    const int N = 10;
    switch (0) { case N: break; }
}
#endif

#ifdef TEST_VLA_CONST
/* Expected: -Wvla triggers -- const int is not a constant expression,
 * so int a[n] is a VLA even when n is const-qualified. */
void test_vla_const(void)
{
    const int n = 10;
    int a[n];
    (void)a;
}
#endif

/* --- Mutability and addressability tests --- */

#ifdef TEST_ENUM_ASSIGN
/* Expected: compile error -- enum constant is an rvalue, not assignable. */
enum { TVAL = 10 };
void test_enum_assign(void) { TVAL = 20; }
#endif

#ifdef TEST_ENUM_ADDR
/* Expected: compile error -- enum constant has no memory location. */
enum { TVAL2 = 10 };
void test_enum_addr(void) { int *p = (int *)&TVAL2; (void)p; }
#endif

#ifdef TEST_CONST_ADDR
/* Expected: compiles -- const int occupies memory and is addressable. */
void test_const_addr(void) { const int N = 10; const int *p = &N; (void)p; }
#endif

#ifdef TEST_DEFINE_REDEFINE
/* Expected: compiles -- macro can be redefined via #undef. */
#undef BUFSIZE
#define BUFSIZE 999
void test_define_redefine(void) { printf("[redefine] %d\n", BUFSIZE); }
#endif
