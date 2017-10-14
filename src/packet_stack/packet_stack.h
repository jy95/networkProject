//
// Created by Alexandre Dewit on 14/10/17.
//

#ifndef PROJECT_PACKET_STACK_H
#define PROJECT_PACKET_STACK_H

#include "../paquet/packet_interface.h"



struct node {
    pkt_t *p;           //On stocke un paquet
    struct node *next;    //Reference vers l'element suivant
};

struct stack {
    struct node *head;
};

/**
 * ajoute un element sur la stack
 * @param stack
 * @param p
 */
void addElem(struct stack *Stack, pkt_t *p);

/**
 * Retire un element de la stack
 * @param stack
 * @return le paquet de la head, NULL le cas echeant
 */
pkt_t *removeElem(struct stack *Stack);

/**
 *
 * @param stack
 * @return vide : 1 ; non vide : 0
 */
int isEmpty(struct stack *Stack);

/**
 * Retourne le paquet contenu dans la head de la stack
 * @param stack
 * @return un paquet si head != NULL, NULL sinon
 */
pkt_t *first(struct stack *Stack);

/**
 * Regarde si le paquet p est deja present dans la stack
 * @param stack
 * @param p
 * @return 0 si pas present, 1 sinon
 */
int isPresent(struct stack *Stack, pkt_t *p);

#endif //PROJECT_PACKET_STACK_H
