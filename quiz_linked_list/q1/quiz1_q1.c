/*
 * quiz1_q1.c - 2018q1 quiz4 Q1: original doubly-linked circular list.
 *
 * Original quiz: https://hackmd.io/@sysprog/linked-list-quiz
 * Reference: https://github.com/LinYunWen/c-review/blob/master/week4/bubble_sort.c
 *
 * Known issues:
 *   1. FuncB: dereferences NULL on empty list (no NULL guard)
 *   2. FuncC: dereferences NULL on empty list (no NULL guard)
 *   3. FuncC: infinite loop when value2 not found (circular list)
 *   4. exchange(): doesn't update neighbor pointers + overwrites without temp
 *   5. bubble_sort(): left/right not reset at start of each outer iteration
 */

#include "quiz1_q1.h"

/* FuncB: insert at head — BUG: no NULL guard */
void FuncB(struct node **start, int value)
{
    struct node *last = (*start)->prev;  /* crashes if *start == NULL */
    struct node *new_node = malloc(sizeof(struct node));
    new_node->data = value;
    new_node->next = *start;
    new_node->prev = last;
    last->next = (*start)->prev = new_node;
    *start = new_node;
}

/* FuncC: insert value1 after node with value2
 * BUG 1: no NULL guard
 * BUG 2: infinite loop if value2 not found (circular list never hits NULL)
 */
void FuncC(struct node **start, int value1, int value2)
{
    struct node *new_node = malloc(sizeof(struct node));
    new_node->data = value1;
    struct node *temp = *start;
    while (temp->data != value2)
        temp = temp->next;
    struct node *next = temp->next;
    temp->next = new_node;
    new_node->prev = temp;
    new_node->next = next;
    next->prev = new_node;
}

/* exchange: swap two adjacent nodes — BUGGY
 * BUG 1: doesn't update neighbor pointers (prev->next, next->prev)
 * BUG 2: overwrites (*left)->next before saving it
 */
void exchange(struct node **left, struct node **right)
{
    (*left)->next = (*right)->next;
    (*right)->prev = (*left)->prev;
    (*left)->prev = *right;
    (*right)->next = *left;
}

/* bubble_sort — BUGGY: left/right not reset per outer iteration */
void bubble_sort(struct node **start, int length)
{
    struct node *left = *start;
    struct node *right = left->next;

    for (int i = 0; i < length - 1; i++) {
        for (int j = i + 1; j < length; j++) {
            if (right->data < left->data)
                exchange(&left, &right);
            left = left->next;
            right = right->next;
        }
    }
}
