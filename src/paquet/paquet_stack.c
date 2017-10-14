//
// Created by Alexandre Dewit on 14/10/17.
//

#include <stdlib.h>
#include "packet_stack.h"

void addElem(stack *stack, pkt_t *p) {
    node_t *elem = malloc(sizeof(*elem));
    if (stack == NULL || elem == NULL) {
        exit(EXIT_FAILURE);
    }

    elem->p = p;
    elem->next = stack->head;
    stack->head = elem;
}

pkt_t *removeElem(stack *stack) {
    if (stack == NULL) {
        exit(EXIT_FAILURE);
    }

    pkt_t *p = NULL;

    if (isEmpty(stack) == 0) {
        node_t *first = stack->head;
        p = first->p;
        stack->head = first->next;
        free(first);
    }

    return p;
}

int isEmpty(stack *stack) {
    if (stack == NULL || stack->head == NULL) return 1;
    return 0;
}

pkt_t *first(stack *stack) {
    if (stack == NULL || stack->head == NULL) return NULL;
    return stack->head->p;
}

int isPresent(stack *stack, pkt_t *p) {
    if (stack->head == NULL) return 0;
    node_t *node = stack->head;
    while (node != NULL) {
        if (node->p == p) {
            return 1;
        }

        node = node->next;
    }
    return 0;
}