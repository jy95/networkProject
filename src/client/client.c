#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../sendAndReceiveData/create_socket.h"
#include "../sendAndReceiveData/real_address.h"
#include "../sendAndReceiveData/read_write_loop.h"
#include "client.h"

int main(int argc, char *argv[]) {
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if ( option_arg == NULL) return EXIT_FAILURE;

    if ( option_arg->filename != NULL ) {
        FILE *fp;
        if ((fp = fopen(option_arg->filename, "r+")) == NULL ) {
            fprintf(stderr, "Le fichier ne peut pas être lue\n");
            return EXIT_FAILURE;
        }
        // maintenant ce fichier doit devenir l'entrée standard
        // dup2 avec STDIN
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

    int socketFileDescriptor;
    if ((socketFileDescriptor = create_socket(NULL, -1, &rval, option_arg->port)) < 0 )
        return EXIT_FAILURE; //On connecte le client au serveur

    fprintf(stdout, "Socket successfully created - listenning to port %d\n", option_arg->port);

    // Step : Calculer le RTT moyen pour envoyer un packet
    // Algo de base : envoyer 3 paquet tronqués hors séquence pour savoir


    // Step : Envoi de message

    read_write_loop(socketFileDescriptor);

    return EXIT_SUCCESS;
}

