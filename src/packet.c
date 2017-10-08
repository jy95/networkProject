#include "header/packet.h"
#include <stdio.h>  /* FILE */
#include <stdlib.h> // malloc , etc
#include <string.h>


packet_status_code pkt_decode(const char *data, const size_t len, packet *pkt) {

    packet_status_code packetStatusCode;

    if (len < 12) return E_NOHEADER; //Pas assez de Bytes pour former un header

    struct header header;

    memcpy(&header, data + 0, 12); //Copie des 12 premiers bytes, qui formeront la structure header

    /*- Verification des donnees et ajout de celles-ci dans le paquet -*/

    packetStatusCode = pkt_set_type(pkt, (const ptypes_t) header.bitFields.type);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_tr(pkt, (const uint8_t) header.bitFields.trFlag);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_window(pkt, (const uint8_t) header.bitFields.window);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_seqnum(pkt, header.seqNum);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_length(pkt, header.length);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_timestamp(pkt, header.timestamp);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    packetStatusCode = pkt_set_crc1(pkt, header.CRC1);

    if (packetStatusCode != PKT_OK) return packetStatusCode;

    /*- Le cas du payload -*/

    uint16_t length = pkt_get_length(pkt);

    if (length > 0 && pkt_get_tr(pkt) == 0) {

        if (len < 12 + length)
            return E_UNCONSISTENT; //La longueur annoncee par length n'est pas pas disponible dans data

        const char *payload;

        memcpy(&payload, data + 12, length); //Copie des pkt_get_length(pkt) bytes qui formeront le payload

        packetStatusCode = pkt_set_payload(pkt, payload, length);

        if (packetStatusCode != PKT_OK) return packetStatusCode;

        if (len < 12 + length + 4) return E_UNCONSISTENT; //Pas assez de bytes pour le CRC2

        uint32_t CRC2;

        memcpy(&CRC2, data + 12 + length, 4); //Copie des pkt_get_length(pkt) bytes qui formeront le payload

        packetStatusCode = pkt_set_crc2(pkt, CRC2);

        if (packetStatusCode != PKT_OK) return packetStatusCode;
    }

    return PKT_OK;
}

packet_status_code pkt_encode(const packet *p, char *buf, size_t *len) {
    // TODO
}

ptypes_t pkt_get_type(const packet *p) {
    return (ptypes_t) (p->header).bitFields.type;
}

uint8_t pkt_get_tr(const packet *p) {
    return (uint8_t) (p->header).bitFields.trFlag;
}

uint8_t pkt_get_window(const packet *p) {
    return (uint8_t) (p->header).bitFields.window;
}

uint8_t pkt_get_seqnum(const packet *p) {
    return (p->header).seqNum;
}

uint16_t pkt_get_length(const packet *p) {
    return htons((p->header).length);
}

uint32_t pkt_get_timestamp(const packet *p) {
    return (p->header).timestamp;
}

uint32_t pkt_get_crc1(const packet *p) {
    return htonl((p->header).CRC1);
}

const char *pkt_get_payload(const packet *p) {
    return (const char *) p->payload;
}

uint32_t pkt_get_crc2(const packet *p) {
    return htonl(p->CRC2);
}


packet_status_code pkt_set_type(packet *p, const ptypes_t type) {
    if (type != 1 && type != 2 && type != 3) return E_TYPE;

    (p->header).bitFields.type = type;

    return PKT_OK;
}

packet_status_code pkt_set_tr(packet *p, const uint8_t tr) {
    if (tr != 1 && tr != 0) return E_TR;

    (p->header).bitFields.trFlag = tr;

    return PKT_OK;
}

packet_status_code pkt_set_window(packet *p, const uint8_t window) {
    if (window > MAX_WINDOW_SIZE || window < 1) return E_WINDOW;

    (p->header).bitFields.window = window;

    return PKT_OK;
}

packet_status_code pkt_set_seqnum(packet *p, const uint8_t seqnum) {

    (p->header).seqNum = seqnum;

    return PKT_OK;
}

packet_status_code pkt_set_length(packet *p, const uint16_t length) {
    if (length > MAX_PAYLOAD_SIZE) return E_LENGTH;


    (p->header).length = length;

    return PKT_OK;
}

packet_status_code pkt_set_timestamp(packet *p, const uint32_t timestamp) {
    (p->header).timestamp = timestamp;
    return PKT_OK;
}

packet_status_code pkt_set_crc1(packet *p, const uint32_t crc1) {
    (p->header).CRC1 = crc1;
    return PKT_OK;
}


packet_status_code pkt_set_payload(packet *p, const char *data, const uint16_t length) {
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

packet_status_code pkt_set_crc2(packet *p, const uint32_t crc2) {
    p->CRC2 = ntohl(crc2);
    return PKT_OK;
}