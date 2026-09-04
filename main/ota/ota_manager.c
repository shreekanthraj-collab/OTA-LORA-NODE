/**
 * @file ota_manager.c
 * @brief ORB DRIVE OTA module HTTP firmware upload manager.
 */

#include "ota_manager.h"

#include <stdio.h>
#include <string.h>

#include "esp_http_client.h"
#include "esp_log.h"

#include "ota_hw_config.h"

static const char *TAG = "OTA_MANAGER";

#define OTA_HTTP_TIMEOUT_MS        15000
#define OTA_UPLOAD_CHUNK_SIZE      4096U

static bool s_initialized = false;

/**
 * @brief Initialize OTA HTTP manager.
 */
void ota_manager_init(void)
{
    if (s_initialized)
    {
        return;
    }

    s_initialized = true;

    ESP_LOGI(
        TAG,
        "OTA HTTP manager initialized"
    );
}

/**
 * @brief Upload firmware to the Node.
 *
 * The firmware is transmitted in controlled chunks.
 * Progress is based on the number of bytes successfully
 * accepted by esp_http_client_write().
 */
bool ota_manager_upload(
    const uint8_t *firmware_data,
    size_t firmware_size,
    ota_progress_callback_t progress_callback,
    void *context)
{
    if (!s_initialized)
    {
        ESP_LOGE(
            TAG,
            "OTA manager not initialized"
        );

        return false;
    }

    if (firmware_data == NULL ||
        firmware_size == 0U)
    {
        ESP_LOGE(
            TAG,
            "Invalid firmware image"
        );

        return false;
    }

    char url[64];

    snprintf(
        url,
        sizeof(url),
        "http://%s%s",
        OTA_NODE_IP,
        OTA_NODE_OTA_PATH
    );

    ESP_LOGI(
        TAG,
        "OTA URL: %s",
        url
    );

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = OTA_HTTP_TIMEOUT_MS,
    };

    esp_http_client_handle_t client =
        esp_http_client_init(&config);

    if (client == NULL)
    {
        ESP_LOGE(
            TAG,
            "HTTP client initialization failed"
        );

        return false;
    }

    esp_err_t err =
        esp_http_client_set_header(
            client,
            "Content-Type",
            "application/octet-stream"
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to set Content-Type"
        );

        esp_http_client_cleanup(client);

        return false;
    }

    /*
     * Tell the Node the exact firmware size.
     * This allows the Node HTTP server to process
     * the complete OTA image as one POST request.
     */
    err =
        esp_http_client_open(
            client,
            (int)firmware_size
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to open HTTP connection: %s",
            esp_err_to_name(err)
        );

        esp_http_client_cleanup(client);

        return false;
    }

    ESP_LOGI(
        TAG,
        "HTTP connection established"
    );

    size_t bytes_sent = 0U;

    while (bytes_sent < firmware_size)
    {
        size_t remaining =
            firmware_size - bytes_sent;

        size_t chunk_size =
            remaining > OTA_UPLOAD_CHUNK_SIZE
                ? OTA_UPLOAD_CHUNK_SIZE
                : remaining;

        int written =
            esp_http_client_write(
                client,
                (const char *)&firmware_data[bytes_sent],
                (int)chunk_size
            );

        if (written < 0)
        {
            ESP_LOGE(
                TAG,
                "HTTP write failed at %u/%u bytes",
                (unsigned int)bytes_sent,
                (unsigned int)firmware_size
            );

            esp_http_client_close(client);
            esp_http_client_cleanup(client);

            return false;
        }

        if (written == 0)
        {
            ESP_LOGE(
                TAG,
                "HTTP write returned zero bytes"
            );

            esp_http_client_close(client);
            esp_http_client_cleanup(client);

            return false;
        }

        bytes_sent += (size_t)written;

        /*
         * Progress is generated from the actual number
         * of bytes accepted by the HTTP client write call.
         */
        if (progress_callback != NULL)
        {
            progress_callback(
                bytes_sent,
                firmware_size,
                context
            );
        }
    }

    ESP_LOGI(
        TAG,
        "Firmware upload transmission complete: %u bytes",
        (unsigned int)bytes_sent
    );

    /*
     * The Node must respond before we report successful OTA.
     */
    err =
        esp_http_client_fetch_headers(client);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed waiting for Node response: %s",
            esp_err_to_name(err)
        );

        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        return false;
    }

    int status_code =
        esp_http_client_get_status_code(client);

    ESP_LOGI(
        TAG,
        "Node HTTP status: %d",
        status_code
    );

    /*
     * Read and discard the Node response body.
     * This allows the HTTP client to complete the
     * transaction cleanly.
     */
    uint8_t response_buffer[256];

    while (true)
    {
        int read_length =
            esp_http_client_read(
                client,
                (char *)response_buffer,
                sizeof(response_buffer)
            );

        if (read_length <= 0)
        {
            break;
        }
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (status_code >= 200 &&
        status_code < 300)
    {
        /*
         * Explicitly ensure the UI reaches exactly 100%.
         */
        if (progress_callback != NULL)
        {
            progress_callback(
                firmware_size,
                firmware_size,
                context
            );
        }

        ESP_LOGI(
            TAG,
            "Node accepted OTA upload"
        );

        return true;
    }

    ESP_LOGE(
        TAG,
        "Node rejected OTA upload: HTTP %d",
        status_code
    );

    return false;
}
