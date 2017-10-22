//
// Created by Alexandre Dewit on 12/10/17.
//
#include "../src/paquet/packet_interface.h"
#include "../src/server_window/server_window_util.h"
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

struct pkt *p;
struct pkt *packet;
window_util_t *windowUtil;


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
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(p), "hello world");
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
    struct pkt *packetTest = pkt_new();
    pkt_set_tr(packetTest, 0);
    pkt_set_length(packetTest, 11);
    CU_ASSERT_EQUAL(pkt_set_payload(packetTest, "hello world", 11), PKT_OK);
    pkt_del(packetTest);
    free(packetTest);
}

// @new_window_util:test_window_init => [Nouveau window_util : ne doit pas etre null apres initialisation]
void test_window_init(void) {
    window_util_t *windowUtilTest = new_window_util();
    CU_ASSERT_NOT_EQUAL(windowUtilTest, NULL);
}

// @isInSlidingWindow:test_window_IsInWindow => [Le numero de sequence du paquet doit etre present dans la window]
void test_window_IsInWindow(void) {
    CU_ASSERT_EQUAL(isInSlidingWindow(windowUtil, pkt_get_seqnum(p)), 1);
}

// @get_lastReceivedSeqNum:test_window_get_lastSeqnum => [Le dernier numero de sequence recu == 21]
void test_window_get_lastSeqnum(void) {
    set_lastReceivedSeqNum(windowUtil, 21);
    CU_ASSERT_EQUAL(get_lastReceivedSeqNum(windowUtil), 21);
}

// @get_window_server:test_window_get_window_server => [la taille du tableau == MAX_WINDOW_SIZE]
void test_window_get_window_server(void) {
    CU_ASSERT_EQUAL(get_window_server(windowUtil), MAX_WINDOW_SIZE);
}

// @set_seqnum_window:test_window_add_seqnum => [On ajoute le paquet et il peut etre ajouté, donc 1 ; on essaye une deuxième fois alors qu'il est deja stocke : 3]
void test_window_add_seqnum(void) {
    CU_ASSERT_EQUAL(set_seqnum_window(windowUtil, p), 1);
    CU_ASSERT_EQUAL(set_seqnum_window(windowUtil, p), 3);
}

// @set_seqnum_window:test_window_add_seqnum_NotInWindow => [Le seqnum du paquet n'est pas dans la window, donc : 2]
void test_window_add_seqnum_NotInWindow(void) {
    pkt_t *pkt1 = pkt_new();
    if (!pkt1) {
        CU_FAIL("MALLOC FAIL FOR test");
    } else {
        pkt_set_seqnum(pkt1, 2);
        CU_ASSERT_EQUAL(set_seqnum_window(windowUtil, pkt1), 2);
        pkt_del(pkt1);
    }
}

// @set_seqnum_window:test_window_add_seqnum_init => [On met le paquet avec seqnum = 0 en stockage puis on applique la méthode printer. Result = dernier seqnum valide = 0 + paquet plus present dans la liste des paquets]
void test_window_add_seqnum_init(void) {
    pkt_t *pkt1 = pkt_new();
    window_util_t *window_util1 = new_window_util();
    if (!pkt1 || !window_util1) {
        CU_FAIL("MALLOC FAIL FOR test");
    } else {
        set_window(window_util1, 5);

        pkt_set_payload(pkt1, "Hello world", 11);
        pkt_set_seqnum(pkt1, 0);
        pkt_set_length(pkt1, 11);

        CU_ASSERT_EQUAL(set_seqnum_window(window_util1, pkt1), 1);
        CU_ASSERT_EQUAL(isPresent_seqnum_window(window_util1, 0), 1);

        printer(window_util1, pkt1);

        CU_ASSERT_EQUAL(isPresent_seqnum_window(window_util1, 0), 0);
        CU_ASSERT_EQUAL(get_lastReceivedSeqNum(window_util1), 0);


        del_window_util(window_util1);
        pkt_del(pkt1);
    }
}

