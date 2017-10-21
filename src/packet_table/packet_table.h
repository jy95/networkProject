//
// Created by Alexandre Dewit on 15/10/17.
//

#ifndef PROJECT_PACKET_TABLE_H
#define PROJECT_PACKET_TABLE_H

#include "../paquet/packet_interface.h"

/**
 * Permet d'ajouter un paquet dans la liste packets
 * @param packets
 * @param p
 */
void addElem(pkt_t **packets, pkt_t *p);

/**
 * Permet de retirer un element de la liste packets et de le retourner
 * @param packets
 * @param seqnum
 * @return le paquet retire de la liste packet
 */
pkt_t *removeElem(pkt_t **packets, uint8_t seqnum);

/**
 * Permet d'obtenir un element de la liste packets
 * @param packets
 * @param seqnum
 * @return La référence du packet
 */
pkt_t * getElem(pkt_t **packets, uint8_t seqnum);

#endif //PROJECT_PACKET_TABLE_H
