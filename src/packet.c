#include "header/packet.h"
#include <stdio.h>  /* FILE */
#include <stdlib.h> // malloc , etc
#include <string.h>


packet_status_code pkt_decode(const char *data, const size_t len, packet *pkt) {
    // TODO
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
    return (p->header).length;
}

uint32_t pkt_get_timestamp(const packet *p) {
    return (p->header).timestamp;
}

uint32_t pkt_get_crc1(const packet *p) {
    return (p->header).CRC1;
}

const char *pkt_get_payload(const packet *p) {
    return (const char *) p->payload;
}

uint32_t pkt_get_crc2(const packet *p) {
    return p->CRC2;
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