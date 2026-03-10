/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LIST_SORT_H
#define LIST_SORT_H

#include "list.h"

typedef int (*list_cmp_func_t)(void *priv, const struct list_head *a,
			       const struct list_head *b);

/* Original kernel list_sort */
void list_sort_vanilla(void *priv, struct list_head *head,
		       list_cmp_func_t cmp);

/* list_sort with natural run detection */
void list_sort_runs(void *priv, struct list_head *head,
		    list_cmp_func_t cmp);

#endif /* LIST_SORT_H */
