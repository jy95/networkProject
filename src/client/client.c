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
    fprintf(stdout, "Use IPV6 Address: %s\n", ipAddress);

    int socketFileDescriptor;

    if ((socketFileDescriptor = create_socket(NULL, -1, &rval, option_arg->port)) < 0)
        return EXIT_FAILURE; //On connecte le client au serveur

    fprintf(stdout, "Socket successfully created - listenning to port %d\n", option_arg->port);

    networkInfo receiverInfo;

    // Step : Calculer le RTT initial pour envoyer un packet (en théorie, doit être recalculé à chaque réception de (N)ACK)
    if ((estimateRTTAndWindowSize(socketFileDescriptor, &receiverInfo)) == -1) {
        fprintf(stdout, "Cannot estimate RTT and Window size of receiver\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Initial calculated RTT : %d ms \n", receiverInfo.RTT);
    fprintf(stdout, "Initial window size of receiver : %d ms \n", receiverInfo.windowsReceiver);

    // Step : Envoi de message

    // init de la windows
    window_util_t *windowUtil = new_window_util();
    set_window_server(windowUtil, receiverInfo.windowsReceiver);

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

    // tant que transfer pas fini
    while (transferNotFinished && finalExit != EXIT_SUCCESS) {

        result = poll(ufds, 2, timer);

        if (result == 0) {
            // timer expiré , on doit resender tous les packets non envoyés
            // TODO resender les packets de la window
        }

        if (result == -1) {
            fprintf(stderr, "Problem with poll\n");
            finalExit = EXIT_FAILURE;
        }

        // on a des données sur STDIN (et on veut encore lire)
        if ((ufds[0].revents & POLLIN) && shouldRead) {

            // on lit et on stocke
            char receivedBuffer[MAX_PACKET_RECEIVED_SIZE];
            ssize_t readCount;

            if ((readCount = read(socketFileDescriptor, receivedBuffer, MAX_PACKET_RECEIVED_SIZE)) == -1) {
                fprintf(stderr, "Cannot read from socket : %s\n", strerror(errno));
                finalExit = EXIT_FAILURE;
            } else {
                pkt_t *packetToSent = pkt_new();

                if (!packetToSent) {
                    fprintf(stderr, "Cannot allocate for sending packet\n");
                    finalExit = EXIT_FAILURE;
                } else {
                    pkt_set_type(packetToSent, PTYPE_DATA);
                    pkt_set_window(packetToSent, get_window(windowUtil));
                    pkt_set_timestamp(packetToSent, time(NULL));

                    // TODO le reste
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
                socklen_t fromlen = sizeof rval;

                if ((countRead = recvfrom(socketFileDescriptor, receivedBuffer, MAX_PACKET_RECEIVED_SIZE, 0,
                                          (struct sockaddr *) &rval, &fromlen)) == -1) {
                    fprintf(stderr, "Cannot recvfrom from dest : %s\n", strerror(errno));
                    finalExit = EXIT_FAILURE;
                } else {

                    pkt_status_code problem;
                    if ((problem = pkt_decode(receivedBuffer, countRead, receivedPacket)) != PKT_OK) {
                        fprintf(stderr, "Corrupted Packet : ignored \n");
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

                            // on sait désormais que les n packets ont été correctement recues
                            set_lastReceivedSeqNum(windowUtil, lastSeqNumReceivedByServer);

                            // TODO supprimer les packets de la window et la faire avancer

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

    // TODO on free tout

    return finalExit;
}

