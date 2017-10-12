#include <stddef.h> /* size_t */
#include <inttypes.h> /* unit8_t */

#ifndef NETWORKPROJECT_PACKET_H
#define NETWORKPROJECT_PACKET_H

// typedef pour définir un type
// STRUCT
struct __attribute__((__packed__)) header {
    // explicit packing
    struct __attribute__((__packed__)) bitFields {
        unsigned int type:2;
        unsigned int trFlag:1; //tr flag
        unsigned int window:5; //WINDOW
    } bitFields;
    uint8_t seqNum; // numéro de séquence
    uint16_t length; // la longueur du packet , warning endian
    uint32_t timestamp; // En théorie, time_t de time.h donne aussi 32 bits ; par sécurité uint32_t
    uint32_t CRC1;
};

// typedef pour définir un type
struct __attribute__((__packed__)) pkt {
    struct header structheader;
    uint32_t CRC2; // 2e CRC si payload
    unsigned char *payload; // payload à malloc plus tard
};
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
} pkt_status_code;

/* Types de paquets */
typedef enum {
    PTYPE_DATA = 1,
    PTYPE_ACK = 2,
    PTYPE_NACK = 3,
} ptypes_t;

// FUNCTIONS

/* Alloue et initialise une struct pkt
 * @return: NULL en cas d'erreur */
struct pkt* pkt_new();
/* Libere le pointeur vers la struct pkt, ainsi que toutes les
 * ressources associees
 */
void pkt_del(struct pkt*);

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
pkt_status_code pkt_decode(const char *data, const size_t len, struct pkt *pkt);

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
pkt_status_code pkt_encode(const struct pkt*, char *buf, size_t *len);

/* Accesseurs pour les champs toujours presents du paquet.
 * Les valeurs renvoyees sont toutes dans l'endianness native
 * de la machine!
 */
ptypes_t pkt_get_type     (const struct pkt*);
uint8_t  pkt_get_tr       (const struct pkt*);
uint8_t  pkt_get_window   (const struct pkt*);
uint8_t  pkt_get_seqnum   (const struct pkt*);
uint16_t pkt_get_length   (const struct pkt*);
uint32_t pkt_get_timestamp(const struct pkt*);
uint32_t pkt_get_crc1     (const struct pkt*);
/* Renvoie un pointeur vers le payload du paquet, ou NULL s'il n'y
 * en a pas.
 */
const char* pkt_get_payload(const struct pkt*);
/* Renvoie le CRC2 dans l'endianness native de la machine. Si
 * ce field n'est pas present, retourne 0.
 */
uint32_t pkt_get_crc2(const struct pkt*);

/* Setters pour les champs obligatoires du paquet. Si les valeurs
 * fournies ne sont pas dans les limites acceptables, les fonctions
 * doivent renvoyer un code d'erreur adapte.
 * Les valeurs fournies sont dans l'endianness native de la machine!
 */
pkt_status_code pkt_set_type     (struct pkt*, const ptypes_t type);
pkt_status_code pkt_set_tr       (struct pkt*, const uint8_t tr);
pkt_status_code pkt_set_window   (struct pkt*, const uint8_t window);
pkt_status_code pkt_set_seqnum   (struct pkt*, const uint8_t seqnum);
pkt_status_code pkt_set_length   (struct pkt*, const uint16_t length);
pkt_status_code pkt_set_timestamp(struct pkt*, const uint32_t timestamp);
pkt_status_code pkt_set_crc1     (struct pkt*, const uint32_t crc1);
/* Defini la valeur du champs payload du paquet.
 * @data: Une succession d'octets representants le payload
 * @length: Le nombre d'octets composant le payload
 * @POST: pkt_get_length(pkt) == length */
pkt_status_code pkt_set_payload(struct pkt*,
                                   const char *data,
                                   const uint16_t length);
/* Setter pour CRC2. Les valeurs fournies sont dans l'endianness
 * native de la machine!
 */
pkt_status_code pkt_set_crc2(struct pkt*, const uint32_t crc2);

#endif //NETWORKPROJECT_PACKET_H
