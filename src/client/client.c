#include "client.h"
#include <sys/poll.h>
#include <time.h>
#include <sys/time.h>
#include "../sendAndReceiveData/create_socket.h"
#include "../sendAndReceiveData/real_address.h"
#include "../sendAndReceiveData/read_write_loop.h"
#include "../server_window/server_window_util.h"
#include "../packet_table/packet_table.h"

int main(int argc, char *argv[]) {
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if ( option_arg == NULL ) return EXIT_FAILURE;

    FILE *fp = NULL;

    // Redirection depuis le fichier
    // https://www.tutorialspoint.com/c_standard_library/c_function_freopen.htm
    // exemple : https://stackoverflow.com/a/586490/6149867
    if ( option_arg->filename != NULL ) {
        if ((fp = freopen(option_arg->filename, "r", stdin)) == NULL ) {
            fprintf(stderr, "Le fichier ne peut pas être lue\n");
            return EXIT_FAILURE;
        }
    }

    struct sockaddr_in6 rval;
    const char *message;
    if ((message = real_address(option_arg->domaine, &rval)) !=
        NULL ) { //On transforme l'addresse en structure lisible par la machine
        fprintf(stderr, "%s\n", message);
        return EXIT_FAILURE;
    }

    // on affiche l'adresse IPV6 qu'on utilise
    char ipAddress[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(rval.sin6_addr), ipAddress, INET6_ADDRSTRLEN);
    fprintf(stderr, "Use IPV6 Address: %s\n", ipAddress);

    int socketFileDescriptor;

    if ((socketFileDescriptor = create_socket(NULL, -1, &rval, option_arg->port)) < 0 )
        return EXIT_FAILURE; //On connecte le client au serveur

    fprintf(stderr, "Socket successfully created - listenning to port %d\n", option_arg->port);


    networkInfo receiverInfo;
    // init de la windows
    window_util_t *windowUtil = new_window_util();
    if ( windowUtil == NULL ) {
        fprintf(stderr, "Cannot init client window\n");
        return EXIT_FAILURE;
    }

    // Step : Calculer le RTT initial pour envoyer un packet (en théorie, doit être recalculé à chaque réception de (N)ACK)
    /*
    if ((estimateRTTAndWindowSize(socketFileDescriptor, &receiverInfo)) == -1 ) {
        fprintf(stderr, "Cannot estimate RTT and Window size of receiver\n");
        return EXIT_FAILURE;
    }*/
    // set de base
    receiverInfo.RTT = 2 * MAX_LATENCE_TIME; // temps par défaut
    receiverInfo.windowsReceiver = 1; // on considère la taille de la window par défaut par défaut

    fprintf(stderr, "Initial calculated RTT : %d ms \n", receiverInfo.RTT);
    fprintf(stderr, "Initial window size of receiver : %d \n", receiverInfo.windowsReceiver);

    // Step : Envoi de message

    set_window_server(windowUtil, receiverInfo.windowsReceiver);
    set_window(windowUtil, DEFAULT_CLIENT_WINDOW_SIZE);

    int transferNotFinished = 1;

    // timestamp
    //uint32_t timestamp;

    int timer = receiverInfo.RTT;
    int RTT = receiverInfo.RTT;
    int finalExit = EXIT_SUCCESS;

    // numéros de séquences
    uint8_t SeqNumToBeSent = 0; // le numéro de séquence pour envoyer nos packets
    uint8_t FirstSeqNumInWindow = 0; // le premier numéro dans la window

    // nombre de packets envoyés
    int sendCounter = 0;

    // tant que transfer pas fini ou une erreur s'est produite
    while (transferNotFinished && finalExit == EXIT_SUCCESS) {

        struct timeval start_t, end_t;

        // la struct pour poll
        struct pollfd ufds[2];

        // lire le stdin
        ufds[0].fd = STDIN_FILENO;
        ufds[0].events = POLLIN; // check for just normal data

        // lire sfd
        ufds[1].fd = socketFileDescriptor;
        ufds[1].events = POLLIN;

        // multiplexing de STDIN_FILENO et socketFileDescriptor
        int result = poll(ufds, 2, timer);

        // poll
        if ( result == -1 ) {
            fprintf(stderr, "Problem with poll\n");
            finalExit = EXIT_FAILURE;
        } else if ( result == 0 ) {

        } else {
            // un ou deux file descriptors sont actifs
            // on a des données sur STDIN et qu'on peut encore envoyer un message
            // et que la window du receiver n'est pas égale à 0

            if ((ufds[0].revents & POLLIN)
                && isSendingWindowFull(windowUtil, FirstSeqNumInWindow) == 0
                && get_window_server(windowUtil) != 0 ) {

                // on envoit un message
                sendMessage(&sendCounter, &finalExit, &SeqNumToBeSent,
                            &transferNotFinished, &socketFileDescriptor,windowUtil);
            }

            // on a des données sur SFD
            if ( ufds[1].revents & POLLIN ) {

                // on receptionne un ACK ou NACK
                receiveACKorNACK(&end_t,&start_t,&RTT,&timer,&finalExit,
                                 &socketFileDescriptor,windowUtil,&sendCounter,&FirstSeqNumInWindow);

            }

        }

        // fin du timer probable : check de timers
        // renvoyer les messages dont on n'a pas recu de ACK
        resendLostMessages(windowUtil,&sendCounter,&FirstSeqNumInWindow,
                           &socketFileDescriptor,&timer,&finalExit);
    }


    fprintf(stderr, "END of transfer\n");

    // envoi du dernier numéro de séquence
    if (finalExit == EXIT_SUCCESS){

        // on renvoit le dernier packet au client
        finalExit = sendLastPacket(SeqNumToBeSent,timer,socketFileDescriptor);

    }

    // si on a ouvert le fp
    if ( fp ) {
        fclose(fp);
    }

    // on free tout
    free(windowUtil);

    return finalExit;
}

