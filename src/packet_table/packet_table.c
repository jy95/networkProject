//
// Created by Alexandre Dewit on 15/10/17.
//

#include "../paquet/packet_interface.h"

void addElem(pkt_t **packets, pkt_t *p) {
    packets[p->structheader.seqNum] = p;
}

pkt_t *removeElem(pkt_t **packets, uint8_t seqnum) {
    if(packets[seqnum] == NULL) {
        return NULL;
    }

    pkt_t *p = packets[seqnum];
    packets[seqnum] = NULL;

    return p;

}