// @set_seqnum_window:test_pkt_decode => [On met le paquet avec seqnum = 0 en stockage puis on applique la méthode printer. Result = dernier seqnum valide = 0 + paquet plus present dans la liste des paquets]
void test_pkt_decode(void) {

    pkt_t *paquetTest = pkt_new();

    if (!paquetTest) {
        CU_FAIL("MALLOC FAIL FOR test");
    }

    pkt_set_type(paquetTest, PTYPE_DATA);
    pkt_set_tr(paquetTest, 0);
    pkt_set_window(paquetTest, 5);
    pkt_set_seqnum(paquetTest, 0);
    pkt_set_length(paquetTest, 11);
    pkt_set_timestamp(paquetTest, 0);
    pkt_set_payload(paquetTest, "hello world", 11);

    char string[27];
    size_t len = 27;
    CU_ASSERT_EQUAL(pkt_encode(paquetTest, string, &len), PKT_OK);


    pkt_t *newPaquetTest = pkt_new();

    if (!newPaquetTest) {
        CU_FAIL("MALLOC FAIL FOR test");
    }

    CU_ASSERT_EQUAL(pkt_decode(string, len, newPaquetTest), PKT_OK);

    CU_ASSERT_EQUAL(pkt_get_type(paquetTest), pkt_get_type(newPaquetTest));
    CU_ASSERT_EQUAL(pkt_get_tr(paquetTest), pkt_get_tr(newPaquetTest));
    CU_ASSERT_EQUAL(pkt_get_window(paquetTest), pkt_get_window(newPaquetTest));
    CU_ASSERT_EQUAL(pkt_get_seqnum(paquetTest), pkt_get_seqnum(newPaquetTest));
    CU_ASSERT_EQUAL(pkt_get_length(paquetTest), pkt_get_length(newPaquetTest));
    CU_ASSERT_EQUAL(pkt_get_timestamp(paquetTest), pkt_get_timestamp(newPaquetTest));
    CU_ASSERT_STRING_EQUAL(pkt_get_payload(paquetTest), pkt_get_payload(newPaquetTest));


    pkt_del(newPaquetTest);
    pkt_del(paquetTest);

}

int main(void) {

    //Suite de tests pour les getters

    if ((p = malloc(sizeof(pkt_t))) == NULL) {
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
    packet = malloc(sizeof(struct pkt));

    if (packet == NULL) {
        return EXIT_FAILURE;
    }

    //Suite de tests pour la window

    windowUtil = new_window_util();
    if (windowUtil == NULL) return EXIT_FAILURE;

    set_lastReceivedSeqNum(windowUtil, 21);

    set_window(windowUtil, pkt_get_window(p));

    CU_pSuite pSuite = NULL;

    // Initialisation de la suite de tests
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    pSuite = CU_add_suite("TESTS", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
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
        NULL == CU_add_test(pSuite, "test_packet_set_payload", test_packet_set_payload) ||
        NULL == CU_add_test(pSuite, "test_window_init", test_window_init) ||
        NULL == CU_add_test(pSuite, "test_window_get_lastSeqnum", test_window_get_lastSeqnum) ||
        NULL == CU_add_test(pSuite, "test_window_IsInWindow", test_window_IsInWindow) ||
        NULL == CU_add_test(pSuite, "test_window_get_window_server", test_window_get_window_server) ||
        NULL == CU_add_test(pSuite, "test_window_add_seqnum", test_window_add_seqnum) ||
        NULL == CU_add_test(pSuite, "test_window_add_seqnum_NotInWindow", test_window_add_seqnum_NotInWindow) ||
        NULL == CU_add_test(pSuite, "test_window_add_seqnum_init", test_window_add_seqnum_init) ||
        NULL == CU_add_test(pSuite, "test_pkt_decode", test_pkt_decode)) {
        CU_cleanup_registry();
        printf("DIE\n");

        del_window_util(windowUtil);
        pkt_del(p);
        pkt_del(packet);

        return CU_get_error();
    }

    // Exécution des tests et vidage de la mémoire
    CU_basic_run_tests();
    CU_get_run_summary();
    CU_basic_show_failures(CU_get_failure_list());
    CU_cleanup_registry();

    del_window_util(windowUtil);
    pkt_del(p);
    pkt_del(packet);

    return CU_get_error();

}