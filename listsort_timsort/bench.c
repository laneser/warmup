// SPDX-License-Identifier: GPL-2.0
/*
 * bench.c - Standalone userspace benchmark for list_sort variants.
 *
 * Compares vanilla list_sort (per-element) vs natural run detection
 * across three input patterns:
 *   1. full_random  — uniformly random values (baseline)
 *   2. random_asc   — random-length ascending runs spliced together
 *   3. random_desc  — random-length descending runs spliced together
 *
 * Total list length is held constant; run lengths are drawn uniformly
 * from [1, max_run_len].
 *
 * Output: CSV to stdout for post-processing with analyze_bench.py
 *
 * Usage: ./bench [total_elements] [iterations] [max_run_len]
 *   defaults: 10000 elements, 30 iterations, 64 max run length
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "list_sort.h"

/* ---- element type ---- */

struct element {
	int value;
	struct list_head list;
};

/* ---- comparison with counter ---- */

struct bench_ctx {
	unsigned long comparisons;
};

static int cmp_count(void *priv, const struct list_head *a,
		     const struct list_head *b)
{
	struct bench_ctx *ctx = priv;
	struct element *ea = container_of(a, struct element, list);
	struct element *eb = container_of(b, struct element, list);

	ctx->comparisons++;
	return (ea->value > eb->value) - (ea->value < eb->value);
}

/* ---- PRNG: xoshiro128** for reproducible, fast randomness ---- */

static uint32_t rng_state[4];

static uint32_t rotl(uint32_t x, int k)
{
	return (x << k) | (x >> (32 - k));
}

static uint32_t xoshiro128ss(void)
{
	uint32_t result = rotl(rng_state[1] * 5, 7) * 9;
	uint32_t t = rng_state[1] << 9;

	rng_state[2] ^= rng_state[0];
	rng_state[3] ^= rng_state[1];
	rng_state[1] ^= rng_state[2];
	rng_state[0] ^= rng_state[3];
	rng_state[2] ^= t;
	rng_state[3] = rotl(rng_state[3], 11);
	return result;
}

static void rng_seed(uint32_t seed)
{
	/* SplitMix32 to initialize state */
	for (int i = 0; i < 4; i++) {
		seed += 0x9e3779b9;
		uint32_t z = seed;

		z = (z ^ (z >> 16)) * 0x85ebca6b;
		z = (z ^ (z >> 13)) * 0xc2b2ae35;
		z = z ^ (z >> 16);
		rng_state[i] = z;
	}
}

/* Random integer in [0, bound) */
static uint32_t rng_bounded(uint32_t bound)
{
	return xoshiro128ss() % bound;
}

/* ---- pattern generators ---- */

static void gen_full_random(struct element *elems, int n)
{
	for (int i = 0; i < n; i++)
		elems[i].value = (int)xoshiro128ss();
}

/*
 * Generate random-length ascending runs.
 * Run lengths drawn from [1, max_run_len].
 * Values within each run are strictly increasing.
 * Runs are concatenated; boundary values are NOT sorted across runs.
 */
static void gen_random_asc_runs(struct element *elems, int n, int max_run_len)
{
	int i = 0;

	while (i < n) {
		int rlen = 1 + (int)rng_bounded(max_run_len);

		if (i + rlen > n)
			rlen = n - i;
		/* Start value for this run: random base */
		int base = (int)(xoshiro128ss() >> 1); /* positive */

		for (int j = 0; j < rlen; j++)
			elems[i + j].value = base + j;
		i += rlen;
	}
}

/*
 * Generate random-length descending runs.
 * Same as above but each run is strictly decreasing.
 */
static void gen_random_desc_runs(struct element *elems, int n, int max_run_len)
{
	int i = 0;

	while (i < n) {
		int rlen = 1 + (int)rng_bounded(max_run_len);

		if (i + rlen > n)
			rlen = n - i;
		int base = (int)(xoshiro128ss() >> 1);

		for (int j = 0; j < rlen; j++)
			elems[i + j].value = base + (rlen - 1 - j);
		i += rlen;
	}
}

/*
 * Generate mixed ascending and descending runs.
 * Each run is randomly ascending or descending with random length.
 */
static void gen_random_mixed_runs(struct element *elems, int n, int max_run_len)
{
	int i = 0;

	while (i < n) {
		int rlen = 1 + (int)rng_bounded(max_run_len);

		if (i + rlen > n)
			rlen = n - i;
		int base = (int)(xoshiro128ss() >> 1);
		int ascending = xoshiro128ss() & 1;

		for (int j = 0; j < rlen; j++) {
			if (ascending)
				elems[i + j].value = base + j;
			else
				elems[i + j].value = base + (rlen - 1 - j);
		}
		i += rlen;
	}
}

