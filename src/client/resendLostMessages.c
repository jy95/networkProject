//
// Created by jy95 on 22/10/2017.
//
#include "client.h"
#include "../server_window/server_window_util.h"

void resendLostMessages(window_util_t *windowUtil, int * sendCounter,uint8_t * FirstSeqNumInWindow,int * socketFileDescriptor,int * timer,int *finalExit ){
    //  le timer a expiré et la taille de la window n'est pas 0
    // timer expiré , on doit resender tous les packets non envoyés
    // pour savoir combien de paquets on peut renvoyer à receiver

    int maxSendCounter = (*sendCounter < get_window_server(windowUtil)) ? *sendCounter : get_window_server(
                windowUtil);

    if (maxSendCounter > 0) {
        uint8_t startIndex = *FirstSeqNumInWindow;
        int resendCounter = 0;
        int processed = 0; // éléments déjà traités

        while (resendCounter <= maxSendCounter && processed < *sendCounter && *finalExit == EXIT_SUCCESS) {
            pkt_t *packetToBeResend = get_window_packet(windowUtil, startIndex);

            // set du timer
            struct timeval end_t; // pour checker le temps mis
            gettimeofday(&end_t, NULL); // on récupère la valeur actuelle

            // on récupère la valeur du timer pour ce packet
            struct timeval start_t = ( ((windowUtil->timers)[startIndex]) != NULL)
                                     ? *((windowUtil->timers)[startIndex]) : end_t;

            // difftime
            int diffTime =
                    (end_t.tv_sec - start_t.tv_sec) * 1000 + (end_t.tv_usec - start_t.tv_usec) / 1000;

            if ( (diffTime > *timer) && packetToBeResend != NULL ) {
                fprintf(stderr,"Packet N°%d must be resend - DELAY %d\n",pkt_get_seqnum(packetToBeResend),diffTime);
                char packetBuffer[MAX_PACKET_RECEIVED_SIZE];
                size_t writeLength = MAX_PACKET_RECEIVED_SIZE;
                pkt_status_code problem;

                if ((problem = pkt_encode(packetToBeResend, packetBuffer, &writeLength)) != PKT_OK ) {
                    fprintf(stderr, "Cannot encode packet : ignored - err code : %d \n", problem);
                    *finalExit = EXIT_FAILURE;
                } else {

                    // on envoit finalement ce packet
                    ssize_t writeCount;
                    if ((writeCount = send(*socketFileDescriptor, packetBuffer, writeLength, MSG_DONTWAIT)) <
                        0 ) {
                        if ( errno != EWOULDBLOCK && errno != EAGAIN ) {
                            fprintf(stderr, "Cannot write from dest : %s\n", strerror(errno));
                            *finalExit = EXIT_FAILURE;
                        }
                    } else {
                        // on set le nouveau temps pour ce timer
                        gettimeofday((windowUtil->timers)[startIndex],NULL);
                        resendCounter++;
                    }
                }
            }

            // on avance au prochain packet
            startIndex++;
            processed++;
        }
    }
}