#!/usr/bin/env bash
# apply_instrumentation.sh — Apply list_sort run-counting instrumentation
#
# Usage:
#   ./apply_instrumentation.sh <kernel-source-dir>
#
# This directly modifies the kernel source tree:
#   1. Adds lib/list_sort_stats.{c,h}
#   2. Patches lib/list_sort.c to call the recorder
#   3. Adds CONFIG_LIST_SORT_STATS to lib/Kconfig.debug
#   4. Adds obj-$(CONFIG_LIST_SORT_STATS) to lib/Makefile
#
# After applying, enable with:
#   echo CONFIG_LIST_SORT_STATS=y >> .config
#   make olddefconfig

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ $# -lt 1 ]; then
    echo "Usage: $0 <kernel-source-dir>"
    exit 1
fi

KSRC="$1"

if [ ! -f "${KSRC}/lib/list_sort.c" ]; then
    echo "ERROR: ${KSRC}/lib/list_sort.c not found"
    exit 1
fi

echo "=== Applying list_sort instrumentation to ${KSRC} ==="

# 1. Create list_sort_stats.h
cat > "${KSRC}/lib/list_sort_stats.h" << 'HEADER_EOF'
/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LIB_LIST_SORT_STATS_H
#define _LIB_LIST_SORT_STATS_H

#ifdef CONFIG_LIST_SORT_STATS

void list_sort_stats_record(unsigned long n_elements, unsigned long n_runs,
			    unsigned long n_asc, unsigned long n_desc);

#else

static inline void list_sort_stats_record(unsigned long n_elements,
					  unsigned long n_runs,
					  unsigned long n_asc,
					  unsigned long n_desc)
{
}

#endif /* CONFIG_LIST_SORT_STATS */
#endif /* _LIB_LIST_SORT_STATS_H */
HEADER_EOF
echo "  Created lib/list_sort_stats.h"

# 2. Create list_sort_stats.c (histogram version)
cat > "${KSRC}/lib/list_sort_stats.c" << 'STATS_EOF'
// SPDX-License-Identifier: GPL-2.0
/*
 * list_sort_stats.c - debugfs instrumentation for list_sort() input patterns
 *
 * Exposes /sys/kernel/debug/list_sort_stats/{stats,reset}
 *
 * Records a histogram of list sizes with per-bucket run statistics,
 * plus per-caller breakdown using __builtin_return_address(1) to
 * identify the actual caller of list_sort().
 */

#include <linux/debugfs.h>
#include <linux/atomic.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>
#include "list_sort_stats.h"

/* Histogram bucket boundaries: [0,10), [10,100), [100,1000), ... */
#define NR_BUCKETS 7
static const unsigned long bucket_bounds[NR_BUCKETS] = {
	0, 10, 100, 1000, 5000, 10000, ULONG_MAX
};

struct bucket_stats {
	atomic64_t calls;
	atomic64_t elements;
	atomic64_t runs;
	atomic64_t already_sorted;
};

static struct bucket_stats buckets[NR_BUCKETS];

/* Global aggregates */
static atomic64_t stat_calls = ATOMIC64_INIT(0);
static atomic64_t stat_elements = ATOMIC64_INIT(0);
static atomic64_t stat_runs = ATOMIC64_INIT(0);
static atomic64_t stat_asc_runs = ATOMIC64_INIT(0);
static atomic64_t stat_desc_runs = ATOMIC64_INIT(0);
static atomic64_t stat_already_sorted = ATOMIC64_INIT(0);
static atomic64_t stat_max_elements = ATOMIC64_INIT(0);

/* Per-caller tracking */
#define MAX_CALLERS 32

struct caller_entry {
	unsigned long addr;
	unsigned long count;
	unsigned long total_elements;
	unsigned long total_runs;
	unsigned long max_elements;
};

static struct caller_entry callers[MAX_CALLERS];
static int n_callers;
static DEFINE_SPINLOCK(callers_lock);

static int bucket_idx(unsigned long n)
{
	int i;

	for (i = NR_BUCKETS - 1; i > 0; i--) {
		if (n >= bucket_bounds[i])
			return i;
	}
	return 0;
}

