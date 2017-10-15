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
#define EXPECTED_LATENCE_RATE  10   // un pourcentage arbitraire pour évaluer délai/latence/le temps de traitement
#define INCREASE_SEARCH_RTT 20  // un nombre pour augmenter la recherche du RTT
#define MAX_PAYLOAD_SIZE    512  // la taille MAX du payload


// pour estimer le RTT
// renvoit le temps en ms qu'il a trouvé
// -1 en cas d'erreur
int estimateRTT(int sfd, struct sockaddr_in6 dest);

#endif //PROJECT_CLIENT_H
