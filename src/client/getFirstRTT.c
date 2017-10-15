#include "client.h"
#include "../paquet/packet_interface.h"
#include <sys/poll.h>

int estimateRTT(int sfd, struct sockaddr_in6 dest) {

    // la struct pour poll
    struct pollfd ufds[1];

    // on init le temps supposé de latence (en ms)
    int delay = MAX_LATENCE_TIME / EXPECTED_LATENCE_RATE;

    // un faux packet, envoyé hors séquence

    pkt_t *emptyPacket = pkt_new();

    // on n'arrive pas à créer le packet
    if ( !emptyPacket )
        return -1;

    pkt_set_seqnum(emptyPacket, 1);
    size_t length = 20;
    char buf[length];
    pkt_status_code problem;

    if ((problem = pkt_encode(emptyPacket, buf, &length)) != PKT_OK ) {
        return -1;
    }

    // on libère le pkt
    pkt_del(emptyPacket);

    // le temps trouvé
    int value = 0;
    int result;

    while (value == 0) {

        // on init
        ufds[0].fd = sfd;
        ufds[0].events = POLLIN; // check for just normal data

        // on send notre message
        sendto(sfd, buf, length, 0, (struct sockaddr *) &dest, sizeof dest);

        // on lance notre timer
        result = poll(ufds, 1, delay);

        if ( result == -1 ) {
            return -1;
        } else if ( result == 0 ) {
            // no data received ; increase the delay for next time
            fprintf(stdout, "Timer of %d ms expired ; try again!\n", delay);
            delay = delay + INCREASE_SEARCH_RTT;
        } else {
            // si on a recu des données de SFD, on sait plus ou moins le temps mis
            if ( ufds[0].revents & POLLIN ) {
                value = delay;
            }
        }
    }

    return value;

}
