#include "headers/packet.h"
#include <stdio.h>  /* FILE */
#include <stdlib.h> // malloc , etc
#include <string.h>


packet_status_code pkt_decode(const char *data, const size_t len, packet *pkt){
    // TODO
}

packet_status_code pkt_encode(const packet*, char *buf, size_t *len){
    // TODO
}

ptypes_t pkt_get_type     (const packet* p){
    return (p->header).bitFields.type;
}
uint8_t  pkt_get_tr       (const packet* p){
    return (p->header).bitFields.trFlag;
}
uint8_t  pkt_get_window   (const packet* p){
    return (p->header).bitFields.window;
}
uint8_t  pkt_get_seqnum   (const packet* p){
    return (p->header).seqNum;
}
uint16_t pkt_get_length   (const packet* p){
    return (p->header).length;
}
uint32_t pkt_get_timestamp(const packet* p){
    return (p->header).timestamp;
}
uint32_t pkt_get_crc1     (const packet* p){
    return (p->header).CRC1;
}

const char* pkt_get_payload(const packet* p) {
    return p->payload;
}

uint32_t pkt_get_crc2(const packet* p) {
    return p->CRC2;
}


packet_status_code pkt_set_type     (packet*, const ptypes_t type) {
    // TODO
}
packet_status_code pkt_set_tr       (packet*, const uint8_t tr) {
    // TODO
}
packet_status_code pkt_set_window   (packet*, const uint8_t window){
    // TODO
}
packet_status_code pkt_set_seqnum   (packet*, const uint8_t seqnum) {
    // TODO
}
packet_status_code pkt_set_length   (packet*, const uint16_t length) {
    // TODO
}

packet_status_code pkt_set_timestamp(packet*, const uint32_t timestamp) {
    // TODO
}

packet_status_code pkt_set_crc1     (packet*, const uint32_t crc1) {
    // TODO
}


packet_status_code pkt_set_payload(packet*,
                                   const char *data,
                                   const uint16_t length) {
    // TODO
}

packet_status_code pkt_set_crc2(packet*, const uint32_t crc2) {
    // TODO
}