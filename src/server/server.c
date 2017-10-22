#include "server.h"
#include "../sendAndReceiveData/wait_for_client.h"
#include "../server_window/server_window_util.h"

int main(int argc, char *argv[]) {
    option_t *option_arg = get_option_arg(argc, argv); //On ajoute dans la structure les infos de la ligne de commande
    if (option_arg == NULL) return EXIT_FAILURE;
    FILE *fp = NULL;

    if (option_arg->filename != NULL) {
        if ((fp = freopen(option_arg->filename, "w", stdout)) == NULL) {
            fprintf(stderr, "Le fichier ne peut pas être lue\n");
            return EXIT_FAILURE;
        }

    }

    struct sockaddr_in6 rval;
    const char *message;
    if ((message = real_address(option_arg->domaine, &rval)) !=
        NULL) { //On transforme l'addresse en structure lisible par la machine
        fprintf(stderr, "%s\n", message);
        if (fp != NULL) fclose(fp);

        return EXIT_FAILURE;
    }

    int socketFileDescriptor;
    if ((socketFileDescriptor = create_socket(&rval, option_arg->port, NULL, -1)) < 0) {
        if (fp != NULL) fclose(fp);
        return EXIT_FAILURE; //On bind le serveur au client
    }

    fprintf(stderr, "Socket successfully bind - listening to port %d\n", option_arg->port);

    ssize_t n = 0;

    char buffer[MAX_PACKET_WINDOW_SIZE * MAX_WINDOW_SIZE];
    socklen_t fromsize = sizeof rval;

    int lengthReceivedPacket = 1;
    int seqnumPacket = -1;
    int lastSeqAck = -1;

    window_util_t *windowUtil = new_window_util();

    if (windowUtil == NULL) {
        if (fp != NULL) fclose(fp);
        return EXIT_FAILURE;
    }

    fprintf(stderr,"Waiting for client \n");
    if (wait_for_client(socketFileDescriptor) == -1) {       //Connexion a un client
        fprintf(stderr, "failed to wait for client");
        if (fp != NULL) fclose(fp);
        return EXIT_FAILURE;
    }

    fprintf(stderr,"A client is connected\n");

    while (lengthReceivedPacket != 0 || seqnumPacket != lastSeqAck) {

        lastSeqAck = seqnumPacket;

        //Recuperation des donnees
        if ((n = recvfrom(socketFileDescriptor, buffer, MAX_PACKET_WINDOW_SIZE * MAX_WINDOW_SIZE, 0,
                          (struct sockaddr *) &rval, &fromsize)) < 0) {
            fprintf(stderr, "Nothing to receive\n"); //On a rien reçu
            if (fp != NULL) fclose(fp);
            return EXIT_FAILURE;
        }

        // Recuperation du paquet

        pkt_t *p = pkt_new();

        if (p == NULL) {
            if (fp != NULL) fclose(fp);
            return EXIT_FAILURE;
        }

        int isIgnore = 0;

        // on decode le buffer
        if (pkt_decode(buffer, (const size_t) n, p) != PKT_OK) {
            fprintf(stderr, "Could not decode the packet\n"); //Erreur lors de la conversion du buffer en paquet
            isIgnore = 1;
        }

        //Traitement du paquet

        lengthReceivedPacket = pkt_get_length(p);
        seqnumPacket = pkt_get_seqnum(p);

        int test1 = (lengthReceivedPacket >= 1 && lengthReceivedPacket <= 512);  // 1 =< length <= 512
        int test2 = isInSlidingWindow(windowUtil, pkt_get_seqnum(p)) == 1; //On est dans la sliding window

        // Le paquet n'est pas ignore + seqnumPacket != lastSeqAck + 1 <= length <= 512 + paquet = DATA
        if (isIgnore == 0 && (test1 || seqnumPacket != lastSeqAck) && pkt_get_type(p) == PTYPE_DATA && test2) {
            set_window(windowUtil, pkt_get_window(p)); //On recupere la taille de la window du client

            fprintf(stderr,"Received valid PACKET N° %d\n", (int) pkt_get_seqnum(p));

            //Bon type de paquet mais tronque
            if (pkt_get_tr(p) == 1) {

                // Creation du nouveau paquet

                fprintf(stderr,"\tThis packet was truncated\n");

                pkt_t *newPkt = pkt_new();
                if (newPkt == NULL) {
                    if (fp != NULL) fclose(fp);
                    return EXIT_FAILURE;
                }

                pkt_set_type(newPkt, PTYPE_NACK);
                pkt_set_tr(newPkt, 1);
                pkt_set_seqnum(newPkt, pkt_get_seqnum(p));
                pkt_set_window(newPkt, (const uint8_t) get_window_server(windowUtil));
                pkt_set_timestamp(newPkt, (const uint32_t) time(NULL));

                char buff[MAX_PACKET_RECEIVED_SIZE_FOR_SERVER];

                size_t len = MAX_PACKET_RECEIVED_SIZE_FOR_SERVER; // la taille de notre buffer pour répondre

                // On encode la nouveau paquet

                pkt_status_code err;
                if ((err = pkt_encode(p, buff, &len)) != PKT_OK) {
                    fprintf(stderr, "\tError when encoding the response packet - error %d\n", err);
                    if (fp != NULL) fclose(fp);
                    return EXIT_FAILURE;
                }

                // On envoie le paquet au client

                if (sendto(socketFileDescriptor, buff, len, 0, (struct sockaddr *) &rval, fromsize) < 0) {
                    fprintf(stderr, "\tError when sending the response packet\n"); //Erreur lors de l'envoi des donnees
                    if (fp != NULL) fclose(fp);
                    return EXIT_FAILURE;
                }

                //Bon type de paquet et pas tronque
            } else if (pkt_get_tr(p) == 0 && isInSlidingWindow(windowUtil, pkt_get_seqnum(p)) == 1) {

                //On cree le nouveau paquet
                fprintf(stderr,"\tThis packet is not truncated\n");
                pkt_t *newPkt = pkt_new();
                if (newPkt == NULL) {
                    if (fp != NULL) fclose(fp);
                    return EXIT_FAILURE;
                }

                if ((get_lastReceivedSeqNum(windowUtil) + 1) % MAX_STORED_PACKAGES == seqnumPacket) {
                    fprintf(stderr,"\tDistribution of packets payload to stdout\n");
                    set_lastReceivedSeqNum(windowUtil, seqnumPacket); //On incremente le numero de sequence valide
                    printer(windowUtil, p); // Permet d'afficher sur stdout tous les paquets avec les numeros de sequence valide
                } else {

                    add_window_packet(windowUtil, p); //On stocke le paquet au frais
                }

                pkt_set_type(newPkt, PTYPE_ACK);
                pkt_set_tr(newPkt, 0);
                pkt_set_seqnum(newPkt, (const uint8_t) (get_lastReceivedSeqNum(windowUtil) % MAX_STORED_PACKAGES));
                pkt_set_window(newPkt, (const uint8_t) get_window_server(windowUtil));
                pkt_set_timestamp(newPkt, (const uint32_t) time(NULL));

                // On encode la nouveau paquet

                char buff[lengthReceivedPacket];
                size_t len = 0;


                pkt_status_code err;
                if ((err = pkt_encode(p, buff, &len)) != PKT_OK) {
                    fprintf(stderr, "encode error %d :\n", err);
                    if (fp != NULL) fclose(fp);
                    return EXIT_FAILURE;
                }

                // On envoie le paquet au client

                if (sendto(socketFileDescriptor, buff, len, 0, (struct sockaddr *) &rval, fromsize) < 0) {
                    fprintf(stderr, "Sending error"); //Erreur lors de l'envoi des donnees
                    if (fp != NULL) fclose(fp);
                    return EXIT_FAILURE;
                }
            }

        }

        pkt_del(p);
        free(p);

    }

    del_window_util(windowUtil);

    if (fp != NULL) fclose(fp);
    return EXIT_SUCCESS;
}