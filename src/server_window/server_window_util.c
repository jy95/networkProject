//
// Created by Alexandre Dewit on 14/10/17.
//

#include "server_window_util.h"

window_util_t *new_window_util() {
    window_util_t *windowUtil = malloc(sizeof(window_util_t));

    if (windowUtil == NULL) return NULL;

    windowUtil->storedPackets = malloc(sizeof(pkt_t) * MAX_STORED_PACKAGES);

    windowUtil->seqAck = (unsigned int *) calloc(256, sizeof(unsigned int));

    if (windowUtil->seqAck == NULL || windowUtil->storedPackets == NULL) {
        free(windowUtil);
        return NULL;
    }

    windowUtil->window_server = MAX_WINDOW_SIZE;
    windowUtil->lastReceivedSeqNum = -1;
    return windowUtil;
}

void del_window_util(window_util_t *windowUtil) {
    uint8_t i = 0;
    while (i != 256) {
        if ((windowUtil->storedPackets)[i] != NULL) {
            free((windowUtil->storedPackets)[i]);
        }
        i++;
    }
    free(windowUtil->storedPackets);
    free(windowUtil->seqAck);
    free(windowUtil);
}

int get_lastReceivedSeqNum(window_util_t *windowUtil) {
    return windowUtil->lastReceivedSeqNum;
}

int get_window(window_util_t *windowUtil) {
    return windowUtil->window;
}

int set_lastReceivedSeqNum(window_util_t *windowUtil, int lastReceivedSeqNum) {
    if (windowUtil == NULL) return -1;
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
    if (windowUtil == NULL || window > 31) return -1;
    windowUtil->window = window;
    return 0;
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
    int lastSeqNum = get_lastReceivedSeqNum(windowUtil);
    int endSlidingWindow = lastSeqNum + get_window(windowUtil);
    if (((seqnum < lastSeqNum + 1 ||
          seqnum > (lastSeqNum + get_window(windowUtil)) % MAX_STORED_PACKAGES) &&
         endSlidingWindow - lastSeqNum + 1 > 0)
        ||
        (((seqnum < lastSeqNum + 1 &&
           seqnum > (lastSeqNum + get_window(windowUtil)) % MAX_STORED_PACKAGES)) &&
         endSlidingWindow - lastSeqNum + 1 < 0)) { //Nous ne sommes pas dans la sliding window
        return 0;
    }
    unsigned int *seqAck = get_seqAck(windowUtil);
    return seqAck[seqnum];
}

unsigned int isInSlidingWindow(window_util_t *windowUtil, uint8_t seqnum) {
    int lastSeqNum = get_lastReceivedSeqNum(windowUtil);
    int endSlidingWindow = lastSeqNum + get_window(windowUtil);
    if (((seqnum < lastSeqNum + 1 ||
          seqnum > (lastSeqNum + get_window(windowUtil)) % MAX_STORED_PACKAGES) &&
         endSlidingWindow - lastSeqNum + 1 > 0)
        ||
        (((seqnum < lastSeqNum + 1 &&
           seqnum > (lastSeqNum + get_window(windowUtil)) % MAX_STORED_PACKAGES)) &&
         endSlidingWindow - lastSeqNum + 1 < 0)) { //Nous ne sommes pas dans la sliding window
        return 0;
    }

    return 1;
}

pkt_t *set_seqnum_window(window_util_t *windowUtil, pkt_t *p) {
    uint8_t seqnum = pkt_get_seqnum(p);
    int lastValideSeqnum = get_lastReceivedSeqNum(windowUtil);

    //Nous ne sommes pas dans la sliding window
    if(isInSlidingWindow(windowUtil, seqnum) == 0) {
        return NULL;
    }

    // Si nous avons le seqnum qui suit le precedent, on decale la sliding window
    if (seqnum == (uint8_t) (lastValideSeqnum + 1)) {
        // on évite le cas de base ou il n'y a pas encore eu de seqnum valide reçu
        if (lastValideSeqnum != -1) {
            if (isPresent_seqnum_window(windowUtil, seqnum) == 0) {
                addElem(windowUtil->storedPackets, p);
                set_seqAck(windowUtil, seqnum); //On set a 1 la case numero seqnum de seqAck
            }

            unset_seqAck(windowUtil, seqnum); //On set a 0 la case numero seqnum - 1 de seqAck
            set_lastReceivedSeqNum(windowUtil, seqnum);
            return removeElem(windowUtil->storedPackets, (uint8_t) (seqnum - 1));

        }

        addElem(windowUtil->storedPackets, p);
        set_seqAck(windowUtil, seqnum); //On set a 1 la case numero seqnum de seqAck
        set_lastReceivedSeqNum(windowUtil, seqnum);
        return NULL;                    //On retourne NULL car il n'y avait jamais eu de paquet valide avant


    }

    //Le seqnum n'est pas encore present
    if(isPresent_seqnum_window(windowUtil, seqnum) == 0) {
        set_seqAck(windowUtil, seqnum); //On set a 1 la case numero seqnum de seqAck
        addElem(windowUtil->storedPackets, p);
        return NULL;
    }

    //le seqnum est present et donc deja ajoute
    return NULL;
}

