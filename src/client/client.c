#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "../sendAndReceiveData/create_socket.h"
#include "../sendAndReceiveData/real_address.h"
#include "../sendAndReceiveData/read_write_loop.h"
#include "client.h"

int main(int argc, char *argv[]) {
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if ( option_arg == NULL) return EXIT_FAILURE;

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
    fprintf(stdout, "Use IPV6 Address: %s\n", ipAddress);

    // Step : Calculer le RTT initial pour envoyer un packet (en théorie, doit être recalculé à chaque réception de (N)ACK)
    // Pour cela, on va l'estimer à l'aide de MAX_LATENCE_TIME et EXPECTED_LATENCE_RATE
    struct timeval RTT;

    int socketFileDescriptor;

    // on init le temps supposé de latence
    int delay = MAX_LATENCE_TIME / EXPECTED_LATENCE_RATE;
    // ms -> s : 1/1000
    RTT.tv_sec = (delay / 1000);
    // us -> s : 1000
    RTT.tv_usec = (delay - (RTT.tv_sec * 1000)) * 1000;

    if ((socketFileDescriptor = create_socket(NULL, -1, &rval, option_arg->port)) < 0 )
        return EXIT_FAILURE; //On connecte le client au serveur

    fprintf(stdout, "Socket successfully created - listenning to port %d\n", option_arg->port);
    fprintf(stdout, "Initial calculated RTT : %d s - %d ms \n", (int) RTT.tv_sec, (int) (RTT.tv_usec / 1000));

    // Step : Envoi de message

    read_write_loop(socketFileDescriptor);

    // si on a ouvert le fp
    if ( fp ) {
        fclose(fp);
    }

    return EXIT_SUCCESS;
}

