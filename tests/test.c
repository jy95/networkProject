//
// Created by Alexandre Dewit on 12/10/17.
//
#include "../src/paquet/packet_interface.h"
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

pkt_t *p;
pkt_t *packet;


// @pkt_get_type:test_packet_get_type[type == PTYPE_DATA]
void test_packet_get_type(void) {
    CU_ASSERT_EQUAL(pkt_get_type(p), PTYPE_DATA);
}

// @pkt_get_tr:test_packet_get_tr => [tr == 0]
void test_packet_get_tr(void) {
    CU_ASSERT_EQUAL(pkt_get_tr(p), 0);
}

// @pkt_get_window:test_packet_get_window => [window == 1]
void test_packet_get_window(void) {
    CU_ASSERT_EQUAL(pkt_get_window(p), 1);
}

// @pkt_get_seqnum:test_packet_get_seqnum => [seqnum == 23]
void test_packet_get_seqnum(void) {
    CU_ASSERT_EQUAL(pkt_get_seqnum(p), 23);
}

// @pkt_get_length:test_packet_get_length => [length == 11]
void test_packet_get_length(void) {
    CU_ASSERT_EQUAL(pkt_get_length(p), 11);
}

// @pkt_get_timestamp:test_packet_get_timestamp => [timestamp == 2]
void test_packet_get_timestamp(void) {
    CU_ASSERT_EQUAL(pkt_get_timestamp(p), 2);
}

// @pkt_get_crc1:test_packet_get_crc1 => [crc1 == 10]
void test_packet_get_crc1(void) {
    CU_ASSERT_EQUAL(pkt_get_crc1(p), 10);
}

// @pkt_get_crc2:test_packet_get_crc2 => [crc2 == 12]
void test_packet_get_crc2(void) {
    CU_ASSERT_EQUAL(pkt_get_crc2(p), 12);
}

// @pkt_get_payload:test_packet_get_payload => [payload == "hello world"]
void test_packet_get_payload(void) {
    CU_ASSERT_EQUAL(pkt_get_payload(p), "hello world");
}

// @pkt_set_type:test_packet_set_type => [set de PTYPE_DATA ; doit renvoyer un PKT_OK]
void test_packet_set_type(void) {
    CU_ASSERT_EQUAL(pkt_set_type(p, PTYPE_DATA), PKT_OK);
}

// @pkt_get_length:test_packet_set_type_error => [set de 0 ; doit renvoyer un E_TYPE]
void test_packet_set_type_error(void) {
    CU_ASSERT_EQUAL(pkt_set_type(p, (const ptypes_t) 0), E_TYPE);
}

// @pkt_set_tr:test_packet_set_tr => [set de 0 ; doit renvoyer un PKT_OK]
void test_packet_set_tr(void) {
    CU_ASSERT_EQUAL(pkt_set_tr(p, 0), PKT_OK);
}

// @pkt_set_window:test_packet_set_window => [set de 1 ; doit renvoyer un PKT_OK]
void test_packet_set_window(void) {
    CU_ASSERT_EQUAL(pkt_set_window(p, 1), PKT_OK);
}

// @pkt_set_seqnum:test_packet_set_seqnum => [set de 23 ; doit renvoyer un PKT_OK]
void test_packet_set_seqnum(void) {
    CU_ASSERT_EQUAL(pkt_set_seqnum(p, 23), PKT_OK);
}

// @pkt_set_length:test_packet_set_length => [set de 11 ; doit renvoyer un PKT_OK]
void test_packet_set_length(void) {
    CU_ASSERT_EQUAL(pkt_set_length(p, 11), PKT_OK);
}

// @pkt_set_timestamp:test_packet_set_timestamp => [set de 2 ; doit renvoyer un PKT_OK]
void test_packet_set_timestamp(void) {
    CU_ASSERT_EQUAL(pkt_set_timestamp(p, 2), PKT_OK);
}

