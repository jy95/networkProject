//
// Created by Alexandre Dewit on 14/10/17.
//

#include "server_window_util.h"

window_util_t *new_window_util() {
    window_util_t *windowUtil = malloc(sizeof(window_util_t));
    return windowUtil;
}

void del_window_util(window_util_t *windowUtil) {
    while (windowUtil->storedPackets->head != NULL) {
        removeElem(windowUtil->storedPackets);
    }
    free(windowUtil->storedPackets);
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

pkt_t *remove_window_packet(window_util_t *windowUtil) {
    if (windowUtil == NULL) return NULL;
    return removeElem(windowUtil->storedPackets);
}

pkt_t *get_first_packet(window_util_t *windowUtil) {
    if (windowUtil == NULL) return NULL;
    return first(windowUtil->storedPackets);
}

int isPresent_packet_window(window_util_t *windowUtil, pkt_t *p) {
    if (windowUtil == NULL || p == NULL) return 0;
    return isPresent(windowUtil->storedPackets, p);
}

