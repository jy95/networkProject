//
// Created by jy95 on 05/10/2017.
//

#include <stddef.h> /* size_t */
#include <inttypes.h> /* unit8_t */

#ifndef NETWORKPROJECT_PACKET_H
#define NETWORKPROJECT_PACKET_H

// STRUCT

// typedef pour définir un type
typedef struct packet {
    struct header {
        // explicit packing
        struct __attribute__((__packed__)) bitFields {
            unsigned int type:2;
            unsigned int trFlag:1; //tr flag
            unsigned int window:5; //WINDOW
        } bitFields;
        uint8_t seqNum; // numéro de séquence
        uint16_t length; // la longueur du packet , warning endian
        unit32_t timestamp; // En théorie, time_t de time.h donne aussi 32 bits ; par sécurité unit32_t
        unit32_t CRC1;
    } header;
    unsigned char * payload; // payload à malloc plus tard
    unit32_t CRC2; // 2e CRC si payload
} packet;

// CONSTANTS

/* Taille maximale permise pour le payload */
// récupéré de l'exo facultatif "Format des segments du projet de groupe"
#define MAX_PAYLOAD_SIZE 512

/* Taille maximale de Window */
// Vu que c'est du selective repeat : (2 exposant 5) -1
#define MAX_WINDOW_SIZE 31

/* Valeur de retours des fonctions */
// récupéré de l'exo facultatif "Format des segments du projet de groupe"
typedef enum {
    PKT_OK = 0,     /* Le paquet a ete traite avec succes */
    E_TYPE,         /* Erreur liee au champs Type */
    E_TR,           /* Erreur liee au champ TR */
    E_LENGTH,       /* Erreur liee au champs Length  */
    E_CRC,          /* CRC invalide */
    E_WINDOW,       /* Erreur liee au champs Window */
    E_SEQNUM,       /* Numero de sequence invalide */
    E_NOMEM,        /* Pas assez de memoire */
    E_NOHEADER,     /* Le paquet n'a pas de header (trop court) */
    E_UNCONSISTENT, /* Le paquet est incoherent */
} packet_status_code;

/* Types de paquets */
typedef enum {
    PTYPE_DATA = 1,
    PTYPE_ACK = 2,
    PTYPE_NACK = 3,
} ptypes_t;

// FUNCTIONS

/*
 * Decode des donnees recues et cree une nouvelle structure pkt.
 * Le paquet recu est en network byte-order.
 * La fonction verifie que:
 * - Le CRC32 du header recu est le même que celui decode a la fin
 *   du header (en considerant le champ TR a 0)
 * - S'il est present, le CRC32 du payload recu est le meme que celui
 *   decode a la fin du payload
 * - Le type du paquet est valide
 * - La longueur du paquet et le champ TR sont valides et coherents
 *   avec le nombre d'octets recus.
 *
 * @data: L'ensemble d'octets constituant le paquet recu
 * @len: Le nombre de bytes recus
 * @pkt: Une struct packet valide
 * @post: pkt est la representation du paquet recu
 *
 * @return: Un code indiquant si l'operation a reussi ou representant
 *         l'erreur rencontree.
 */
packet_status_code pkt_decode(const char *data, const size_t len, packet *pkt);

/*
 * Encode une struct pkt dans un buffer, prêt a être envoye sur le reseau
 * (c-a-d en network byte-order), incluant le CRC32 du header et
 * eventuellement le CRC32 du payload si celui-ci est non nul.
 *
 * @packet: La structure a encoder
 * @buf: Le buffer dans lequel la structure sera encodee
 * @len: La taille disponible dans le buffer
 * @len-POST: Le nombre de d'octets ecrit dans le buffer
 * @return: Un code indiquant si l'operation a reussi ou E_NOMEM si
 *         le buffer est trop petit.
 */
packet_status_code pkt_encode(const packet*, char *buf, size_t *len);

/* Accesseurs pour les champs toujours presents du paquet.
 * Les valeurs renvoyees sont toutes dans l'endianness native
 * de la machine!
 */
ptypes_t pkt_get_type     (const packet*);
uint8_t  pkt_get_tr       (const packet*);
uint8_t  pkt_get_window   (const packet*);
uint8_t  pkt_get_seqnum   (const packet*);
uint16_t pkt_get_length   (const packet*);
uint32_t pkt_get_timestamp(const packet*);
uint32_t pkt_get_crc1     (const packet*);
/* Renvoie un pointeur vers le payload du paquet, ou NULL s'il n'y
 * en a pas.
 */
const char* pkt_get_payload(const packet*);
/* Renvoie le CRC2 dans l'endianness native de la machine. Si
 * ce field n'est pas present, retourne 0.
 */
uint32_t pkt_get_crc2(const packet*);

/* Setters pour les champs obligatoires du paquet. Si les valeurs
 * fournies ne sont pas dans les limites acceptables, les fonctions
 * doivent renvoyer un code d'erreur adapte.
 * Les valeurs fournies sont dans l'endianness native de la machine!
 */
packet_status_code pkt_set_type     (packet*, const ptypes_t type);
packet_status_code pkt_set_tr       (packet*, const uint8_t tr);
packet_status_code pkt_set_window   (packet*, const uint8_t window);
packet_status_code pkt_set_seqnum   (packet*, const uint8_t seqnum);
packet_status_code pkt_set_length   (packet*, const uint16_t length);
packet_status_code pkt_set_timestamp(packet*, const uint32_t timestamp);
packet_status_code pkt_set_crc1     (packet*, const uint32_t crc1);
/* Defini la valeur du champs payload du paquet.
 * @data: Une succession d'octets representants le payload
 * @length: Le nombre d'octets composant le payload
 * @POST: pkt_get_length(pkt) == length */
packet_status_code pkt_set_payload(packet*,
                                const char *data,
                                const uint16_t length);
/* Setter pour CRC2. Les valeurs fournies sont dans l'endianness
 * native de la machine!
 */
packet_status_code pkt_set_crc2(packet*, const uint32_t crc2);

#endif //NETWORKPROJECT_PACKET_H
