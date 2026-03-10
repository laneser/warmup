/*
 * qsort_impl.c - Four quicksort variants on singly-linked list.
 *
 * 1. qsort_original  - two recursive calls (2021q1 quiz1)
 * 2. qsort_tco       - linD026's "TCO" (skips recursion on empty partitions)
 * 3. qsort_method2   - recurse on smaller half, loop on larger (O(log n) stack)
 * 4. qsort_iterative - explicit stack, no recursion (alienryderflex style)
 */

#include "qsort.h"

/* ---- Version 1: original (from 2021q1 quiz1) ---- */

void qsort_original(node_t **list)
{
    if (!*list)
        return;

    node_t *pivot = *list;
    int value = pivot->value;
    node_t *p = pivot->next;
    pivot->next = NULL;

    node_t *left = NULL, *right = NULL;
    while (p) {
        node_t *n = p;
        p = p->next;
        list_add(n->value > value ? &right : &left, n);
    }

    qsort_original(&left);
    qsort_original(&right);

    node_t *result = NULL;
    list_concat(&result, left);
    list_concat(&result, pivot);
    list_concat(&result, right);
    *list = result;
}

/* ---- Version 2: linD026's quicksort_tco ---- */

/*
 * linD026's approach: skip recursion when a partition is empty.
 * Despite the name "tco", this is NOT tail call optimization —
 * it still makes two recursive calls in non-tail position.
 * The only difference from original is early termination on
 * empty partitions.
 */
void qsort_tco(node_t **list)
{
    if (!*list)
        return;

    node_t *pivot = *list;
    int value = pivot->value;
    node_t *p = pivot->next;
    pivot->next = NULL;

    node_t *left = NULL, *right = NULL;
    while (p) {
        node_t *n = p;
        p = p->next;
        list_add(n->value > value ? &right : &left, n);
    }

    node_t *result = NULL;
    if (!left && right) {
        list_concat(&result, pivot);
        qsort_tco(&right);
        list_concat(&result, right);
    } else if (left && right) {
        qsort_tco(&left);
        list_concat(&result, left);
        list_concat(&result, pivot);
        qsort_tco(&right);
        list_concat(&result, right);
    } else if (left && !right) {
        qsort_tco(&left);
        list_concat(&result, left);
        list_concat(&result, pivot);
    } else {
        list_concat(&result, pivot);
    }
    *list = result;
}

/* ---- Version 3: recurse smaller, loop larger (O(log n) stack) ---- */

/*
 * After partitioning, compare |left| and |right|.  Always recurse
 * on the SMALLER half (guaranteed <= n/2) and loop on the LARGER.
 *
 * When the larger half is right, use the tail-pointer trick.
 * When the larger half is left, accumulate a "suffix" (pivot +
 * sorted_right) and continue the loop on left; attach after loop.
 *
 * Guarantee: recursion depth <= log2(n).
 */
void qsort_method2(node_t **list)
{
    node_t *suffix = NULL;

    while (*list) {
        node_t *pivot = *list;
        int value = pivot->value;
        node_t *p = pivot->next;
        pivot->next = NULL;

        node_t *left = NULL, *right = NULL;
        int lc = 0, rc = 0;
        while (p) {
            node_t *n = p;
            p = p->next;
            if (n->value > value) {
                list_add(&right, n);
                rc++;
            } else {
                list_add(&left, n);
                lc++;
            }
        }

        if (lc <= rc) {
            /* Left is smaller: recurse left, loop right */
            qsort_method2(&left);

            node_t *result = NULL;
            list_concat(&result, left);
            list_concat(&result, pivot);
            node_t **tail = &result;
            while (*tail)
                tail = &(*tail)->next;
            *tail = right;

            *list = result;
            list = tail;
        } else {
            /* Right is smaller: recurse right, loop left */
            qsort_method2(&right);

            /* Defer: pivot -> sorted_right -> old suffix */
            pivot->next = right;
            list_concat(&pivot, suffix);
            suffix = pivot;

            /* Continue loop to sort left */
            *list = left;
        }
    }

    /* Attach accumulated suffix */
    *list = suffix;
}

/* ---- Version 4: iterative with explicit stack (no recursion) ---- */

/*
 * Inspired by alienryderflex.com/quicksort/.  Uses a stack of
 * sublist pointers instead of recursion.  After partitioning,
 * push right, pivot, left (bottom-to-top) so pop order is
 * left -> pivot -> right, producing correct sorted order.
 *
 * Single-node entries are appended directly to the result.
 */
#define ITERATIVE_STACK_SIZE 256

void qsort_iterative(node_t **list)
{
    if (!*list)
        return;

    node_t *stack[ITERATIVE_STACK_SIZE];
    int top = 0;
    stack[top++] = *list;

    node_t *result = NULL;
    node_t **rtail = &result;

    while (top > 0) {
        node_t *sub = stack[--top];

        /* Single node: append to sorted result */
        if (!sub->next) {
            sub->next = NULL;
            *rtail = sub;
            rtail = &sub->next;
            continue;
        }

        /* Partition */
        node_t *pivot = sub;
        int value = pivot->value;
        node_t *p = pivot->next;
        pivot->next = NULL;

        node_t *left = NULL, *right = NULL;
        while (p) {
            node_t *n = p;
            p = p->next;
            list_add(n->value > value ? &right : &left, n);
        }

        /* Push right, pivot, left (bottom to top).
         * Pop order: left, pivot, right = correct sorted order. */
        if (right)
            stack[top++] = right;
        stack[top++] = pivot;
        if (left)
            stack[top++] = left;
    }

    *list = result;
}