/* ---- list building ---- */

static void build_list(struct list_head *head, struct element *elems, int n)
{
	INIT_LIST_HEAD(head);
	for (int i = 0; i < n; i++)
		list_add_tail(&elems[i].list, head);
}

/* ---- verification ---- */

static int verify_sorted(struct list_head *head)
{
	struct list_head *pos;
	int prev_val = 0;
	int first = 1;

	list_for_each(pos, head) {
		struct element *e = container_of(pos, struct element, list);

		if (!first && e->value < prev_val)
			return 0;
		prev_val = e->value;
		first = 0;
	}
	return 1;
}

/* ---- timing ---- */

static inline long long timespec_ns(struct timespec *ts)
{
	return (long long)ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

/* ---- benchmark driver ---- */

typedef void (*sort_func_t)(void *priv, struct list_head *head,
			    list_cmp_func_t cmp);

struct sort_variant {
	const char *name;
	sort_func_t sort;
};

static const struct sort_variant variants[] = {
	{ "vanilla", list_sort_vanilla },
	{ "runs",    list_sort_runs },
};

struct pattern_desc {
	const char *name;
	int needs_max_run_len; /* 0 = no, pattern ignores max_run_len */
};

static const struct pattern_desc patterns[] = {
	{ "full_random",       0 },
	{ "random_asc_runs",   1 },
	{ "random_desc_runs",  1 },
	{ "random_mixed_runs", 1 },
};

static void generate_pattern(const char *name, struct element *elems,
			     int n, int max_run_len)
{
	if (strcmp(name, "full_random") == 0)
		gen_full_random(elems, n);
	else if (strcmp(name, "random_asc_runs") == 0)
		gen_random_asc_runs(elems, n, max_run_len);
	else if (strcmp(name, "random_desc_runs") == 0)
		gen_random_desc_runs(elems, n, max_run_len);
	else if (strcmp(name, "random_mixed_runs") == 0)
		gen_random_mixed_runs(elems, n, max_run_len);
}

static const int default_sizes[] = { 1000, 5000, 10000, 50000, 100000 };
#define N_SIZES ((int)(sizeof(default_sizes) / sizeof(default_sizes[0])))

int main(int argc, char *argv[])
{
	int iterations = 30;
	int max_run_len = 64;

	if (argc > 1)
		iterations = atoi(argv[1]);
	if (argc > 2)
		max_run_len = atoi(argv[2]);

	fprintf(stderr, "iterations=%d max_run_len=%d\n",
		iterations, max_run_len);

	/* CSV header */
	printf("variant,pattern,n,max_run_len,iter,comparisons,time_ns\n");

	for (int si = 0; si < N_SIZES; si++) {
		int n = default_sizes[si];
		struct element *elems = malloc(n * sizeof(*elems));

		if (!elems) {
			fprintf(stderr, "malloc failed for n=%d\n", n);
			return 1;
		}

		for (int pi = 0; pi < (int)(sizeof(patterns) / sizeof(patterns[0])); pi++) {
			for (int vi = 0; vi < (int)(sizeof(variants) / sizeof(variants[0])); vi++) {
				for (int iter = 0; iter < iterations; iter++) {
					struct list_head head;
					struct bench_ctx ctx = { .comparisons = 0 };
					struct timespec t0, t1;

					/*
					 * Use a deterministic seed per
					 * (pattern, size, iter) so both
					 * variants sort identical input.
					 */
					uint32_t seed = (uint32_t)(pi * 1000000 +
								   n * 100 + iter);
					rng_seed(seed);

					generate_pattern(patterns[pi].name,
							 elems, n, max_run_len);
					build_list(&head, elems, n);

					clock_gettime(CLOCK_MONOTONIC, &t0);
					variants[vi].sort(&ctx, &head,
							  cmp_count);
					clock_gettime(CLOCK_MONOTONIC, &t1);

					long long elapsed =
						timespec_ns(&t1) -
						timespec_ns(&t0);

					if (!verify_sorted(&head)) {
						fprintf(stderr,
							"FAIL: %s/%s n=%d iter=%d not sorted!\n",
							variants[vi].name,
							patterns[pi].name,
							n, iter);
						free(elems);
						return 1;
					}

					printf("%s,%s,%d,%d,%d,%lu,%lld\n",
					       variants[vi].name,
					       patterns[pi].name,
					       n, max_run_len, iter,
					       ctx.comparisons, elapsed);
				}
			}
		}
		free(elems);
		fprintf(stderr, "  n=%d done\n", n);
	}

	return 0;
}
