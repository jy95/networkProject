#include "headers/create_socket.h"
#include "headers/real_address.h"
#include "headers/read_write_loop.h"
#include "headers/wait_for_client.h"

#include <netdb.h> // Nécessaire
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h> // timeout pour select
#include <sys/types.h> // read, write
#include <unistd.h>

#define BUFFER_LENGTH   1024

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
    options.ai_protocol = 0;          /* N'importe quel protocol (pas nécessaire) */

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

    if ( ( socketFileDescriptor = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP) ) == -1) {
        // pas possible d'ouvrir le socket
        fprintf(stderr,"Cannot create socket\n");
        return -1;
    }

    // Etape 2 : se connecter

    // On veut bind le socket avec une source
    if (source_addr){

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
    if (dest_addr){

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

void read_write_loop(int sfd){

    // les variables pour opérer avec select
    fd_set readfds ;
    struct timeval tv;
    int result;
    int maxFd = sfd + 1; // the max fd + 1 for select;

    // settage du timeout (durée arbitraire : 3 secondes)
    tv.tv_sec = 3;

    // On vide l'ensemble
    FD_ZERO(&readfds);

    // espions pour lecture

    // On va espionner STDIN (fd : 0)
    FD_SET(0, &readfds);

    // et notre socket (sfd)
    FD_SET(sfd, &readfds);

    // On n'a pas besoin de setter ces champs là :
    // writefds , exceptfds
    result = select(maxFd, &readfds, NULL , NULL , &tv);

    if (result == -1){
        fprintf(stderr,"Error with select\n");
    } else if (result == 0){
        // timer expiré : ne rien faire
    } else {

        char    buf[BUFFER_LENGTH];

        // Des données sont disponibles sur stdin
        if (FD_ISSET(0, &readfds)) {

            // on s'assure que ce buffer est vide
            memset(buf,0,BUFFER_LENGTH);

            // lecture réussie
            if ( read(0,buf,BUFFER_LENGTH) >= 0 ){
                // On s'assure qu'il y ait bien un \0 à la fin
                int bufferLength = strlen(buf) -1;
                if (buf[bufferLength] == '\n')
                    buf[bufferLength] = '\0';

                // on envoit tout cela à la socket
                if ( write(sfd,buf,bufferLength + 1) < 0){
                    fprintf(stderr,"Cannot write message to socket\n");
                }
            } else {
                fprintf(stderr,"Cannot read STDIN\n");
            }

        }

        // Des données sont disponibles sur la socket
        if (FD_ISSET(sfd, &readfds)) {

            // on s'assure que ce buffer est vide
            memset(buf,0,BUFFER_LENGTH);

            // lecture réussie
            if ( read(sfd,buf,BUFFER_LENGTH) >= 0 ){

                // On s'assure qu'il y ait bien un \0 à la fin
                int bufferLength = strlen(buf) -1;
                if (buf[bufferLength] == '\n')
                    buf[bufferLength] = '\0';

                // On envoit tout cela sur STDOUT (FD : 1)
                if ( write(1,buf,BUFFER_LENGTH) < 0 ){
                    fprintf(stderr,"Cannot write message to socket\n");
                }

            } else {
                fprintf(stderr,"Cannot read socket\n");
            }
        }
    }
}

int wait_for_client(int sfd) {

    char    buf[BUFFER_LENGTH];

    // On s'assure qu'il sera vide
    memset(buf,0,BUFFER_LENGTH);

    struct sockaddr_in6 senderAddress;
    socklen_t fromlen = sizeof(senderAddress);

    // flags à 0 : on veut du bloquant
    if ( recvfrom(sfd,buf,BUFFER_LENGTH,0, (struct sockaddr *) &senderAddress, &fromlen ) == -1 ){
        fprintf(stderr,"Cannot recvfrom with socket\n");
        return -1;
    } else {

        // On veut se connecter à celui qui nous a contacté
        if ( connect(sfd, (struct sockaddr *) &senderAddress, sizeof(senderAddress) ) == -1 ){
            fprintf(stderr,"Cannot connect with destination address\n");
            return -1;
        }

        return 0;

    }

}