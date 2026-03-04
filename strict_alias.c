/* Demonstrate strict aliasing violation: writing through int* to a float
 * object, then reading through the original float lvalue.
 *
 * C99 §6.5¶7: accessing an object through an lvalue of incompatible type
 * is undefined behavior (strict aliasing rule).
 *
 * Compile: gcc -std=c99 -Wall -Wextra -O{0,2} -o strict_alias strict_alias.c
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* --- Test 1: strict aliasing violation ---
 * Two pointers of different types to the same object.  The compiler
 * assumes float* and int* cannot alias (per §6.5¶7), so after *ip = 0
 * it may keep the old value of *fp in a register. */
__attribute__((noinline))
static float alias_violation(float *fp, int *ip)
{
    *fp = 1.0f;
    *ip = 0;        /* UB: int* writing to a float object */
    return *fp;     /* compiler may return 1.0f from register */
}

/* --- Test 2: memcpy version (well-defined) ---
 * memcpy operates through character types, always legal. */
__attribute__((noinline))
static float memcpy_version(void)
{
    float g = 1.0f;
    int zero = 0;
    memcpy(&g, &zero, sizeof g);
    return g;
}

/* --- Test 3: unsigned char* (always legal per §6.5¶7) --- */
__attribute__((noinline))
static float uchar_version(void)
{
    float h = 1.0f;
    unsigned char *cp = (unsigned char *)&h;
    for (int i = 0; i < (int)sizeof h; i++)
        cp[i] = 0;
    return h;
}

/* --- Test 4: show IEEE 754 bit pattern --- */
static void show_bits(void)
{
    float f = 1.0f;
    uint32_t bits;
    memcpy(&bits, &f, sizeof bits);
    printf("[v4] 1.0f bits = 0x%08x (%u)\n", bits, bits);
}

int main(void)
{
    /* v1: pass same address as both float* and int* */
    float x;
    float result = alias_violation(&x, (int *)&x);
    printf("[v1] alias violation: %f\n", result);

    printf("[v2] memcpy:          %f\n", memcpy_version());
    printf("[v3] uchar*:          %f\n", uchar_version());
    show_bits();
    return 0;
}
