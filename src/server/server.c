#include "server.h"

int main(int argc, char *argv[]) {
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if (option_arg == NULL) return EXIT_FAILURE;

    if (option_arg->filename != NULL) {
        FILE *fp;
        if ((fp = fopen(option_arg->filename, "w+")) == NULL) {
            fprintf(stderr, "Le fichier ne peut pas être lu\n");
            return EXIT_FAILURE;
        }
        // maintenant ce fichier doit devenir l'entrée standard
        // dup2 avec STDIN
    }

    struct sockaddr_in6 rval;
    const char *message;
    if ((message = real_address(option_arg->domaine, &rval)) !=
        NULL) { //On transforme l'addresse en structure lisible par la machine
        fprintf(stderr, "%s\n", message);
        return EXIT_FAILURE;
    }

    int socketFileDescriptor;
    if ((socketFileDescriptor = create_socket(&rval, option_arg->port, NULL, -1)) < 0)
        return EXIT_FAILURE; //On bind le serveur au client

    // 1) Wait for client
    // 2) Read write loop

    fprintf(stdout, "Socket successfully bind - listening to port %d\n", option_arg->port);

    ssize_t n = 0;

    char buffer[MAX_PACKET_WINDOW_SIZE * MAX_WINDOW_SIZE];
    socklen_t fromsize = sizeof rval;

    if ((n = recvfrom(socketFileDescriptor, buffer, MAX_PACKET_WINDOW_SIZE * MAX_WINDOW_SIZE, 0, (struct sockaddr *) &rval, &fromsize)) < 0 ) {
        fprintf(stderr, "Nothing to receive"); //On a rien reçu
        return EXIT_FAILURE;
    }

    pkt_t *p = pkt_new();

    if(p == NULL) return EXIT_FAILURE;

    if(pkt_decode(buffer, (const size_t) n, p) != PKT_OK) {
        fprintf(stderr, "Could not decode the packet"); //Erreur lors de la conversion du buffer en paquet
        return EXIT_FAILURE;
    }


    if ( sendto(socketFileDescriptor, buffer, strlen(buffer), 0, (struct sockaddr *) &rval, fromsize) < 0 ) {
        fprintf(stderr, "Sending error"); //Erreur lors de l'envoi des donnees
        return EXIT_FAILURE;
    }


    // Step : Envoi de message

    read_write_loop(socketFileDescriptor);

    return EXIT_SUCCESS;
}