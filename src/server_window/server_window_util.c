//
// Created by Alexandre Dewit on 14/10/17.
//

#include <stdio.h>
#include <stdlib.h>
#include "server_window_util.h"

window_util_t *new_window_util() {
    window_util_t *windowUtil = malloc(sizeof(window_util_t));
    if (windowUtil == NULL) return NULL;

    windowUtil->window_server = MAX_WINDOW_SIZE;
    windowUtil->lastReceivedSeqNum = 255;
    return windowUtil;
}

void del_window_util(window_util_t *windowUtil) {
    free(windowUtil);
}

uint8_t get_lastReceivedSeqNum(window_util_t *windowUtil) {
    return windowUtil->lastReceivedSeqNum;
}

uint8_t get_window(window_util_t *windowUtil) {
    return windowUtil->window;
}

uint8_t get_window_server(window_util_t *windowUtil) {
    return windowUtil->window_server;
}

void set_window_server(window_util_t *windowUtil, uint8_t size) {
    windowUtil->window_server = size;
}

void set_lastReceivedSeqNum(window_util_t *windowUtil, uint8_t lastReceivedSeqNum) {
    windowUtil->lastReceivedSeqNum = lastReceivedSeqNum;
}

unsigned int *get_seqAck(window_util_t *windowUtil) {
    return windowUtil->seqAck;
}

void set_seqAck(window_util_t *windowUtil, uint8_t seqnum) {
    (windowUtil->seqAck)[seqnum] = 1;
}

void unset_seqAck(window_util_t *windowUtil, uint8_t seqnum) {
    (windowUtil->seqAck)[seqnum] = 0;
}

int set_window(window_util_t *windowUtil, uint8_t window) {
    if (windowUtil == NULL || window > MAX_WINDOW_SIZE) return -1;
    windowUtil->window = window;
    return 0;
}

int get_first_value_window(window_util_t* windowUtil) {
    if(get_window(windowUtil) == 0) return 0;
    return (windowUtil->seqAck)[get_lastReceivedSeqNum(windowUtil) + 1];
}

int add_window_packet(window_util_t *windowUtil, pkt_t *p) {
    if (windowUtil == NULL || p == NULL) return -1;

    addElem(windowUtil->storedPackets, p);
    return 0;
}

pkt_t *remove_window_packet(window_util_t *windowUtil, uint8_t seqnum) {
    return removeElem(windowUtil->storedPackets, seqnum);
}

pkt_t *get_window_packet(window_util_t *windowUtil, uint8_t seqnum) {
    return getElem(windowUtil->storedPackets, seqnum);
}

unsigned int isPresent_seqnum_window(window_util_t *windowUtil, uint8_t seqnum) {
    if(get_window_server(windowUtil) == 0) return 0;
    uint8_t lastSeqNum = get_lastReceivedSeqNum(windowUtil);
    uint8_t EndIntervalWindow = lastSeqNum + get_window_server(windowUtil);

    int test1 = seqnum < lastSeqNum + 1 || seqnum > EndIntervalWindow; // Hors de la window
    int test2 = EndIntervalWindow - lastSeqNum + 1 > 0; //Cas normal : window ne chevauche pas le tableau
    int test3 = seqnum < lastSeqNum + 1 && seqnum > EndIntervalWindow; // Hors de la window (chevauche)
    int test4 = EndIntervalWindow - lastSeqNum + 1 < 0; // La window chevauche

    if ((test1 && test2) || (test3 && test4)) { //Nous ne sommes pas dans la sliding window
        return 0;
    }
    return (windowUtil->seqAck)[seqnum];
}

unsigned int isInSlidingWindow(window_util_t *windowUtil, uint8_t seqnum) {
    if(get_window_server(windowUtil) == 0) return 0;
    uint8_t lastSeqNum = get_lastReceivedSeqNum(windowUtil);
    uint8_t EndIntervalWindow = lastSeqNum + get_window_server(windowUtil);

    int test1 = seqnum < lastSeqNum + 1 || seqnum > EndIntervalWindow; // Hors de la window
    int test2 = EndIntervalWindow - lastSeqNum + 1 > 0; //Cas normal : window ne chevauche pas le tableau
    int test3 = seqnum < lastSeqNum + 1 && seqnum > EndIntervalWindow; // Hors de la window (chevauche)
    int test4 = EndIntervalWindow - lastSeqNum + 1 < 0; // La window chevauche

    if ((test1 && test2) || (test3 && test4)) { //Nous ne sommes pas dans la sliding window
        return 0;
    }

    return 1;
}

int set_seqnum_window(window_util_t *windowUtil, pkt_t *p) {
    uint8_t seqnum = pkt_get_seqnum(p);

    if(get_window_server(windowUtil) == 0) return 2;

    //Nous ne sommes pas dans la sliding window
    if (isInSlidingWindow(windowUtil, seqnum) == 0) {
        return 2;
    }

    //Le seqnum n'est pas encore present
    if (isPresent_seqnum_window(windowUtil, seqnum) == 0) {
        uint8_t window_server = get_window_server(windowUtil);
        window_server--;
        set_window_server(windowUtil, window_server);
        set_seqAck(windowUtil, seqnum); //On set a 1 la case numero seqnum de seqAck
        windowUtil->storedPackets[seqnum] = p;
        return 1;
    }

    //le seqnum est present et donc deja ajoute
    return 3;
}

void printer(window_util_t *windowUtil, pkt_t *first_pkt) {
    uint8_t seqnum = pkt_get_seqnum(first_pkt);

    if ((uint8_t) (get_lastReceivedSeqNum(windowUtil) + 1) == seqnum && isInSlidingWindow(windowUtil, seqnum)) {
        if(pkt_get_length(first_pkt) > 0) {
            fwrite(pkt_get_payload(first_pkt), pkt_get_length(first_pkt), 1, stdout);

        }

        set_lastReceivedSeqNum(windowUtil, seqnum);
        unset_seqAck(windowUtil, seqnum);

        seqnum++;

        pkt_t * p;
        // Tant qu'on est dans la window et que le premier numero de sequence est deja sotcke
        while (isInSlidingWindow(windowUtil, seqnum) && get_first_value_window(windowUtil) == 1) {

            p = removeElem(windowUtil->storedPackets, seqnum); // On retire l'element de la liste des paquets
            if(pkt_get_length(p) > 0) {
                fwrite(pkt_get_payload(p), pkt_get_length(p), 1, stdout); // on print le payload du paquet
            }
            set_lastReceivedSeqNum(windowUtil, seqnum); //On decale la window
            unset_seqAck(windowUtil, seqnum); // On retire la presence du paquet dans la liste des numero de sequence valide
            uint8_t window_server = get_window_server(windowUtil);
            window_server++;

            set_window_server(windowUtil, window_server);
            seqnum++;

            free(p);

        }

    }

}
