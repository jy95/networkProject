#include "packet_interface.h"
#include <stdio.h>  /* FILE */
#include <stdlib.h> // malloc , etc
#include <string.h>
#include <arpa/inet.h>
#include <zlib.h> //crc32

pkt_t *pkt_new() {
    return calloc(1, sizeof(pkt_t));
}

void pkt_del(pkt_t *pkt) {
    if (pkt->payload != NULL) {
        free(pkt->payload);
    }
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt) {

    pkt_status_code packetStatusCode;

    if (len < sizeof(struct header)) return E_NOHEADER; //Pas assez de Bytes pour former un header

    int nbrBytesLus = 0;
    struct bitFields bitFields;
    uint32_t crc1FromData;
    uint32_t crc1Calculated = (uint32_t) crc32(0L, Z_NULL, 0); // init du crc1

    // lecture des bitFields

    memcpy(&bitFields, data, sizeof(struct bitFields));
    nbrBytesLus += sizeof(struct bitFields);

    /*- Verification des donnees et ajout de celles-ci dans le paquet -*/

    packetStatusCode = pkt_set_type(pkt, (const ptypes_t) bitFields.type);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_tr(pkt, (const uint8_t) bitFields.trFlag);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_window(pkt, (const uint8_t) bitFields.window);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    // calcul du crc1
    // pour se faire, temporairement set de flag à 0

    // ajout des bitfields dans le crc1 , avec le tr à 0
    uint8_t bitfieldsForCRC = (pkt_get_type(pkt) << 6) | (0 << 5) | (pkt_get_window(pkt));
    crc1Calculated = (uint32_t) crc32(crc1Calculated, (const Bytef *) &(bitfieldsForCRC), sizeof(uint8_t));

    // lecture du numéro de séquence
    uint8_t seqNum;
    memcpy(&seqNum, data + nbrBytesLus, sizeof(uint8_t));
    nbrBytesLus += sizeof(uint8_t);

    packetStatusCode = pkt_set_seqnum(pkt, (const uint8_t) seqNum);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    // ajout du seqNum dans le crc1
    crc1Calculated = (uint32_t) crc32(crc1Calculated, (const Bytef *) &seqNum, sizeof(uint8_t));

    // lecture de la length
    uint16_t length;
    memcpy(&length, data + nbrBytesLus, sizeof(uint16_t));
    nbrBytesLus += sizeof(uint16_t);

    // on save la length qui a été htons;
    uint16_t length_network = length;

    // on convertit en byte order de notre machine
    length = ntohs(length);

    packetStatusCode = pkt_set_length(pkt, (const uint16_t) length);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    // ajout de length dans le crc1
    crc1Calculated = (uint32_t) crc32(crc1Calculated, (const Bytef *) &length_network, sizeof(uint16_t));

    // lecture du timestamp
    uint32_t timestamp;
    memcpy(&timestamp, data + nbrBytesLus, sizeof(uint32_t));
    nbrBytesLus += sizeof(uint32_t);

    packetStatusCode = pkt_set_timestamp(pkt, (const uint32_t) timestamp);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    // ajout de timestamp dans le crc1
    crc1Calculated = (uint32_t) crc32(crc1Calculated, (const Bytef *) &timestamp, sizeof(uint32_t));

    // Phase critique 1 : check du CRC1

    // lecture du crc1 de data
    memcpy(&crc1FromData, data + nbrBytesLus, sizeof(uint32_t));
    nbrBytesLus += sizeof(uint32_t);
    crc1FromData = ntohl(crc1FromData); // en byte order de notre machine

    // check du crc1
    if ( crc1FromData != crc1Calculated ) return E_CRC;

    packetStatusCode = pkt_set_crc1(pkt, (const uint32_t) crc1Calculated);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    /*- Le cas du payload -*/

    if (length > 0 && pkt_get_tr(pkt) == 0) {

        if ( len < (size_t)(nbrBytesLus + length))
            return E_UNCONSISTENT; //La longueur annoncee par length n'est pas pas disponible dans data

        // on déclare notre payload
        char payload[length];

        memcpy(&payload, data + nbrBytesLus, length); //Copie des pkt_get_length(pkt) bytes qui formeront le payload
        nbrBytesLus += length;
        packetStatusCode = pkt_set_payload(pkt, payload, length);

        if (packetStatusCode != PKT_OK) return packetStatusCode;

        if ( len < (size_t)(nbrBytesLus + sizeof(uint32_t))) return E_UNCONSISTENT; //Pas assez de bytes pour le CRC2

        uint32_t CRC2;

        memcpy(&CRC2, data + nbrBytesLus,
               sizeof(uint32_t)); //Copie des pkt_get_length(pkt) bytes qui formeront le payload

        // calcul du crc2
        uint32_t crc2Calculated = (uint32_t) crc32(0L, Z_NULL, 0); // init du crc2

        CRC2 = ntohl(CRC2); // on convertit pour avoir la valeur en byte order de notre machine
        crc2Calculated = (uint32_t) crc32(crc2Calculated, (const Bytef *) &payload, length);

        if ( CRC2 != crc2Calculated ) return E_CRC;

        packetStatusCode = pkt_set_crc2(pkt, CRC2);

        if (packetStatusCode != PKT_OK) return packetStatusCode;
    }

    return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t *p, char *buf, size_t *len) {
    uint16_t length = pkt_get_length(p);
    size_t totalSize = sizeof(struct header) + sizeof(uint32_t) + length;
    uint32_t crc1 = (uint32_t) crc32(0L, Z_NULL, 0); // init du crc1

    if (totalSize > *len) return E_NOMEM;

    if (length > MAX_PAYLOAD_SIZE) return E_LENGTH;
    // on initialize le nombre inscrits à 0;
    *len = 0;

    // le buffer doit être en network byte-order !!!!

    // On s'occupe des bitfields (type - flag - window)
    memcpy(buf, &(p->structheader).bitFields, sizeof(struct bitFields));

    // le crc1 doit prendre tous les bitfields , sauf TR qui doit être à 0

    uint8_t bitfieldsForCRC = (pkt_get_type(p) << 6) | (0 << 5) | (pkt_get_window(p));
    crc1 = (uint32_t) crc32(crc1, (const Bytef *) &(bitfieldsForCRC), sizeof(uint8_t));

    // on augmente la taille
    *len += sizeof(uint8_t);

    uint8_t segnum = pkt_get_seqnum(p);
    // au lieu de faire buf +1, utilisons la length qu'on incrémente
    memcpy(buf + *len, &segnum, sizeof(uint8_t));
    // le crc1 doit prendre aussi le numéro de séquence
    crc1 = (uint32_t) crc32(crc1, (const Bytef *) &segnum, sizeof(uint8_t));

    // on augmente la taille
    *len += sizeof(uint8_t);

    uint16_t length_network = htons(pkt_get_length(p));
    memcpy(buf + *len, &length_network, sizeof(uint16_t));
    // le crc1 doit aussi prendre la length
    crc1 = (uint32_t) crc32(crc1, (const Bytef *) &length_network, sizeof(uint16_t));

    // on augmente la taille
    *len += sizeof(uint16_t);

    uint32_t timestamp = pkt_get_timestamp(p);
    memcpy(buf + *len, &timestamp, sizeof(uint32_t));
    // le crc1 doit aussi prendre le timestamp
    crc1 = (uint32_t) crc32(crc1, (const Bytef *) &timestamp, sizeof(uint32_t));

    // on augmente la taille
    *len += sizeof(uint32_t);

    // la version Network byte order du crc1
    uint32_t crc1NBO = htonl(crc1);
    memcpy(buf + *len, &crc1NBO, sizeof(uint32_t));

    // on augmente la taille
    *len += sizeof(uint32_t);

    if (pkt_get_tr(p) == 0 && length > 0) {
        memcpy(buf + *len, pkt_get_payload(p), length);

        // on augmente la taille
        *len += length;

        // calcul du crc2
        uint32_t crc2 = (uint32_t) crc32(0L, Z_NULL, 0); // init du crc2
        crc2 = (uint32_t) crc32(crc2, (const Bytef *) pkt_get_payload(p), length);

        uint32_t crc2NBO = htonl(crc2);
        // on rajoute le crc2
        memcpy(buf + *len, &crc2NBO, sizeof(uint32_t));

        // on augmente la taille
        *len += sizeof(uint32_t);

    }
    return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t *p) {
    return (ptypes_t) (p->structheader).bitFields.type;
}

