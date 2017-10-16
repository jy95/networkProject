#include "client.h"
#include "../paquet/packet_interface.h"
#include <sys/poll.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

int estimateRTTAndWindowSize(int sfd, struct networkInfo * receiverInfo) {

    // timers
    time_t start_t, end_t;

    // en théorie, on est sensé checker que l'addresse de destination recu est la même que celle envoyé
    // ici, je suppose qu'il n'y a de problème
    struct sockaddr_in6 dest;

    // un faux packet, envoyé hors séquence

    pkt_t *emptyPacket = pkt_new();

    // pour le stockage
    pkt_t * recu = pkt_new();

    //
    if ( (receiverInfo = calloc(1, sizeof(struct networkInfo))) == NULL){
        return -1;
    }

    // on n'arrive pas à créer le packet
    if ( !emptyPacket )
        return -1;

    // on n'arrive pas à créer le packet de stockage
    if ( !recu )
        return -1;

    int result = 0;
    unsigned int succes = 0;

    // init du faux packet
    pkt_set_seqnum(emptyPacket, 1);
    pkt_set_type(emptyPacket, PTYPE_DATA);
    pkt_set_tr(emptyPacket, 1);
    pkt_set_length(emptyPacket,0);

    size_t length = 20;
    char sendBuf[length];
    pkt_status_code problem;

    char receivedBuf[length];

    while (result != -1) {

        // la struct pour poll
        struct pollfd ufds[1];

        start_t = time(NULL);
        pkt_set_timestamp(emptyPacket, (const uint32_t) start_t);

        if ((problem = pkt_encode(emptyPacket, sendBuf, &length)) != PKT_OK ) {
            fprintf(stderr,"Cannot encode empty packet to get RTT : error %d \n",problem);
            result = -1;
        } else {
            // on init
            ufds[0].fd = sfd;
            ufds[0].events = POLLIN; // check for just normal data

            // démarrage du timer start
            time(&start_t);

            // on send notre message
            if ( sendto(sfd, sendBuf, length, 0, (struct sockaddr *) &dest, sizeof dest) == -1) {
                fprintf(stderr,"Cannot send empty packet to receiver\n");
                result = -1;
            } else {
                // on lance notre timer
                result = poll(ufds, 1, 2 *MAX_LATENCE_TIME);

                if ( result == -1 ) {
                    // ne rien faire
                } else if ( result == 0 ) {
                    // no data received ; increase the delay for next time
                    fprintf(stdout, "%d ms expired ; Reset a false packet \n", 2 * MAX_LATENCE_TIME);
                } else {
                    // si on a recu des données de SFD, on sait plus ou moins le temps mis
                    if ( ufds[0].revents & POLLIN ) {

                        // lecture du packet
                        socklen_t fromlen = sizeof dest;
                        int byte_count;

                        if ( (byte_count = recvfrom(sfd, receivedBuf, length, 0, (struct sockaddr *) &dest, &fromlen)) == -1 ){
                            fprintf(stderr,"Cannot allocate received buffer\n");
                            result = -1;
                        } else {

                            if ((problem = pkt_decode(receivedBuf,byte_count,recu)) != PKT_OK){
                                fprintf(stderr,"Cannot decode received packet to get RTT : error %d \n",problem);
                                result = -1;
                            } else {
                                // on récupère la valeur
                                end_t = pkt_get_timestamp(recu);

                                // le RTT est d'environ 2 * la différence entre ces deux timestamps
                                int diff = 2 * getDiffTimeInMs(&start_t,&end_t);

                                // on stocke la valeur
                                receiverInfo->RTT = diff;
                                receiverInfo->windowsReceiver = pkt_get_window(recu);

                                // on a fini
                                succes = 1;
                                result = -1;
                            }
                        }
                    }
                }
            }
        }
    }

    // on libère les pkt
    pkt_del(emptyPacket);
    pkt_del(recu);

    free(emptyPacket);
    free(recu);

    if (succes == 1){
        return 0;
    } else {
        free(receiverInfo);
        return -1;
    }

}

//
int getDiffTimeInMs(time_t * start, time_t * end){

    int calculatedTime;

    // difftime renvoit la différence en seconds par le type double
    // donc on cast et on multiple par 1000
    // pour avoir une valeur arrondi ; j'utilise floor + 0.5
    calculatedTime =  1000 * (int) floor( difftime(*end,*start) + 0.5);

    return calculatedTime;

}
