//
// Created by Alexandre Dewit on 14/10/17.
//

#ifndef PROJECT_PACKET_STACK_H
#define PROJECT_PACKET_STACK_H

#include "packet_interface.h"

typedef struct node node_t;
typedef struct stack stack;

struct node {
    pkt_t *p;           //On stocke un paquet
    node_t *next;    //Reference vers l'element suivant
};

struct stack {
    node_t *head;
};

/**
 * ajoute un element sur la stack
 * @param stack
 * @param p
 */
void addElem(stack *stack, pkt_t *p);

/**
 * Retire un element de la stack
 * @param stack
 * @return le paquet de la head, NULL le cas echeant
 */
pkt_t *removeElem(stack *stack);

/**
 *
 * @param stack
 * @return vide : 1 ; non vide : 0
 */
int isEmpty(stack *stack);

/**
 * Retourne le paquet contenu dans la head de la stack
 * @param stack
 * @return un paquet si head != NULL, NULL sinon
 */
pkt_t *first(stack *stack);

/**
 * Regarde si le paquet p est deja present dans la stack
 * @param stack
 * @param p
 * @return 0 si pas present, 1 sinon
 */
int isPresent(stack *stack, pkt_t *p);

#endif //PROJECT_PACKET_STACK_H
