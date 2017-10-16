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
#include "../common/common.h"

#define MAX_LATENCE_TIME    2000 // le temps maximal
#define MAX_PAYLOAD_SIZE    512  // la taille MAX du payload

// structure pour le calcul de RTT
typedef struct networkInfo {
    int RTT; // le RTT en ms
    int windowsReceiver; // la taille de la window du receiver
} networkInfo;

// pour estimer le RTT et la taille de la window
// -1 en cas d'erreur ; sinon 0
int estimateRTTAndWindowSize(int sfd, struct networkInfo * receiverInfo);

// retour la valeur arrrondi en ms de la différence
int getDiffTimeInMs(time_t * start, time_t * end);

#endif //PROJECT_CLIENT_H
