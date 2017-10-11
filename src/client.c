#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "headers/real_address.h"

const char* real_full_address(const char *address, const char* port, struct sockaddr_in6 *rval) {
    // les options pour getaddrinfo
    struct addrinfo options;
    // la struct addrinfo qui va être modifier par getaddrinfo
    struct addrinfo *result;
    int errorCode;

    // settage des paramètres
    options.ai_family = AF_INET6; // IPV6
    options.ai_socktype = SOCK_DGRAM; // SOCK_DGRAM ou SOCK_STREAM
    options.ai_flags = 0;
    options.ai_protocol = IPPROTO_UDP;

    errorCode = getaddrinfo(address, port, &options, &result);
    if(errorCode != 0) {
        // gai_strerror nous donne une explication concrète avec le errorCode
        return gai_strerror(errorCode);
    } else {
        // un memccpy pour récupérer les infos dans la structure
        memcpy (rval, result->ai_addr, result->ai_addrlen);

        // pour s'assurer que le numéro de port est bien dans l'endianisme de la machine
        rval->sin6_port = htons(rval->sin6_port);

        // et que c'est de IPV6
        rval->sin6_family = AF_INET6;

        // on libère la ressource temporaire
        freeaddrinfo(result);

        return NULL;
    }
}

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
    option_t *option_arg = get_option_arg(argc, argv);
    if (option_arg == NULL) return EXIT_FAILURE;

    struct sockaddr_in6 *rval = malloc(sizeof(struct sockaddr_in6));
    const char* message;
    if((message = real_full_address(option_arg->domaine, option_arg->port, rval)) != NULL) {
        fprintf(stderr,"%s\n", message);
        return EXIT_FAILURE;
    }
}