void list_sort_stats_record(unsigned long n_elements, unsigned long n_runs,
			    unsigned long n_asc, unsigned long n_desc)
{
	/*
	 * __builtin_return_address(1) gives us list_sort()'s caller
	 * (e.g. an XFS function), since this function is called from
	 * within list_sort().  Level 0 would just give list_sort itself.
	 */
	unsigned long ret_addr = (unsigned long)__builtin_return_address(1);
	int idx = bucket_idx(n_elements);
	s64 cur_max;
	int i;

	/* Global stats */
	atomic64_inc(&stat_calls);
	atomic64_add(n_elements, &stat_elements);
	atomic64_add(n_runs, &stat_runs);
	atomic64_add(n_asc, &stat_asc_runs);
	atomic64_add(n_desc, &stat_desc_runs);
	if (n_runs <= 1)
		atomic64_inc(&stat_already_sorted);

	/* Track max elements seen */
	cur_max = atomic64_read(&stat_max_elements);
	if ((s64)n_elements > cur_max)
		atomic64_cmpxchg(&stat_max_elements, cur_max, n_elements);

	/* Histogram bucket */
	atomic64_inc(&buckets[idx].calls);
	atomic64_add(n_elements, &buckets[idx].elements);
	atomic64_add(n_runs, &buckets[idx].runs);
	if (n_runs <= 1)
		atomic64_inc(&buckets[idx].already_sorted);

	/* Per-caller */
	spin_lock(&callers_lock);
	for (i = 0; i < n_callers; i++) {
		if (callers[i].addr == ret_addr) {
			callers[i].count++;
			callers[i].total_elements += n_elements;
			callers[i].total_runs += n_runs;
			if (n_elements > callers[i].max_elements)
				callers[i].max_elements = n_elements;
			spin_unlock(&callers_lock);
			return;
		}
	}
	if (n_callers < MAX_CALLERS) {
		callers[n_callers].addr = ret_addr;
		callers[n_callers].count = 1;
		callers[n_callers].total_elements = n_elements;
		callers[n_callers].total_runs = n_runs;
		callers[n_callers].max_elements = n_elements;
		n_callers++;
	}
	spin_unlock(&callers_lock);
}

static int stats_show(struct seq_file *m, void *v)
{
	s64 calls = atomic64_read(&stat_calls);
	s64 elements = atomic64_read(&stat_elements);
	s64 runs = atomic64_read(&stat_runs);
	int i;

	seq_printf(m, "calls:           %lld\n", calls);
	seq_printf(m, "total_elements:  %lld\n", elements);
	seq_printf(m, "total_runs:      %lld\n", runs);
	seq_printf(m, "total_asc_runs:  %lld\n", atomic64_read(&stat_asc_runs));
	seq_printf(m, "total_desc_runs: %lld\n", atomic64_read(&stat_desc_runs));
	seq_printf(m, "already_sorted:  %lld\n",
		   atomic64_read(&stat_already_sorted));
	seq_printf(m, "max_elements:    %lld\n",
		   atomic64_read(&stat_max_elements));
	if (calls > 0) {
		seq_printf(m, "avg_elements:    %lld\n", elements / calls);
		seq_printf(m, "avg_runs:        %lld\n", runs / calls);
	}

	/* Histogram */
	seq_puts(m, "\n--- histogram by list size ---\n");
	seq_printf(m, "%-16s %10s %12s %10s %8s %12s\n",
		   "range", "calls", "tot_elem", "tot_runs",
		   "sorted%", "avg_elem");

	for (i = 0; i < NR_BUCKETS; i++) {
		s64 bc = atomic64_read(&buckets[i].calls);
		s64 be = atomic64_read(&buckets[i].elements);
		s64 br = atomic64_read(&buckets[i].runs);
		s64 bs = atomic64_read(&buckets[i].already_sorted);
		char range[32];

		if (bc == 0)
			continue;

		if (i == NR_BUCKETS - 1)
			snprintf(range, sizeof(range), "[%lu,+inf)",
				 bucket_bounds[i]);
		else
			snprintf(range, sizeof(range), "[%lu,%lu)",
				 bucket_bounds[i], bucket_bounds[i + 1]);

		seq_printf(m, "%-16s %10lld %12lld %10lld %7lld%% %12lld\n",
			   range, bc, be, br,
			   bc > 0 ? bs * 100 / bc : 0,
			   bc > 0 ? be / bc : 0);
	}

	/* Per-caller */
	seq_puts(m, "\n--- per-caller breakdown ---\n");
	seq_printf(m, "%-50s %8s %10s %10s %8s\n",
		   "caller", "calls", "avg_elem", "avg_runs", "max_n");

	spin_lock(&callers_lock);
	for (i = 0; i < n_callers; i++) {
		char sym[KSYM_SYMBOL_LEN];
		unsigned long avg_elem = 0, avg_runs = 0;

		sprint_symbol(sym, callers[i].addr);
		if (callers[i].count > 0) {
			avg_elem = callers[i].total_elements / callers[i].count;
			avg_runs = callers[i].total_runs / callers[i].count;
		}
		seq_printf(m, "%-50s %8lu %10lu %10lu %8lu\n",
			   sym, callers[i].count, avg_elem, avg_runs,
			   callers[i].max_elements);
	}
	spin_unlock(&callers_lock);

	return 0;
}

static int stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, stats_show, NULL);
}

