//
// Created by Alexandre Dewit on 14/10/17.
//

#include <stdio.h>
#include "server_window_util.h"

window_util_t *new_window_util() {
    window_util_t *windowUtil = malloc(sizeof(window_util_t));
    if (windowUtil == NULL) return NULL;

    windowUtil->window_server = MAX_WINDOW_SIZE;
    windowUtil->lastReceivedSeqNum = -1;
    return windowUtil;
}

void del_window_util(window_util_t *windowUtil) {
    free(windowUtil);
}

int get_lastReceivedSeqNum(window_util_t *windowUtil) {
    return windowUtil->lastReceivedSeqNum;
}

int get_window(window_util_t *windowUtil) {
    return windowUtil->window;
}

uint8_t get_window_server(window_util_t *windowUtil) {
    return windowUtil->window_server;
}

void set_window_server(window_util_t *windowUtil, uint8_t size) {
    windowUtil->window_server = size;
}

int set_lastReceivedSeqNum(window_util_t *windowUtil, int lastReceivedSeqNum) {
    windowUtil->lastReceivedSeqNum = lastReceivedSeqNum;
    return 0;
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
    return (windowUtil->seqAck)[(get_lastReceivedSeqNum(windowUtil) + 1) % MAX_STORED_PACKAGES];
}

int add_window_packet(window_util_t *windowUtil, pkt_t *p) {
    if (windowUtil == NULL || p == NULL) return -1;

    addElem(windowUtil->storedPackets, p);
    return 0;
}

pkt_t *remove_window_packet(window_util_t *windowUtil, uint8_t seqnum) {
    return removeElem(windowUtil->storedPackets, seqnum);
}

unsigned int isPresent_seqnum_window(window_util_t *windowUtil, uint8_t seqnum) {
    if(get_window_server(windowUtil) == 0) return 0;

    int lastSeqNum = get_lastReceivedSeqNum(windowUtil);
    int EndIntervalWindow = (lastSeqNum + get_window_server(windowUtil)) % MAX_STORED_PACKAGES;

    int test1 = seqnum < (lastSeqNum + 1) % MAX_STORED_PACKAGES || seqnum > EndIntervalWindow; // Hors de la window
    int test2 = EndIntervalWindow - lastSeqNum + 1 > 0; //Cas normal : window ne chevauche pas le tableau
    int test3 = seqnum < (lastSeqNum + 1) % MAX_STORED_PACKAGES && seqnum > EndIntervalWindow; // Hors de la window (chevauche)
    int test4 = EndIntervalWindow - lastSeqNum + 1 < 0; // La window chevauche

    if ((test1 && test2) || (test3 && test4)) { //Nous ne sommes pas dans la sliding window
        return 0;
    }
    return (windowUtil->seqAck)[seqnum];
}

unsigned int isInSlidingWindow(window_util_t *windowUtil, uint8_t seqnum) {
    if(get_window_server(windowUtil) == 0) return 0;
    int lastSeqNum = get_lastReceivedSeqNum(windowUtil);
    int EndIntervalWindow = (lastSeqNum + get_window_server(windowUtil)) % MAX_STORED_PACKAGES;

    int test1 = seqnum < (lastSeqNum + 1) % MAX_STORED_PACKAGES || seqnum > EndIntervalWindow; // Hors de la window
    int test2 = EndIntervalWindow - lastSeqNum + 1 > 0; //Cas normal : window ne chevauche pas le tableau
    int test3 = seqnum < (lastSeqNum + 1) % MAX_STORED_PACKAGES && seqnum > EndIntervalWindow; // Hors de la window (chevauche)
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

    if ((get_lastReceivedSeqNum(windowUtil) + 1) % MAX_STORED_PACKAGES == seqnum && isInSlidingWindow(windowUtil, seqnum)) {

        fprintf(stdout, "%s", pkt_get_payload(first_pkt));

        set_lastReceivedSeqNum(windowUtil, seqnum);
        unset_seqAck(windowUtil, seqnum);

        seqnum = (uint8_t) ((seqnum + 1) % MAX_STORED_PACKAGES);

        pkt_t * p;
        // Tant qu'on est dans la window et que le premier numero de sequence est deja sotcke
        while (isInSlidingWindow(windowUtil, seqnum) && get_first_value_window(windowUtil) == 1) {

            p = removeElem(windowUtil->storedPackets, seqnum); // On retire l'element de la liste des paquets
            fprintf(stdout, "%s", pkt_get_payload(p)); // on print le payload du paquet
            set_lastReceivedSeqNum(windowUtil, seqnum); //On decale la window
            unset_seqAck(windowUtil, seqnum); // On retire la presence du paquet dans la liste des numero de sequence valide
            uint8_t window_server = get_window_server(windowUtil);
            window_server++;

            set_window_server(windowUtil, window_server);
            seqnum = (uint8_t) ((seqnum + 1) % MAX_STORED_PACKAGES);

            free(p);

        }

    }

}

