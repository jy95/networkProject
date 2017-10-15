//
// Created by Alexandre Dewit on 14/10/17.
//

#ifndef PROJECT_SERVER_WINDOW_UTIL_H

#define PROJECT_SERVER_WINDOW_UTIL_H

#include <stdlib.h>
#include <stdbool.h>
#include "../paquet/packet_interface.h"
#include "../packet_table/packet_table.h"

#define MAX_WINDOW_SIZE 32
#define MAX_STORED_PACKAGES 256


/*- PARTIE WINDOW -*/


typedef struct window_util {
    int lastReceivedSeqNum; //On stocke le dernier numero de sequence valide reçu
    pkt_t ** storedPackets; //On stocke tous les packets reçu
    uint8_t window_server; //On stocke le window du serveur
    uint8_t window; //On stocke le window du client
    unsigned int *seqAck;
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

/**
 * check si le numero de sequence en parametre est deja mis a true dans le seqAck
 * @param windowUtil
 * @param seqnum
 * @return pas present : 0 ; present : 1
 */
unsigned int isPresent_seqnum_window(window_util_t *windowUtil, uint8_t seqnum);

/**
 *
 * @param windowUtil
 * @return la liste des numero de sequence acquis ou non
 */
unsigned int *get_seqAck(window_util_t *windowUtil);

/**
 * Permet de set la presence d'un numero de sequence a 1 a la position seqnum du tableau seqAck
 * @param windowUtil
 * @param seqnum
 */
void set_seqAck(window_util_t *windowUtil, uint8_t seqnum);

/**
 * Permet d'enlever la presence d'un numero de sequence en mettant 0 a la position sqnum du tableau seqAck
 * @param windowUtil
 * @param seqnum
 */
void unset_seqAck(window_util_t *windowUtil, uint8_t seqnum);

/**
 *
 * @param windowUtil
 * @param seqnum
 * @return le paquet ayant le numero de sequence seqnum dans storedPacket
 */
pkt_t *remove_window_packet(window_util_t *windowUtil, uint8_t seqnum);

/**
 * Permet de check si un numero de sequence est dans les bornes de la sliding window
 * @param windowUtil
 * @param seqnum
 * @return 1 si dans les bornes, 0 sinon
 */
unsigned int isInSlidingWindow(window_util_t *windowUtil, uint8_t seqnum);

/**
 * Permet d'ajouter la reception du numero de sequence contenu dans p et p lui meme s'il n'a jamais ete ajoute dans
 * la liste des paquets
 * Dans certains cas, si le numero de sequence sui directement l'ancien numero de sequence valide, alors on retire le paquet
 * ayant l'ancien numero de sequence valide et on le retourne. Aussi, on supprime sa presence dans la table des ack et on decale la
 * sliding window
 * @param windowUtil
 * @param p
 * @return le paquet retire de la liste des paquets, NULL sinon
 */
pkt_t *set_seqnum_window(window_util_t *windowUtil, pkt_t *p);


#endif //PROJECT_SERVER_WINDOW_UTIL_H
