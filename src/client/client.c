#include "client.h"
#include <sys/poll.h>
#include <time.h>
#include "../sendAndReceiveData/create_socket.h"
#include "../sendAndReceiveData/real_address.h"
#include "../sendAndReceiveData/read_write_loop.h"
#include "../server_window/server_window_util.h"
#include "../packet_table/packet_table.h"

int main(int argc, char *argv[]) {
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if (option_arg == NULL) return EXIT_FAILURE;

    FILE *fp = NULL;

    // Redirection depuis le fichier
    // https://www.tutorialspoint.com/c_standard_library/c_function_freopen.htm
    // exemple : https://stackoverflow.com/a/586490/6149867
    if (option_arg->filename != NULL) {
        if ((fp = freopen(option_arg->filename, "r", stdin)) == NULL) {
            fprintf(stderr, "Le fichier ne peut pas être lue\n");
            return EXIT_FAILURE;
        }
    }

    struct sockaddr_in6 rval;
    const char *message;
    if ((message = real_address(option_arg->domaine, &rval)) !=
        NULL) { //On transforme l'addresse en structure lisible par la machine
        fprintf(stderr, "%s\n", message);
        return EXIT_FAILURE;
    }

    // on affiche l'adresse IPV6 qu'on utilise
    char ipAddress[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(rval.sin6_addr), ipAddress, INET6_ADDRSTRLEN);
    fprintf(stderr, "Use IPV6 Address: %s\n", ipAddress);

    int socketFileDescriptor;

    if ((socketFileDescriptor = create_socket(NULL, -1, &rval, option_arg->port)) < 0)
        return EXIT_FAILURE; //On connecte le client au serveur

    fprintf(stderr, "Socket successfully created - listenning to port %d\n", option_arg->port);

    // set mode non bloquant
    fcntl(socketFileDescriptor, F_SETFL, O_NONBLOCK);

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
    fprintf(stderr, "Initial window size of receiver : %d ms \n", receiverInfo.windowsReceiver);

    // Step : Envoi de message

    set_window_server(windowUtil, receiverInfo.windowsReceiver);
    set_window(windowUtil, DEFAULT_CLIENT_WINDOW_SIZE);

    int transferNotFinished = 1;
    int shouldRead = 1;

    // timestamp
    // TODO le timestamp qui va nous servir à l'envoi
    uint32_t timestamp;

    // la struct pour poll
    struct pollfd ufds[2];

    // lire le stdin
    ufds[0].fd = STDIN_FILENO;
    ufds[0].events = POLLIN; // check for just normal data

    // lire sfd
    ufds[1].fd = socketFileDescriptor;
    ufds[1].events = POLLIN;

    int timer = receiverInfo.RTT;
    int RTT = receiverInfo.RTT;
    int result;
    int finalExit = EXIT_SUCCESS;

    // numéros de séquences
    uint8_t SeqNumToBeSent = 0; // le numéro de séquence pour envoyer nos packets
    uint8_t FirstSeqNumInWindow = 0; // le premier numéro dans la window

    // tant que transfer pas fini
    while (transferNotFinished && finalExit != EXIT_SUCCESS) {

        result = poll(ufds, 2, timer);

        if (result == 0) {
            // timer expiré , on doit resender tous les packets non envoyés
            // TODO remove_window_packet
        }

        if (result == -1) {
            fprintf(stderr, "Problem with poll\n");
            finalExit = EXIT_FAILURE;
        }

        // on a des données sur STDIN (et on veut encore lire (et qu'on peut encore envoyer un message)
        if ((ufds[0].revents & POLLIN) && isSendingWindowFull(windowUtil,FirstSeqNumInWindow) == 0 && shouldRead) {

            // on lit et on stocke
            char receivedBuffer[MAX_PAYLOAD_SIZE];
            ssize_t readCount;

            if ((readCount = read(STDIN_FILENO, receivedBuffer, MAX_PAYLOAD_SIZE)) < 0 ) {
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    fprintf(stderr, "Cannot read from STDIN : %s\n", strerror(errno));
                    finalExit = EXIT_FAILURE;
                }
            } else {
                pkt_t *packetToSent = pkt_new();

                if (!packetToSent) {
                    fprintf(stderr, "Cannot allocate for sending packet\n");
                    finalExit = EXIT_FAILURE;
                } else {

                    // on construit le packet à envoyer
                    pkt_set_type(packetToSent, PTYPE_DATA);
                    pkt_set_window(packetToSent, get_window(windowUtil));
                    pkt_set_timestamp(packetToSent, time(NULL));
                    pkt_set_payload(packetToSent, receivedBuffer, readCount);
                    pkt_set_seqnum(packetToSent, SeqNumToBeSent++); // on incrémente le numéro après

                    // on le stocke dans la window , au cas ou on devrait le renvoyer
                    add_window_packet(windowUtil,packetToSent);

                    // on encode notre packet
                    char packetBuffer[MAX_PACKET_RECEIVED_SIZE];
                    pkt_status_code problem;
                    size_t writeLength = MAX_PACKET_RECEIVED_SIZE;
                    if ( (problem = pkt_encode(packetToSent,receivedBuffer,&writeLength)) != PKT_OK ) {
                        fprintf(stderr, "Cannot encode packet : ignored - err code : %d \n",problem);
                        finalExit = EXIT_FAILURE;
                    } else {
                        // on envoit finalement ce packet
                        ssize_t writeCount;

                        if ( (writeCount = write(socketFileDescriptor, packetBuffer, writeLength)) < 0){
                            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                                fprintf(stderr, "Cannot write from dest : %s\n", strerror(errno));
                                finalExit = EXIT_FAILURE;
                            }
                        }

                        // TODO le reste ?
                    }
                }

            }

            // feof , on a rencontré un EOF
            if (feof(stdin)) {
                shouldRead = 0;
            }
        }

        // on a des données sur SFD
        if (ufds[1].revents & POLLIN) {
            pkt_t *receivedPacket = pkt_new();

            // on alloue le packet
            if (!receivedPacket) {
                fprintf(stderr, "Cannot allocate for received packet\n");
                finalExit = EXIT_FAILURE;
            } else {
                // on prépare le buffer qui va réceptionner notre packet
                char receivedBuffer[MAX_PACKET_RECEIVED_SIZE];

                // on lit ce qu'on a recu
                ssize_t countRead;

                if ((countRead = read(socketFileDescriptor, receivedBuffer, MAX_PACKET_RECEIVED_SIZE)) < 0 ) {
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        fprintf(stderr, "Cannot read from dest : %s\n", strerror(errno));
                        finalExit = EXIT_FAILURE;
                    }
                } else {

                    pkt_status_code problem;
                    if ((problem = pkt_decode(receivedBuffer, countRead, receivedPacket)) != PKT_OK) {
                        fprintf(stderr, "Corrupted Packet : ignored - err code : %d \n",problem);
                    } else {

                        // calculer le nouveau RTT
                        uint32_t timestampFromServer = pkt_get_timestamp(receivedPacket);

                        time_t *start = (time_t *) &timestamp;
                        time_t *end = (time_t *) &timestampFromServer;

                        uint32_t diffTime = 2 * getDiffTimeInMs(start, end);

                        // réarmer le timer pour la prochaine itération : T2 - T1 / 2
                        RTT = (RTT + diffTime) / 2;
                        timer = RTT;

                        // On ne s'intéresse qu'aux packet de type ACK
                        if (pkt_get_type(receivedPacket) == PTYPE_ACK) {

                            uint8_t lastSeqNumReceivedByServer = pkt_get_seqnum(receivedPacket);

                            // TODO supprimer cela
                            // code bidon pour make
                            if (lastSeqNumReceivedByServer){

                            }

                            // TODO Code à continuer
                            /*
                            if (isInSlidingWindowOfClient(windowUtil,lastSeqNumReceivedByServer) == 1){

                                // TODO supprimer les packets de la window et la faire avancer

                                // on sait désormais que les n packets jusqu'à ce num ont été correctement envoyés et recues
                                set_lastReceivedSeqNum(windowUtil, lastSeqNumReceivedByServer);
                            }*/

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

    // si on a ouvert le fp
    if (fp) {
        fclose(fp);
    }

    // on free tout
    free(windowUtil);

    return finalExit;
}

int isSendingWindowFull(window_util_t *windowUtil,uint8_t FirstSeqNumInWindow) {

    // on obtient la taille
    int windowSize = get_window(windowUtil);

    // si la fenêtre du client est à 0, c'est bloquant
    if ( windowSize == 0 )
        return 1;

    // on commence
    uint8_t index = FirstSeqNumInWindow;
    // par défaut, y a pas de place
    int result = 1;
    // savoir si on doit continuer
    int shouldContinue = 1;
    // un counter pour savoir combien d'éléments on doit parcourir
    int count = 0;

    while (count < windowSize && shouldContinue == 1) {

        // technique en C pour checker si cela est null
        if ((windowUtil->storedPackets)[index]) {
            // on a trouvé un emplacement libre
            result = 0;
            // on arrête la boucle
            shouldContinue--;
        }
        index++;
        count++;
    }
    return result;
}