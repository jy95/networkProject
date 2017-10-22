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

            // renvoyer les messages
            resendLostMessages(windowUtil,&sendCounter,&FirstSeqNumInWindow,
                               &socketFileDescriptor,&timer,&finalExit,&start_t);

        }

        if ( result > 0 ) {
            // on a des données sur STDIN et qu'on peut encore envoyer un message
            // et que la window du receiver n'est pas égale à 0

            if ((ufds[0].revents & POLLIN)
                && isSendingWindowFull(windowUtil, FirstSeqNumInWindow) == 0
                && get_window_server(windowUtil) != 0 ) {

                // on envoit un message
                sendMessage(&sendCounter, &finalExit, &SeqNumToBeSent,
                            &transferNotFinished, &socketFileDescriptor,windowUtil);
            }

        }

        // on a des données sur SFD
        if ( ufds[1].revents & POLLIN ) {

            // on receptionne un ACK ou NACK
            receiveACKorNACK(&end_t,&start_t,&RTT,&timer,&finalExit,
                             &socketFileDescriptor,windowUtil,&sendCounter,&FirstSeqNumInWindow);

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