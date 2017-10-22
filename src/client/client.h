//
// Created by jy95 on 14/10/2017.
//

#ifndef PROJECT_CLIENT_H
#define PROJECT_CLIENT_H

#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "../paquet/packet_interface.h"
#include "../server_window/server_window_util.h"
#include "../common/common.h"

#define MAX_LATENCE_TIME    2000 // le temps maximal
#define MAX_PAYLOAD_SIZE    512  // la taille MAX du payload
#define MAX_PACKET_RECEIVED_SIZE 530// la taille max
#define DEFAULT_CLIENT_WINDOW_SIZE  5 // une taille de window par défaut

// structure pour le calcul de RTT
typedef struct networkInfo {
    int RTT; // le RTT en ms
    int windowsReceiver; // la taille de la window du receiver
} networkInfo;

// pour estimer le RTT et la taille de la window
// -1 en cas d'erreur ; sinon 0
int estimateRTTAndWindowSize(int sfd, networkInfo * receiverInfo);

// retour la valeur arrrondi en ms de la différence
int getDiffTimeInMs(time_t * start, time_t * end);

/**
 * Permet de savoir si notre fenêtre d'envoi (du sender) est pleine
 * cad qu'il n'est plus possible d'envoyer de nouveaux messages du sender au receiver
 * @param windowUtil
 * @return 1 si c'est le cas , 0 sinon
 */
int isSendingWindowFull(window_util_t *windowUtil,uint8_t FirstSeqNumInWindow);

// check si le numéro de seq dans la window du client
// 1, si c'est le cas ; 0 sinon
unsigned int isInSlidingWindowOfClient(uint8_t seqnum, uint8_t start, int count);

// Envoyer un / des messages sur la socket
void sendMessage(int * sendCounter,int * finalExit, uint8_t * SeqNumToBeSent,
                 int * transferNotFinished, int * socketFileDescriptor,window_util_t *windowUtil);

// check si la sliding window est full
int isSendingWindowFull(window_util_t *windowUtil, uint8_t FirstSeqNumInWindow);

#endif //PROJECT_CLIENT_H

// check si num valide
unsigned int isInSlidingWindowOfClient(uint8_t seqnum, uint8_t start, int count);

// resend de packets non correctement recus
void resendLostMessages(window_util_t *windowUtil, int * sendCounter,uint8_t * FirstSeqNumInWindow,
                        int * socketFileDescriptor,int * timer,int *finalExit,struct timeval * start_t );