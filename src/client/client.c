#include "client.h"
#include "../sendAndReceiveData/create_socket.h"
#include "../sendAndReceiveData/real_address.h"
#include "../sendAndReceiveData/read_write_loop.h"

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

    int socketFileDescriptor;

    if ((socketFileDescriptor = create_socket(NULL, -1, &rval, option_arg->port)) < 0 )
        return EXIT_FAILURE; //On connecte le client au serveur

    fprintf(stdout, "Socket successfully created - listenning to port %d\n", option_arg->port);

    networkInfo receiverInfo;

    // Step : Calculer le RTT initial pour envoyer un packet (en théorie, doit être recalculé à chaque réception de (N)ACK)
    if ((estimateRTTAndWindowSize(socketFileDescriptor, &receiverInfo)) == -1 ) {
        fprintf(stdout, "Cannot estimate RTT and Window size of receiver\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Initial calculated RTT : %d ms \n", receiverInfo.RTT);
    fprintf(stdout, "Initial window size of receiver : %d ms \n", receiverInfo.windowsReceiver);

    // Step : Envoi de message
    // TODO
    read_write_loop(socketFileDescriptor);

    // si on a ouvert le fp
    if ( fp ) {
        fclose(fp);
    }

    return EXIT_SUCCESS;
}