static ssize_t reset_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	int i;

	atomic64_set(&stat_calls, 0);
	atomic64_set(&stat_elements, 0);
	atomic64_set(&stat_runs, 0);
	atomic64_set(&stat_asc_runs, 0);
	atomic64_set(&stat_desc_runs, 0);
	atomic64_set(&stat_already_sorted, 0);
	atomic64_set(&stat_max_elements, 0);

	for (i = 0; i < NR_BUCKETS; i++) {
		atomic64_set(&buckets[i].calls, 0);
		atomic64_set(&buckets[i].elements, 0);
		atomic64_set(&buckets[i].runs, 0);
		atomic64_set(&buckets[i].already_sorted, 0);
	}

	spin_lock(&callers_lock);
	n_callers = 0;
	spin_unlock(&callers_lock);

	return count;
}

static const struct file_operations stats_fops = {
	.open = stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations reset_fops = {
	.write = reset_write,
};

static int __init list_sort_stats_init(void)
{
	struct dentry *dir;

	dir = debugfs_create_dir("list_sort_stats", NULL);
	debugfs_create_file("stats", 0444, dir, NULL, &stats_fops);
	debugfs_create_file("reset", 0200, dir, NULL, &reset_fops);
	pr_info("list_sort_stats: instrumentation enabled\n");
	return 0;
}
late_initcall(list_sort_stats_init);
STATS_EOF
echo "  Created lib/list_sort_stats.c"

# 3. Patch lib/list_sort.c — add include and pre-scan
SORT_FILE="${KSRC}/lib/list_sort.c"

# Add include after existing includes
if ! grep -q "list_sort_stats.h" "${SORT_FILE}"; then
    sed -i '/#include <linux\/list.h>/a\
#include "list_sort_stats.h"' "${SORT_FILE}"
    echo "  Added #include to list_sort.c"
fi

# Add pre-scan block before "head->prev->next = NULL"
if ! grep -q "list_sort_stats_record" "${SORT_FILE}"; then
    sed -i '/head->prev->next = NULL;/i\
\t/* Instrumentation: count natural runs before sorting */\
\t{\
\t\tstruct list_head *_scan = list;\
\t\tsize_t _n_elem = 1, _n_runs = 1;\
\t\tsize_t _n_asc = 0, _n_desc = 0;\
\t\tint _in_desc = 0;\
\t\twhile (_scan->next != head) {\
\t\t\tint _c = cmp(priv, _scan, _scan->next);\
\t\t\t_n_elem++;\
\t\t\tif (_in_desc) {\
\t\t\t\tif (_c <= 0) { _n_runs++; _in_desc = 0; }\
\t\t\t} else {\
\t\t\t\tif (_c > 0) { _n_runs++; _in_desc = 1; }\
\t\t\t}\
\t\t\t_scan = _scan->next;\
\t\t}\
\t\t_n_asc = (_n_runs + 1) / 2;\
\t\t_n_desc = _n_runs / 2;\
\t\tlist_sort_stats_record(_n_elem, _n_runs, _n_asc, _n_desc);\
\t}' "${SORT_FILE}"
    echo "  Added run-counting pre-scan to list_sort.c"
fi

# 4. Add Kconfig option
KCONFIG="${KSRC}/lib/Kconfig.debug"
if ! grep -q "LIST_SORT_STATS" "${KCONFIG}"; then
    # Insert after 'menu "Kernel hacking"'
    sed -i '/^menu "Kernel hacking"/a\
\
config LIST_SORT_STATS\
\tbool "Collect natural-run statistics for list_sort()"\
\tdepends on DEBUG_FS\
\tdefault n\
\thelp\
\t  When enabled, list_sort() scans its input before sorting to\
\t  count the number of natural ascending and descending runs.\
\t  Results are exposed via /sys/kernel/debug/list_sort_stats.\
\t  This adds a small overhead per list_sort() call.  Say N\
\t  unless you are researching list_sort() input patterns.' "${KCONFIG}"
    echo "  Added CONFIG_LIST_SORT_STATS to Kconfig.debug"
fi

# 5. Add to lib/Makefile
# Insert after the multi-line obj-y block that contains list_sort.o,
# NOT directly after the list_sort.o line (which is mid-continuation).
LIBMK="${KSRC}/lib/Makefile"
if ! grep -q "list_sort_stats" "${LIBMK}"; then
    sed -i '/^obj-y += string_helpers\.o/i\
obj-$(CONFIG_LIST_SORT_STATS) += list_sort_stats.o' "${LIBMK}"
    echo "  Added list_sort_stats.o to lib/Makefile"
fi

echo ""
echo "=== Done. Next steps: ==="
echo "  cd ${KSRC}"
echo "  echo 'CONFIG_LIST_SORT_STATS=y' >> .config"
echo "  make olddefconfig"
echo "  make -j\$(nproc)"
echo ""
echo "After boot, check:"
echo "  cat /sys/kernel/debug/list_sort_stats/stats"
echo "  echo 1 > /sys/kernel/debug/list_sort_stats/reset"
