/*
 * quiz1_q1.h - 2018q1 quiz4 Q1: doubly-linked circular list.
 *
 * Original quiz: https://hackmd.io/@sysprog/linked-list-quiz
 * Header-only so both buggy and fixed versions can be tested.
 */

#ifndef QUIZ1_Q1_H
#define QUIZ1_Q1_H

#include <stdlib.h>

struct node {
    int data;
    struct node *next, *prev;
};

/* ---- Helpers (shared by buggy and fixed) ---- */

static inline void free_list(struct node *start)
{
    if (!start)
        return;
    struct node *cur = start->next;
    while (cur != start) {
        struct node *tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    free(start);
}

static inline int list_to_array(struct node *start, int *arr, int max)
{
    if (!start)
        return 0;
    int i = 0;
    struct node *p = start;
    do {
        if (i >= max)
            break;
        arr[i++] = p->data;
        p = p->next;
    } while (p != start);
    return i;
}

static inline int verify_integrity(struct node *start)
{
    if (!start)
        return 1;
    struct node *p = start;
    do {
        if (p->next->prev != p)
            return 0;
        if (p->prev->next != p)
            return 0;
        p = p->next;
    } while (p != start);
    return 1;
}

/* FuncA: insert at tail — correct (has NULL guard) */
static inline void FuncA(struct node **start, int value)
{
    if (!*start) {
        struct node *new_node = malloc(sizeof(struct node));
        new_node->data = value;
        new_node->next = new_node->prev = new_node;
        *start = new_node;
        return;
    }
    struct node *last = (*start)->prev;
    struct node *new_node = malloc(sizeof(struct node));
    new_node->data = value;
    new_node->next = *start;
    (*start)->prev = new_node;
    new_node->prev = last;
    last->next = new_node;
}

static inline struct node *build_list(const int *vals, int n)
{
    struct node *head = NULL;
    for (int i = 0; i < n; i++)
        FuncA(&head, vals[i]);
    return head;
}

#endif /* QUIZ1_Q1_H */
