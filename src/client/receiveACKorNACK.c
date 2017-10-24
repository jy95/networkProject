//
// Created by jy95 on 22/10/2017.
//
#include "client.h"
#include <errno.h>

void receiveACKorNACK(struct timeval * end_t, struct timeval * start_t , int * RTT, int * timer,  int * finalExit ,
                      int * socketFileDescriptor, window_util_t *windowUtil , int * sendCounter, uint8_t * FirstSeqNumInWindow ) {

    pkt_t *receivedPacket = pkt_new();

    // on alloue le packet
    if ( !receivedPacket ) {
        fprintf(stderr, "Cannot allocate for received packet\n");
        *finalExit = EXIT_FAILURE;
    } else {
        // on prépare le buffer qui va réceptionner notre packet
        char receivedBuffer[MAX_PACKET_RECEIVED_SIZE];

        // on lit ce qu'on a recu
        ssize_t countRead;
        if ((countRead = recv(*socketFileDescriptor, receivedBuffer, MAX_PACKET_RECEIVED_SIZE,
                              MSG_DONTWAIT)) < 0 ) {
            if ( errno != EWOULDBLOCK && errno != EAGAIN ) {
                fprintf(stderr, "Cannot read from dest : %s\n", strerror(errno));
                *finalExit = EXIT_FAILURE;
            }
        } else {

            pkt_status_code problem;
            if ((problem = pkt_decode(receivedBuffer, countRead, receivedPacket)) != PKT_OK ) {
                fprintf(stderr, "Corrupted Packet : ignored - err code : %d \n", problem);
            } else {

                // calculer le nouveau RTT
                //uint32_t timestampFromServer = pkt_get_timestamp(receivedPacket);

                //time_t *start = (time_t *) &timestamp;
                //time_t *end = (time_t *) &timestampFromServer;

                //uint32_t diffTime = 2 * getDiffTimeInMs(start, end);
                // on obtient le temps actuel
                gettimeofday(end_t, NULL);
                int diffTime =
                        (end_t->tv_sec - start_t->tv_sec) * 1000 + (end_t->tv_usec - start_t->tv_usec) / 1000;

                // réarmer le timer pour la prochaine itération : T2 - T1 / 2
                *RTT = (*RTT + diffTime) / 2;
                *timer = *RTT;

                fprintf(stderr, "\tReceived a packet , takes %d , RTT is : %d \n", diffTime, *RTT);

                // on set la taille de la window server
                set_window_server(windowUtil, pkt_get_window(receivedPacket));

                // On ne s'intéresse qu'aux packet de type ACK
                if ( pkt_get_type(receivedPacket) == PTYPE_ACK ) {

                    uint8_t seqNumToCheck = pkt_get_seqnum(receivedPacket);
                    fprintf(stderr, "RECEIVED : ACK with seqNum : %d \n", seqNumToCheck);

                    // On checke s'il est bien dans la sliding window
                    if ( isInSlidingWindowOfClient(seqNumToCheck, *FirstSeqNumInWindow, *sendCounter) == 1 ) {

                        // supprimer les packets de la window et la faire avancer
                        uint8_t deleteIndex = *FirstSeqNumInWindow;
                        int shouldStopRemove = 0; // stop remove when reach

                        while (shouldStopRemove == 0) {
                            // on supprime le packet
                            // côté client, on ne sait pas ce qu'on a réussi à envoyer avec un réordonnancement
                            pkt_t *packetToBeRemoved = remove_window_packet(windowUtil, deleteIndex);
                            if ( packetToBeRemoved != NULL ) {
                                free(packetToBeRemoved);
                                free((windowUtil->timers)[deleteIndex]); // suppression du timer
                            }
                            // on décrémente le nombre de packets envoyés (puisque le receiver a recu le packet)
                            sendCounter--;

                            // on incrémente la taille de notre window (puisqu'on a une nouvelle place libre)
                            set_window(windowUtil, get_window(windowUtil) + 1);

                            // quand on s'arrête
                            if ( deleteIndex == seqNumToCheck - 1 ) {
                                shouldStopRemove++;
                            }
                            deleteIndex++;
                        }

                        // on sait désormais que les n packets jusqu'à ce num (non inclus) ont été correctement envoyés et recues
                        // le premier numéro dans la window devient donc seqNumToCheck
                        *FirstSeqNumInWindow = (seqNumToCheck);
                    }

                }

                // Si on recoit un packet de type NACK ; on sait que le réseau est congestionné
                if ( pkt_get_type(receivedPacket) == PTYPE_NACK ) {

                    // de ce fait, on réduit notre window
                    set_window(windowUtil, get_window(windowUtil) - 1);
                }

            }

        }
        // on supprime progressivement le pkt
        pkt_del(receivedPacket);
    }
    // on free le packet temporaire
    free(receivedPacket);

}