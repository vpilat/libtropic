/**
 * @file lt_test_mock_hardware_fail.c
 * @brief Test HARDWARE_FAIL L3 Result handling.
 * @copyright Copyright (c) 2020-2025 Tropic Square s.r.o.
 *
 * @license For the license see file LICENSE.txt file in the root directory of this source tree.
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "libtropic.h"
#include "libtropic_common.h"
#include "libtropic_functional_tests.h"
#include "libtropic_l2.h"
#include "libtropic_logging.h"
#include "libtropic_macros.h"
#include "libtropic_port_mock.h"
#include "lt_crc16.h"
#include "lt_functional_mock_tests.h"
#include "lt_l1.h"
#include "lt_l2_api_structs.h"
#include "lt_l2_frame_check.h"
#include "lt_mock_helpers.h"
#include "lt_test_common.h"
#include "lt_port_wrap.h"
#include "lt_l3_process.h"

int lt_test_mock_hardware_fail(lt_handle_t *h)
{
    LT_LOG_INFO("----------------------------------------------");
    LT_LOG_INFO("lt_test_mock_hardware_fail()");
    LT_LOG_INFO("----------------------------------------------");

    lt_mock_hal_reset(&h->l2);
    LT_LOG_INFO("Mocking initialization...");
    LT_TEST_ASSERT(LT_OK, mock_init_communication(h, (uint8_t[]){0x00, 0x00, 0x00, 0x02}));

    LT_LOG_INFO("Initializing handle");
    LT_TEST_ASSERT(LT_OK, lt_init(h));

    LT_LOG_INFO("Setting up session...");
    uint8_t kcmd[TR01_AES256_KEY_LEN];
    uint8_t kres[TR01_AES256_KEY_LEN];
    LT_TEST_ASSERT(LT_OK, lt_random_bytes(h, kcmd, sizeof(kcmd)));
    memcpy(kres, kcmd, TR01_AES256_KEY_LEN);
    mock_session_start(h, kcmd, kres);

    LT_LOG_INFO("Mocking HARDWARE_FAIL in Pairing_Key_Write reply...");
    uint8_t dummy_key[4];
    for (int slot = TR01_PAIRING_KEY_SLOT_INDEX_0; slot <= TR01_PAIRING_KEY_SLOT_INDEX_3; slot++){
        LT_LOG_INFO("Mocking for slot %d...", slot);
        // Mock replies to the command.
        LT_TEST_ASSERT(LT_OK, mock_l3_command_responses(h, 1));

        // Mock command result itself.
        uint8_t pairing_key_write_plaintext[] = {
            TR01_L3_RESULT_HARDWARE_FAIL,
        };
        LT_TEST_ASSERT(LT_OK, mock_l3_result(h, pairing_key_write_plaintext, sizeof(pairing_key_write_plaintext)));

        LT_TEST_ASSERT(LT_OK, lt_random_bytes(h, dummy_key, sizeof(dummy_key)));
        LT_TEST_ASSERT(LT_L3_HARDWARE_FAIL, lt_pairing_key_write(h, dummy_key, slot));
    }

    LT_LOG_INFO("Mocking HARDWARE_FAIL in Pairing_Key_Invalidate reply...");
    for (int slot = TR01_PAIRING_KEY_SLOT_INDEX_0; slot <= TR01_PAIRING_KEY_SLOT_INDEX_3; slot++){
        LT_LOG_INFO("Mocking for slot %d...", slot);
        // Mock replies to the command.
        LT_TEST_ASSERT(LT_OK, mock_l3_command_responses(h, 1));

        // Mock command result itself.
        uint8_t pairing_key_invalidate_plaintext[] = {
            TR01_L3_RESULT_HARDWARE_FAIL,
        };
        LT_TEST_ASSERT(LT_OK, mock_l3_result(h, pairing_key_invalidate_plaintext, sizeof(pairing_key_invalidate_plaintext)));

        LT_TEST_ASSERT(LT_OK, lt_random_bytes(h, dummy_key, sizeof(dummy_key)));
        LT_TEST_ASSERT(LT_L3_HARDWARE_FAIL, lt_pairing_key_invalidate(h, slot));
    }

    LT_LOG_INFO("Deinitializing handle");
    LT_TEST_ASSERT(LT_OK, lt_deinit((lt_handle_t *)h));
    
    return 0;
}
