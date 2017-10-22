//
// Created by jy95 on 22/10/2017.
//
#include "client.h"
#include "../server_window/server_window_util.h"

void resendLostMessages(window_util_t *windowUtil, int * sendCounter,uint8_t * FirstSeqNumInWindow,int * socketFileDescriptor,int * timer,int *finalExit,struct timeval * start_t ){
    //  le timer a expiré et la taille de la window n'est pas 0
    // timer expiré , on doit resender tous les packets non envoyés
    // pour savoir combien de paquets on peut renvoyer à receiver
    fprintf(stderr, "TIMER of %d ms expired : resend packets if possible\n", *timer);
    fprintf(stderr, "CONDITION CHECK : %d\n", get_window_server(windowUtil));
    if ( get_window_server(windowUtil) != 0 ) {
        int maxSendCounter = (*sendCounter < get_window_server(windowUtil)) ? *sendCounter : get_window_server(
                windowUtil);
        fprintf(stderr, "\t resend  %d packet(s)\n", maxSendCounter);
        // à partir de quel packet on send tout cela
        int startIndex = *FirstSeqNumInWindow;
        int shouldStopResend = 0; // pour arrêter prématurément la boucle
        int resendCounter = 0;

        // set du timer
        gettimeofday(start_t, NULL);

        // on arrête la boucle si on a un probleme
        while (shouldStopResend == 0 && *finalExit != EXIT_SUCCESS) {
            pkt_t *packetToBeResend = get_window_packet(windowUtil, startIndex);

            if ( packetToBeResend != NULL ) {
                char packetBuffer[MAX_PAYLOAD_SIZE];
                size_t writeLength = MAX_PAYLOAD_SIZE;
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
                    }
                }
            }

            // condition pour finir
            if ( resendCounter == maxSendCounter ) {
                shouldStopResend = 1;
            }
            // on avance au prochain packet
            startIndex++;
        }
    }
}