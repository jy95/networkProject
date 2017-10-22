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
    if ((estimateRTTAndWindowSize(socketFileDescriptor, &receiverInfo)) == -1 ) {
        fprintf(stderr, "Cannot estimate RTT and Window size of receiver\n");
        return EXIT_FAILURE;
    }

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

    // tant que transfer pas fini
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

        int result = poll(ufds, 2, timer);

        if ( result == -1 ) {
            fprintf(stderr, "Problem with poll\n");
            finalExit = EXIT_FAILURE;
        }

        if ( result == 0 ) {

            resendLostMessages(windowUtil,&sendCounter,&FirstSeqNumInWindow,&socketFileDescriptor,&timer,&finalExit,&start_t);

        }

        if ( result > 0 ) {
            // on a des données sur STDIN et qu'on peut encore envoyer un message
            // et que la window du receiver n'est pas égale à 0

            if ((ufds[0].revents & POLLIN)
                && isSendingWindowFull(windowUtil, FirstSeqNumInWindow) == 0
                && get_window_server(windowUtil) != 0 ) {
                // on envoit un message
                sendMessage(&sendCounter, &finalExit, &SeqNumToBeSent, &transferNotFinished, &socketFileDescriptor,windowUtil);
            }

        }

        // on a des données sur SFD
        if ( ufds[1].revents & POLLIN ) {
            pkt_t *receivedPacket = pkt_new();

            // on alloue le packet
            if ( !receivedPacket ) {
                fprintf(stderr, "Cannot allocate for received packet\n");
                finalExit = EXIT_FAILURE;
            } else {
                // on prépare le buffer qui va réceptionner notre packet
                char receivedBuffer[MAX_PACKET_RECEIVED_SIZE];

                // on lit ce qu'on a recu
                ssize_t countRead;
                if ((countRead = recv(socketFileDescriptor, receivedBuffer, MAX_PACKET_RECEIVED_SIZE,
                                      MSG_DONTWAIT)) < 0 ) {
                    if ( errno != EWOULDBLOCK && errno != EAGAIN ) {
                        fprintf(stderr, "Cannot read from dest : %s\n", strerror(errno));
                        finalExit = EXIT_FAILURE;
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
                        gettimeofday(&end_t, NULL);
                        int diffTime =
                                (end_t.tv_sec - start_t.tv_sec) * 1000 + (end_t.tv_usec - start_t.tv_usec) / 1000;

                        // réarmer le timer pour la prochaine itération : T2 - T1 / 2
                        RTT = (RTT + diffTime) / 2;
                        timer = RTT;

                        fprintf(stderr, "\tReceived a packet , takes %d , RTT is : %d \n", diffTime, RTT);

                        // on set la taille de la window server
                        set_window_server(windowUtil, pkt_get_window(receivedPacket));

                        // On ne s'intéresse qu'aux packet de type ACK
                        if ( pkt_get_type(receivedPacket) == PTYPE_ACK ) {

                            uint8_t seqNumToCheck = pkt_get_seqnum(receivedPacket);
                            fprintf(stderr, "RECEIVED : ACK with seqNum : %d \n", seqNumToCheck);

                            // On checke s'il est bien dans la sliding window
                            if ( isInSlidingWindowOfClient(seqNumToCheck, FirstSeqNumInWindow, sendCounter) == 1 ) {

                                // supprimer les packets de la window et la faire avancer
                                uint8_t deleteIndex = FirstSeqNumInWindow;
                                int shouldStopRemove = 0; // stop remove when reach

                                while (shouldStopRemove == 0) {
                                    // on supprime le packet
                                    // côté client, on ne sait pas ce qu'on a réussi à envoyer avec un réordonnancement
                                    pkt_t *packetToBeRemoved = remove_window_packet(windowUtil, deleteIndex);
                                    if ( packetToBeRemoved != NULL ) {
                                        free(packetToBeRemoved);
                                    }
                                    // on décrémente le nombre de packets envoyés (puisque le receiver a recu le packet)
                                    sendCounter--;

                                    // on incrémente la taille de notre window (puisqu'on a une nouvelle place libre)
                                    set_window(windowUtil, get_window(windowUtil) + 1);

                                    // quand on s'arrête
                                    if ( deleteIndex == seqNumToCheck ) {
                                        shouldStopRemove++;
                                    }
                                    deleteIndex++;
                                }

                                // on sait désormais que les n packets jusqu'à ce num ont été correctement envoyés et recues
                                // le premier numéro dans la window devient donc seqNumToCheck +1
                                FirstSeqNumInWindow = (seqNumToCheck + 1);
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
    }


    fprintf(stderr, "END of transfer\n");

    // si on a ouvert le fp
    if ( fp ) {
        fclose(fp);
    }

    // on renvoit le dernier packet au client

    // on free tout
    free(windowUtil);

    return finalExit;
}