uint8_t pkt_get_tr(const pkt_t *p) {
    return (uint8_t) (p->structheader).bitFields.trFlag;
}

uint8_t pkt_get_window(const pkt_t *p) {
    return (uint8_t) (p->structheader).bitFields.window;
}

uint8_t pkt_get_seqnum(const pkt_t *p) {
    return (p->structheader).seqNum;
}

uint16_t pkt_get_length(const pkt_t *p) {
    return (p->structheader).length;
}

uint32_t pkt_get_timestamp(const pkt_t *p) {
    return (p->structheader).timestamp;
}

uint32_t pkt_get_crc1(const pkt_t *p) {
    return (p->structheader).CRC1;
}

const char *pkt_get_payload(const pkt_t *p) {
    return (const char *) p->payload;
}

/* Renvoie le CRC2 dans l'endianness native de la machine. Si
 * ce field n'est pas present, retourne 0.
 */
uint32_t pkt_get_crc2(const pkt_t *p) {
    if (pkt_get_tr(p) == 0 && pkt_get_length(p) > 0) {
        return p->CRC2;
    }
    return 0;
}


pkt_status_code pkt_set_type(pkt_t *p, const ptypes_t type) {
    if (type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK) return E_TYPE;

    (p->structheader).bitFields.type = type;

    return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *p, const uint8_t tr) {
    if (tr != 1 && tr != 0) return E_TR;

    (p->structheader).bitFields.trFlag = tr;

    return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *p, const uint8_t window) {
    if ( window > MAX_PACKET_WINDOW_SIZE || window < 1 ) return E_WINDOW;

    (p->structheader).bitFields.window = window;

    return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *p, const uint8_t seqnum) {

    (p->structheader).seqNum = seqnum;

    return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *p, const uint16_t length) {
    if (length > MAX_PAYLOAD_SIZE) return E_LENGTH;

    (p->structheader).length = length;

    return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *p, const uint32_t timestamp) {
    (p->structheader).timestamp = timestamp;
    return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *p, const uint32_t crc1) {
    (p->structheader).CRC1 = crc1;
    return PKT_OK;
}


pkt_status_code pkt_set_payload(pkt_t *p, const char *data, const uint16_t length) {
    if (length > MAX_PAYLOAD_SIZE) return E_NOMEM;

    // directement faire un malloc du pointeur du payload
    if ((p->payload = malloc(length * sizeof(unsigned char))) == NULL) {
        return E_NOMEM;
    }

    // on peut stocker notre payload
    memcpy(p->payload, data, length);

    // on doit setter la length qu'on vient de copier
    return pkt_set_length(p, length);
}

pkt_status_code pkt_set_crc2(pkt_t *p, const uint32_t crc2) {
    p->CRC2 = crc2;
    return PKT_OK;
}

