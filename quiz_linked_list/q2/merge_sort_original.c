/*
 * merge_sort_original.c - Original quiz version (1-vs-(n-1) split).
 *
 * This is the exact code from the quiz with blanks filled in.
 * Split strategy: left gets 1 element, right gets the rest.
 * This leads to O(n) recursion depth and O(n^2) overall complexity.
 */

#include "merge_sort.h"

list *sort(list *start)
{
    if (!start || !start->next)
        return start;

    list *left = start;
    list *right = left->next;
    left->next = NULL; /* LL0 */

    left = sort(left);
    right = sort(right);

    for (list *merge = NULL; left || right;) {
        if (!right || (left && left->data < right->data)) {
            if (!merge) {
                start = merge = left; /* LL1 */
            } else {
                merge->next = left; /* LL2 */
                merge = merge->next;
            }
            left = left->next; /* LL3 */
        } else {
            if (!merge) {
                start = merge = right; /* LL4 */
            } else {
                merge->next = right; /* LL5 */
                merge = merge->next;
            }
            right = right->next; /* LL6 */
        }
    }
    return start;
}
