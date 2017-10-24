#include "client.h"
#include "errno.h"
#include "time.h"
#include "../server_window/server_window_util.h"


void sendMessage(int * sendCounter,int * finalExit, uint8_t * SeqNumToBeSent,int * transferNotFinished, int * socketFileDescriptor,window_util_t *windowUtil){
    // on lit et on stocke
    char receivedBuffer[MAX_PAYLOAD_SIZE];
    int readCount;
    if ((readCount = read(STDIN_FILENO, receivedBuffer, MAX_PAYLOAD_SIZE)) <= 0 ) {

        if ( readCount == 0 ) {
            // on n'a rien lu et qu'on a tout envoyé ; on est arrivé à la fin
            if ( *sendCounter == 0 ) {
                fprintf(stderr,"No more data to send\n");
                *transferNotFinished = 0;
            }
        } else {
            // on a tenté de lire et on bloque
            if ( errno != EWOULDBLOCK && errno != EAGAIN ) {
                fprintf(stderr, "\t Cannot read from STDIN : %s\n", strerror(errno));
                *finalExit = EXIT_FAILURE;
            }
        }
    } else {
        pkt_t *packetToSent = pkt_new();

        fprintf(stderr, "A new payload is ready to be read from STDIN\n");

        if ( packetToSent == NULL ) {
            fprintf(stderr, "\t Cannot allocate for sending packet\n");
            *finalExit = EXIT_FAILURE;
        } else {

            // on construit le packet à envoyer
            pkt_set_type(packetToSent, PTYPE_DATA);
            pkt_set_window(packetToSent, get_window(windowUtil));
            pkt_set_timestamp(packetToSent, time(NULL));
            pkt_set_payload(packetToSent, receivedBuffer, readCount);
            pkt_set_seqnum(packetToSent, (*SeqNumToBeSent)++); // on incrémente le numéro après

            // on le stocke dans la window , au cas ou on devrait le renvoyer
            add_window_packet(windowUtil, packetToSent);

            fprintf(stderr, "\t Setting the packet N° %d to be send\n", pkt_get_seqnum(packetToSent));
            fprintf(stderr, "\t Payload : %d \n", pkt_get_length(packetToSent));

            // on encode notre packet
            char packetBuffer[MAX_PACKET_RECEIVED_SIZE];
            pkt_status_code problem;
            size_t writeLength = MAX_PACKET_RECEIVED_SIZE;
            if ((problem = pkt_encode(packetToSent, packetBuffer, &writeLength)) != PKT_OK ) {
                fprintf(stderr, "\t Cannot encode packet : ignored - err code : %d \n", problem);
                *finalExit = EXIT_FAILURE;
            } else {
                // on envoit finalement ce packet
                ssize_t writeCount;

                fprintf(stderr, "\t Packet correctly encoded - ready to be sent\n");

                if ((writeCount = send(*socketFileDescriptor, packetBuffer, writeLength, MSG_DONTWAIT)) <
                    0 ) {
                    if ( errno != EWOULDBLOCK && errno != EAGAIN ) {
                        fprintf(stderr, "\t Cannot write from dest : %s\n", strerror(errno));
                        *finalExit = EXIT_FAILURE;
                    }
                } else {
                    fprintf(stderr, "\t Packet correctly sent \n");
                    // on set le timer , en allouant  dynamiquement l'élément

                    if ( ((windowUtil->timers)[pkt_get_seqnum(packetToSent)]
                                  = malloc(sizeof(struct timeval))) == NULL ){
                        fprintf(stderr, "\t Cannot allocate timer : %s\n", strerror(errno));
                        *finalExit = EXIT_FAILURE;
                    } else {
                        gettimeofday((windowUtil->timers)[pkt_get_seqnum(packetToSent)],NULL); // set du timer
                        // on a envoyé un message
                        (*sendCounter)++;
                        // on diminue la taille de la window du receiver: 1 place va être occupé
                        set_window_server(windowUtil, get_window_server(windowUtil) - 1);
                    }
                }
            }
        }

    }
}