// @pkt_set_crc1:test_packet_set_crc1 => [set de 10 ; doit renvoyer un PKT_OK]
void test_packet_set_crc1(void) {
    CU_ASSERT_EQUAL(pkt_set_crc1(p, 10), PKT_OK);
}

// @pkt_set_crc2:test_packet_set_crc2 => [set de 12 ; doit renvoyer un PKT_OK]
void test_packet_set_crc2(void) {
    CU_ASSERT_EQUAL(pkt_set_crc2(p, 12), PKT_OK);
}

// @pkt_get_payload:test_packet_set_payload => [set de "hello world" ; doit renvoyer un PKT_OK]
void test_packet_set_payload(void) {
    pkt_t *packetTest = pkt_new();
    pkt_set_tr(packetTest, 0);
    pkt_set_length(packetTest, 11);
    CU_ASSERT_EQUAL(pkt_set_payload(packetTest, "hello world", 11), PKT_OK);
    pkt_del(packetTest);
    free(packetTest);
}

int main() {

    //Suite de tests pour les getters

    p = malloc(sizeof(pkt_t));

    if (p == NULL) {
        return EXIT_FAILURE;
    }

    pkt_set_type(p, PTYPE_DATA);
    pkt_set_tr(p, 0);
    pkt_set_window(p, 1);
    pkt_set_seqnum(p, 23);
    pkt_set_length(p, 11);
    pkt_set_timestamp(p, 2);
    pkt_set_crc1(p, 10);
    pkt_set_crc2(p, 12);
    pkt_set_payload(p, "hello world", 11);

    //Suite de tests pour les setters
    packet = malloc(sizeof(pkt_t));

    if (packet == NULL) {
        return EXIT_FAILURE;
    }

    CU_pSuite pSuite = NULL;

    // Initialisation de la suite de tests
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    // Ajout des tests à la suite
    if (NULL == CU_add_test(pSuite, "test_packet_get_type", test_packet_get_type) ||
        NULL == CU_add_test(pSuite, "test_packet_get_tr", test_packet_get_tr) ||
        NULL == CU_add_test(pSuite, "test_packet_get_window", test_packet_get_window) ||
        NULL == CU_add_test(pSuite, "test_packet_get_seqnum", test_packet_get_seqnum) ||
        NULL == CU_add_test(pSuite, "test_packet_get_length", test_packet_get_length) ||
        NULL == CU_add_test(pSuite, "test_packet_get_timestamp", test_packet_get_timestamp) ||
        NULL == CU_add_test(pSuite, "test_packet_get_crc1", test_packet_get_crc1) ||
        NULL == CU_add_test(pSuite, "test_packet_get_crc2", test_packet_get_crc2) ||
        NULL == CU_add_test(pSuite, "test_packet_get_payload", test_packet_get_payload) ||
        NULL == CU_add_test(pSuite, "test_packet_set_type", test_packet_set_type) ||
        NULL == CU_add_test(pSuite, "test_packet_set_type_error", test_packet_set_type_error) ||
        NULL == CU_add_test(pSuite, "test_packet_set_tr", test_packet_set_tr) ||
        NULL == CU_add_test(pSuite, "test_packet_set_window", test_packet_set_window) ||
        NULL == CU_add_test(pSuite, "test_packet_set_seqnum", test_packet_set_seqnum) ||
        NULL == CU_add_test(pSuite, "test_packet_set_length", test_packet_set_length) ||
        NULL == CU_add_test(pSuite, "test_packet_set_timestamp", test_packet_set_timestamp) ||
        NULL == CU_add_test(pSuite, "test_packet_set_crc1", test_packet_set_crc1) ||
        NULL == CU_add_test(pSuite, "test_packet_set_crc2", test_packet_set_crc2) ||
        NULL == CU_add_test(pSuite, "test_packet_set_payload", test_packet_set_payload)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Exécution des tests et vidage de la mémoire
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();

}