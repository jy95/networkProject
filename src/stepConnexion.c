#include "headers/create_socket.h"
#include "headers/real_address.h"

#include <netdb.h> // Nécessaire
#include <stdio.h>
#include <string.h>

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
    //options.ai_protocol = 0;          /* N'importe quel protocol (pas nécessaire) */

    // on essaye de trouver une adresse
    // service à NULL parce qu'on a pas un numéro de port précis

    errorCode = getaddrinfo(address, NULL, &options, &result);
    if(errorCode != 0) {
        // On a un problème
        // gai_strerror nous donne une explication concrète avec le errorCode
        return gai_strerror(errorCode);
    } else {
        // en théorie , la structure addrinfo est récursive
        // nous, nous n'avons besoin que de la première adresse IPv6 qu'il a trouvé

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

int create_socket(struct sockaddr_in6 *source_addr,
                  int src_port,
                  struct sockaddr_in6 *dest_addr,
                  int dst_port) {


    // Etape 1 : ouverture du socket
    int socketFileDescriptor;
    // SOCK_DGRAM ou SOCK_STREAM pour le 3e param
    if ( ( socketFileDescriptor = socket(AF_INET6,SOCK_DGRAM,0) ) == -1) {
        // pas possible d'ouvrir le socket
        fprintf(stderr,"Cannot create socket\n");
        return -1;
    }

    // Etape 2 : se connecter

    // On veut bind le socket avec une source
    if (source_addr != NULL){

        if (src_port > 0) {
            source_addr->sin6_port = src_port;
        } else {
            source_addr->sin6_port = 0; // par défaut, n'importe quel port suffira
        }

        if ( bind(socketFileDescriptor, (struct sockaddr *) source_addr, sizeof(*source_addr) ) == -1 ){
            fprintf(stderr,"Cannot bind with source address\n");
            return -1;
        }
    }

    // On veut connecter le socket avec une destination
    if (dest_addr != NULL){

        if (dst_port > 0){
            dest_addr->sin6_port = dst_port;
        } else {
            dest_addr->sin6_port = 0; // par défaut, n'importe quel port suffira
        }
        // Bloque ici sur INGI
        if ( connect(socketFileDescriptor, (struct sockaddr *) dest_addr, sizeof(*dest_addr) ) == -1 ){
            fprintf(stderr,"Cannot connect with destination address\n");
            return -1;
        }

    }

    return socketFileDescriptor;
}