/*
 * ESP-IDF Hello World example using libtropic (SPI + mbedtls CAL)
 */

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "libtropic.h"
#include "libtropic_common.h"
#include "libtropic_mbedtls_v4.h"
#include "libtropic_port_esp_idf.h"
#include "psa/crypto.h"

#define TAG "libtropic_hello"

// Message to send with Ping L3 command.
#define PING_MSG "This is Hello World message from TROPIC01!!"
// Size of the Ping message, including '\0'.
#define PING_MSG_SIZE 44

// Choose pairing keypair for slot 0.
#if LT_USE_SH0_ENG_SAMPLE
#define LT_EX_SH0_PRIV sh0priv_eng_sample
#define LT_EX_SH0_PUB sh0pub_eng_sample
#elif LT_USE_SH0_PROD0
#define LT_EX_SH0_PRIV sh0priv_prod0
#define LT_EX_SH0_PUB sh0pub_prod0
#endif

void app_main(void)
{
    ESP_LOGI(TAG, "Starting libtropic ESP-IDF Hello World example");

    // Initialize PSA crypto
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        ESP_LOGE(TAG, "PSA Crypto initialization failed: %d", status);
        return;
    }

    // Libtropic handle
    lt_handle_t lt_handle = {0};

    // Device structure for ESP-IDF
    lt_dev_esp_idf_t device = {0};
    // Default pins for a typical ESP32 DevKit; adjust if you use different wiring.
    device.spi_host_id = HSPI_HOST;  // or VSPI_HOST depending on your wiring
    device.spi_cs_gpio_pin = GPIO_NUM_5;
    device.spi_miso_pin = GPIO_NUM_19;
    device.spi_mosi_pin = GPIO_NUM_23;
    device.spi_clk_pin = GPIO_NUM_18;
    device.spi_clk_hz = 5000000;  // 5 MHz
#if LT_USE_INT_PIN
    device.int_gpio_pin = GPIO_NUM_4;  // change if you connect INT pin
#endif

    lt_handle.l2.device = &device;

    // Crypto abstraction layer (CAL) context.
    lt_ctx_mbedtls_v4_t crypto_ctx;
    lt_handle.l3.crypto_ctx = &crypto_ctx;

    ESP_LOGI(TAG, "Initializing libtropic handle...");
    lt_ret_t ret = lt_init(&lt_handle);
    if (LT_OK != ret) {
        ESP_LOGE(TAG, "lt_init() failed: %s", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return;
    }
    ESP_LOGI(TAG, "lt_init OK");

    ESP_LOGI(TAG, "Sending reboot request...");
    ret = lt_reboot(&lt_handle, TR01_REBOOT);
    if (ret != LT_OK) {
        ESP_LOGE(TAG, "lt_reboot() failed: %s", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return;
    }
    ESP_LOGI(TAG, "Reboot request OK");

    ESP_LOGI(TAG, "Starting Secure Session with key slot %d", (int)TR01_PAIRING_KEY_SLOT_INDEX_0);
    ret = lt_verify_chip_and_start_secure_session(&lt_handle, LT_EX_SH0_PRIV, LT_EX_SH0_PUB,
                                                  TR01_PAIRING_KEY_SLOT_INDEX_0);
    if (LT_OK != ret) {
        ESP_LOGE(TAG, "Failed to start Secure Session: %s", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return;
    }
    ESP_LOGI(TAG, "Secure Session OK");

    uint8_t recv_buf[PING_MSG_SIZE];
    ESP_LOGI(TAG, "Sending Ping command... Message: '%s'", PING_MSG);
    ret = lt_ping(&lt_handle, (const uint8_t *)PING_MSG, recv_buf, PING_MSG_SIZE);
    if (LT_OK != ret) {
        ESP_LOGE(TAG, "Ping command failed: %s", lt_ret_verbose(ret));
        lt_session_abort(&lt_handle);
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return;
    }
    ESP_LOGI(TAG, "Ping reply: '%s'", recv_buf);

    ESP_LOGI(TAG, "Aborting Secure Session...");
    ret = lt_session_abort(&lt_handle);
    if (LT_OK != ret) {
        ESP_LOGE(TAG, "lt_session_abort() failed: %s", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return;
    }
    ESP_LOGI(TAG, "Session aborted OK");

    ESP_LOGI(TAG, "Deinitializing handle...");
    ret = lt_deinit(&lt_handle);
    if (LT_OK != ret) {
        ESP_LOGE(TAG, "lt_deinit() failed: %s", lt_ret_verbose(ret));
        mbedtls_psa_crypto_free();
        return;
    }
    ESP_LOGI(TAG, "Deinit OK");

    // Free mbedtls PSA resources
    mbedtls_psa_crypto_free();

    ESP_LOGI(TAG, "Example finished.");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
