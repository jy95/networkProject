#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "headers/real_address.h"

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port) {

    // Doc : http://beej.us/guide/bgnet/output/html/multipage/syscalls.html#socket

    // Etape 1 : ouverture du socket
    int socketFileDescriptor;

    // on crée le socket
    if ((socketFileDescriptor = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == -1) {

        // pas possible d'ouvrir le socket
        fprintf(stderr, "Cannot create socket\n");
        return -1;
    }

    // On veut bind le socket avec une source
    if (source_addr) {

        // un port custom a été demandé
        if (src_port > 0) {
            source_addr->sin6_port = (in_port_t) src_port;
        }

        if (bind(socketFileDescriptor, (struct sockaddr *) source_addr, sizeof(struct sockaddr_in6)) == -1) {
            int errnum = errno;
            fprintf(stderr, "Cannot bind with source address : %s\n", strerror(errnum));
            return -1;
        }

    }

        // On veut connecter le socket avec une destination
    else if (dest_addr) {

        // un port custom a été demandé
        if (dst_port > 0) {
            dest_addr->sin6_port = (in_port_t) dst_port;
        }

        if (connect(socketFileDescriptor, (struct sockaddr *) dest_addr, sizeof(struct sockaddr_in6)) == -1) {
            int errnum = errno;
            fprintf(stderr, "Cannot connect with destination address : %s\n", strerror(errnum));
            return -1;
        }
    }

    return socketFileDescriptor;
}

const char *real_address(const char *address, struct sockaddr_in6 *rval) {
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

    // on essaye de trouver une adresse
    // service à NULL parce qu'on a pas un numéro de port précis

    errorCode = getaddrinfo(address, NULL, &options, &result);
    if (errorCode != 0) {
        // On a un problème
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
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if (option_arg == NULL) return EXIT_FAILURE;

    struct sockaddr_in6 *rval = malloc(sizeof(struct sockaddr_in6));
    const char *message;
    if ((message = real_address(option_arg->domaine, rval)) != NULL) { //On transforme l'addresse en structure lisible par la machine
        fprintf(stderr, "%s\n", message);
        return EXIT_FAILURE;
    }

    if (create_socket(NULL, -1, rval, (int) strtol(option_arg->port, NULL, 10)) < 0) return EXIT_FAILURE; //On connecte le client au serveur


}