int sendLastPacket(int SeqNumToBeSent,int timer, int sfd){
    // on crée le packet qui va signaler la fin du transfer
    pkt_t *emptyPacket = pkt_new();

    // on n'arrive pas à créer le packet
    if ( !emptyPacket )
        return EXIT_FAILURE;

    // pour le stockage
    pkt_t *recu = pkt_new();

    // on n'arrive pas à créer le packet de stockage
    if ( !recu )
        return EXIT_FAILURE;

    // init du dernier packet
    pkt_set_seqnum(emptyPacket, SeqNumToBeSent - 1 );
    pkt_set_type(emptyPacket, PTYPE_DATA);
    pkt_set_window(emptyPacket, 1); // on ne veut être notifié
    pkt_set_length(emptyPacket, 0); // un packet vide

    // condition pour break le while
    int hasReceivedResponse = 1;
    unsigned int succes = 0;
    int result = 0;
    // pour envoyer un seul message
    int shouldSend = 1;

    while (hasReceivedResponse && result != -1){
        // envoi du packet

        size_t length = 20; // 12 bytes devraient être suffisants
        char sendBuf[length];
        pkt_status_code problem;
        char receivedBuf[length];

        // la struct pour poll
        struct pollfd ufds[1];

        pkt_set_timestamp(emptyPacket, (const uint32_t) time(NULL));

        if ((problem = pkt_encode(emptyPacket, sendBuf, &length)) != PKT_OK ) {
            fprintf(stderr, "Cannot encode END packet  : %d\n", problem);
            return EXIT_FAILURE;
        } else {
            // on init
            ufds[0].fd = sfd;
            ufds[0].events = POLLIN; // pour envoyer/recevoir des données

            if (( shouldSend )) {
                ssize_t writeCount;
                fprintf(stderr, "Trying to send a END packed \n");
                if ((writeCount = send(sfd, sendBuf, length, MSG_DONTWAIT)) < 0 ) {
                    if ( errno != EWOULDBLOCK && errno != EAGAIN ) {
                        fprintf(stderr, "Cannot send END packet to receiver : %s\n", strerror(errno));
                        result = -1;
                    }
                } else {
                    shouldSend = 0;
                }
            }

            // on lance notre timer
            result = poll(ufds, 1, timer);

            if ( result == -1 ) {
                // ne rien faire
            } else if ( result == 0 ) {
                // no data received ; increase the delay for next time
                fprintf(stdout, "%d ms expired ; Reset a END packet \n", timer);
                shouldSend = 1;
            } else {

                // si on a recu des données de SFD, on sait plus ou moins le temps mis
                if ( ufds[0].revents & POLLIN ) {

                    // lecture du packet
                    ssize_t byte_count;

                    if ((byte_count = recv(sfd, receivedBuf, length, MSG_DONTWAIT)) < 0 ) {
                        if ( errno != EWOULDBLOCK && errno != EAGAIN ) {
                            fprintf(stderr, "Cannot allocate received buffer : %s\n", strerror(errno));
                            result = -1;
                        }
                    } else {

                        if ((problem = pkt_decode(receivedBuf, byte_count, recu)) != PKT_OK ) {
                            fprintf(stderr, "Cannot decode received packet to get RTT : error %d \n", problem);
                            result = -1;
                        } else {

                            // si c'est celui qu'on attendait
                            if ( (pkt_get_seqnum(recu) == pkt_get_seqnum(emptyPacket))
                                 && (pkt_get_type(recu) == PTYPE_ACK) ){
                                // on a fini
                                succes = 1;
                                hasReceivedResponse = 0;
                            }
                        }
                    }
                }
            }

        }

    }

    if (succes) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }

}