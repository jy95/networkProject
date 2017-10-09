#include "header/packet.h"
#include <stdio.h>  /* FILE */
#include <stdlib.h> // malloc , etc
#include <string.h>
#include <arpa/inet.h>

// STRUCT
struct header {
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
} head;

// typedef pour définir un type
typedef struct pkt {
    struct header structheader;
    char *payload; // payload à malloc plus tard
    uint32_t CRC2; // 2e CRC si payload
} pkt_t;

pkt_t* pkt_new() {
    pkt_t* pkt = malloc(sizeof(struct pkt));
    if(pkt == NULL) return NULL;
    return pkt;
}

void pkt_del(pkt_t* pkt) {
    if(pkt->payload != NULL) {
        free(pkt->payload);
    }
}
pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt) {

    pkt_status_code packetStatusCode;

    if (len < 12) return E_NOHEADER; //Pas assez de Bytes pour former un header

    struct header structheader;

    memcpy(&structheader, data + 0, 12); //Copie des 12 premiers bytes, qui formeront la structure header

    /*- Verification des donnees et ajout de celles-ci dans le paquet -*/

    packetStatusCode = pkt_set_type(pkt, (const ptypes_t) structheader.bitFields.type);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_tr(pkt, (const uint8_t) structheader.bitFields.trFlag);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_window(pkt, (const uint8_t) structheader.bitFields.window);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_seqnum(pkt, structheader.seqNum);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_length(pkt, structheader.length);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_timestamp(pkt, structheader.timestamp);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_crc1(pkt, structheader.CRC1);
    if (packetStatusCode != PKT_OK) return packetStatusCode;

    /*- Le cas du payload -*/

    uint16_t length = pkt_get_length(pkt);

    if (length > 0 && pkt_get_tr(pkt) == 0) {

        if (len < (size_t) (12 + length))
            return E_UNCONSISTENT; //La longueur annoncee par length n'est pas pas disponible dans data

        const char *payload;

        memcpy(&payload, data + 12, length); //Copie des pkt_get_length(pkt) bytes qui formeront le payload

        packetStatusCode = pkt_set_payload(pkt, payload, length);

        if (packetStatusCode != PKT_OK) return packetStatusCode;

        if (len < (size_t)(12 + length + 4)) return E_UNCONSISTENT; //Pas assez de bytes pour le CRC2

        uint32_t CRC2;

        memcpy(&CRC2, data + 12 + length, 4); //Copie des pkt_get_length(pkt) bytes qui formeront le payload

        packetStatusCode = pkt_set_crc2(pkt, CRC2);

        if (packetStatusCode != PKT_OK) return packetStatusCode;
    }

    return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t *p, char *buf, size_t *len) {
    if (sizeof(p) > *len) return E_NOMEM;

    uint16_t length = p->structheader.length;

    if (length > MAX_PAYLOAD_SIZE) return E_LENGTH;


    memcpy(buf, &((p->structheader).bitFields), sizeof(uint8_t));

    uint8_t segnum = pkt_get_seqnum(p);
    memcpy(buf + 1, &segnum, sizeof(uint8_t));

    uint16_t length_network = pkt_get_length(p);
    memcpy(buf + 2, &length_network, sizeof(uint16_t));

    uint32_t timestamp = pkt_get_timestamp(p);
    memcpy(buf + 4, &timestamp, sizeof(uint32_t));

    uint32_t crc1 = pkt_get_crc1(p);
    memcpy(buf + 8, &crc1, sizeof(uint32_t));

    if (pkt_get_tr(p) == 0 && length > 0) {
        memcpy(buf + 12, pkt_get_payload(p), length);

        uint32_t crc2 = pkt_get_crc2(p);
        memcpy(buf + 12 + length, &crc2, sizeof(uint32_t));
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
    return htons((p->structheader).length);
}

uint32_t pkt_get_timestamp(const pkt_t *p) {
    return (p->structheader).timestamp;
}

uint32_t pkt_get_crc1(const pkt_t *p) {
    return htonl((p->structheader).CRC1);
}

const char *pkt_get_payload(const pkt_t *p) {
    return (const char *) p->payload;
}

uint32_t pkt_get_crc2(const pkt_t *p) {
    return htonl(p->CRC2);
}


pkt_status_code pkt_set_type(pkt_t *p, const ptypes_t type) {
    if (type != 1 && type != 2 && type != 3) return E_TYPE;

    (p->structheader).bitFields.type = type;

    return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *p, const uint8_t tr) {
    if (tr != 1 && tr != 0) return E_TR;

    (p->structheader).bitFields.trFlag = tr;

    return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *p, const uint8_t window) {
    if (window > MAX_WINDOW_SIZE || window < 1) return E_WINDOW;

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

    char *payload = malloc(sizeof(char) * length);

    if (payload == NULL) return E_NOMEM;

    int i = 0;

    while (i < length) {
        *(payload + i) = (unsigned char) *(data + i);
        i++;
    }

    p->payload = payload;

    return PKT_OK;


}

pkt_status_code pkt_set_crc2(pkt_t *p, const uint32_t crc2) {
    p->CRC2 = ntohl(crc2);
    return PKT_OK;
}