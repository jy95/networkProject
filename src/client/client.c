#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "../sendAndReceiveData/create_socket.h"
#include "../sendAndReceiveData/real_address.h"

option_t *get_option_arg(int argc, char *argv[]) {
    int opt;
    const char *filename = NULL;
    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;

                if (filename == NULL) {
                    fprintf(stderr, "L'option -%c requiert un argument\n", optopt);
                    return NULL;
                }
                break;
            default:
                fprintf(stderr, "erreur\n");
                return NULL;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Pas assez d'arguments");
        return NULL;
    }
    char *domaine = argv[optind];

    optind++;
    if (optind >= argc) {
        fprintf(stderr, "Pas assez d'arguments");
        return NULL;
    }
    char *port = argv[optind];
    option_t *option_arg = malloc(sizeof(struct option));
    if (option_arg == NULL) {
        fprintf(stderr, "Erreur allocation de memoire");
        return NULL;
    }
    option_arg->domaine = domaine;
    option_arg->filename = filename;
    option_arg->port = port;

    return option_arg;
}

int main(int argc, char *argv[]) {
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if (option_arg == NULL) return EXIT_FAILURE;

    struct sockaddr_in6 *rval = malloc(sizeof(struct sockaddr_in6));
    const char *message;
    if ((message = real_address(option_arg->domaine, rval)) !=
        NULL) { //On transforme l'addresse en structure lisible par la machine
        fprintf(stderr, "%s\n", message);
        return EXIT_FAILURE;
    }

    if (create_socket(NULL, -1, rval, (int) strtol(option_arg->port, NULL, 10)) < 0)
        return EXIT_FAILURE; //On connecte le client au serveur

    return EXIT_SUCCESS;
}

