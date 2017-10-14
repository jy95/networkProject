//
// Created by Alexandre Dewit on 14/10/17.
//

#include <stdlib.h>
#include "packet_stack.h"

void addElem(struct stack *Stack, pkt_t *p) {
    struct node *elem = malloc(sizeof(*elem));
    if (Stack == NULL || elem == NULL) {
        exit(EXIT_FAILURE);
    }

    elem->p = p;
    elem->next = Stack->head;
    Stack->head = elem;
}

pkt_t *removeElem(struct stack *Stack) {
    if (Stack == NULL) {
        exit(EXIT_FAILURE);
    }

    pkt_t *p = NULL;

    if (isEmpty(Stack) == 0) {
        struct node *first = Stack->head;
        p = first->p;
        Stack->head = first->next;
        free(first);
    }

    return p;
}

int isEmpty(struct stack *Stack) {
    if (Stack == NULL || Stack->head == NULL) return 1;
    return 0;
}

pkt_t *first(struct stack *Stack) {
    if (Stack == NULL || Stack->head == NULL) return NULL;
    return Stack->head->p;
}

int isPresent(struct stack *Stack, pkt_t *p) {
    if (Stack->head == NULL) return 0;
    struct node *structnode = Stack->head;
    while (structnode != NULL) {
        if (structnode->p == p) {
            return 1;
        }

        structnode = structnode->next;
    }
    return 0;
}