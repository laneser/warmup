/*
 * fix_quiz1_q1.c - Suggested version of 2018q1 quiz4 Q1.
 *
 * Improvements:
 *   1. FuncB: add NULL guard for empty list
 *   2. FuncC: add NULL guard + detect when value2 not found
 *   3. bubble_sort: swap data instead of relinking nodes
 *   4. bubble_sort: reset traversal at start of each outer iteration
 */

#include "quiz1_q1.h"

/* FuncB: insert at head — FIX: add NULL guard */
void FuncB(struct node **start, int value)
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
    new_node->prev = last;
    last->next = (*start)->prev = new_node;
    *start = new_node;
}

/* FuncC: insert value1 after node with value2 — FIX: NULL guard + loop detect */
void FuncC(struct node **start, int value1, int value2)
{
    if (!*start)
        return;

    struct node *temp = *start;
    do {
        if (temp->data == value2)
            break;
        temp = temp->next;
    } while (temp != *start);

    if (temp->data != value2)
        return;

    struct node *new_node = malloc(sizeof(struct node));
    new_node->data = value1;
    struct node *next = temp->next;
    temp->next = new_node;
    new_node->prev = temp;
    new_node->next = next;
    next->prev = new_node;
}

/* bubble_sort — FIX: swap data, reset per iteration */
void bubble_sort(struct node **start, int length)
{
    for (int i = 0; i < length - 1; i++) {
        struct node *cur = *start;
        for (int j = 0; j < length - 1 - i; j++) {
            if (cur->data > cur->next->data) {
                int tmp = cur->data;
                cur->data = cur->next->data;
                cur->next->data = tmp;
            }
            cur = cur->next;
        }
    }
}
