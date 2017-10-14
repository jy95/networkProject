//
// Created by Alexandre Dewit on 14/10/17.
//

#ifndef PROJECT_SERVER_WINDOW_UTIL_H

#define PROJECT_SERVER_WINDOW_UTIL_H

#include <stdlib.h>
#include "../paquet/packet_interface.h"
#include "../packet_stack/packet_stack.h"


/*- PARTIE WINDOW -*/


typedef struct window_util {
    int lastReceivedSeqNum; //On stocke le dernier numero de sequence reçu
    struct stack *storedPackets; //On stocke tous les packets reçus
    uint8_t window; //On stocke le window du client
} window_util_t;

/**
 * initialise un window_util
 * @return un window_util, NULL le cas echeant
 */
window_util_t *new_window_util();

/**
 * free windowUtil
 * @return un window_util, NULL le cas echeant
 */
void del_window_util(window_util_t *windowUtil);

/**
 *
 * @param windowUtil
 * @return lastReceivedSeqNum de windowUtil
 */
int get_lastReceivedSeqNum(window_util_t *windowUtil);

/**
 *
 * @param windowUtil
 * @return window de windowUtil
 */
int get_window(window_util_t *windowUtil);

/**
 *
 * @param windowUtil
 * @param lastReceivedSeqNum
 * @return SUCCESS : 0 ; ELSE : -1
 */
int set_lastReceivedSeqNum(window_util_t *windowUtil, int lastReceivedSeqNum);

/**
 *
 * @param windowUtil
 * @param window
 * @return SUCCESS : 0 ; ELSE : -1
 */
int set_window(window_util_t *windowUtil, uint8_t window);

/**
 * Ajoute un packet dans la stack de windowUtil
 * @param windowUtil
 * @param p
 * @return SUCCESS : 0 ; ELSE : -1
 */
int add_window_packet(window_util_t *windowUtil, pkt_t *p);

/**
 * Retire le dernier paquet ajoute dans la stack de windowUtil et retourne ce paquet
 * @param windowUtil
 */
pkt_t *remove_window_packet(window_util_t *windowUtil);

/**
 * Retourne le dernier paquet ajoute dans la stack de windowUtil
 * @param windowUtil
 */
pkt_t *get_first_packet(window_util_t *windowUtil);

/**
 *
 * @param windowUtil
 * @param p
 * @return pas present : 0 ; present : 1
 */
int isPresent_packet_window(window_util_t *windowUtil, pkt_t *p);

#endif //PROJECT_SERVER_WINDOW_UTIL_H
