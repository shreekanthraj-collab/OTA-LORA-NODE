/**
 * @file oled.c
 * @brief ORB DRIVE OTA module SSD1306 OLED driver.
 */

#include "oled.h"

#include <stdio.h>
#include <string.h>

#include "driver/i2c_master.h"

#include "esp_err.h"
#include "esp_log.h"

#include "ota_hw_config.h"

static const char *TAG = "OLED";

#define OLED_I2C_PORT              I2C_NUM_0
#define OLED_TIMEOUT_MS            1000

#define SSD1306_CONTROL_CMD        0x00
#define SSD1306_CONTROL_DATA       0x40

#define SSD1306_CMD_DISPLAY_OFF    0xAE
#define SSD1306_CMD_DISPLAY_ON     0xAF
#define SSD1306_CMD_SET_CONTRAST   0x81
#define SSD1306_CMD_NORMAL_DISPLAY 0xA6
#define SSD1306_CMD_SET_MULTIPLEX  0xA8
#define SSD1306_CMD_SET_OFFSET     0xD3
#define SSD1306_CMD_SET_START_LINE 0x40
#define SSD1306_CMD_CHARGE_PUMP    0x8D
#define SSD1306_CMD_MEMORY_MODE    0x20
#define SSD1306_CMD_SEG_REMAP      0xA1
#define SSD1306_CMD_COM_SCAN       0xC8
#define SSD1306_CMD_COM_PINS       0xDA
#define SSD1306_CMD_CLOCK_DIV      0xD5
#define SSD1306_CMD_PRECHARGE      0xD9
#define SSD1306_CMD_VCOM_DETECT    0xDB

static i2c_master_bus_handle_t s_i2c_bus = NULL;
static i2c_master_dev_handle_t s_oled = NULL;

static bool s_initialized = false;

static esp_err_t oled_write_command(uint8_t command)
{
    uint8_t data[2];

    data[0] = SSD1306_CONTROL_CMD;
    data[1] = command;

    return i2c_master_transmit(
        s_oled,
        data,
        sizeof(data),
        OLED_TIMEOUT_MS
    );
}

static esp_err_t oled_write_commands(
    const uint8_t *commands,
    size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        esp_err_t err =
            oled_write_command(commands[i]);

        if (err != ESP_OK)
        {
            return err;
        }
    }

    return ESP_OK;
}

static esp_err_t oled_write_data(
    const uint8_t *data,
    size_t count)
{
    uint8_t buffer[129];

    if (count > 128U)
    {
        return ESP_ERR_INVALID_SIZE;
    }

    buffer[0] = SSD1306_CONTROL_DATA;

    memcpy(
        &buffer[1],
        data,
        count
    );

    return i2c_master_transmit(
        s_oled,
        buffer,
        count + 1U,
        OLED_TIMEOUT_MS
    );
}

static esp_err_t oled_set_cursor(
    uint8_t page,
    uint8_t column)
{
    esp_err_t err;

    err = oled_write_command(
        0xB0U | page
    );

    if (err != ESP_OK)
    {
        return err;
    }

    err = oled_write_command(
        0x00U | (column & 0x0FU)
    );

    if (err != ESP_OK)
    {
        return err;
    }

    return oled_write_command(
        0x10U | ((column >> 4U) & 0x0FU)
    );
}

static esp_err_t oled_fill(uint8_t pattern)
{
    uint8_t line[128];

    memset(
        line,
        pattern,
        sizeof(line)
    );

    for (uint8_t page = 0; page < 8U; page++)
    {
        esp_err_t err =
            oled_set_cursor(page, 0U);

        if (err != ESP_OK)
        {
            return err;
        }

        err =
            oled_write_data(
                line,
                sizeof(line)
            );

        if (err != ESP_OK)
        {
            return err;
        }
    }

    return ESP_OK;
}

static esp_err_t oled_draw_block(
    uint8_t page,
    uint8_t start_column,
    uint8_t width,
    bool filled)
{
    uint8_t data[128];

    if ((uint16_t)start_column + width > 128U)
    {
        return ESP_ERR_INVALID_ARG;
    }

    memset(
        data,
        filled ? 0xFFU : 0x00U,
        width
    );

    esp_err_t err =
        oled_set_cursor(
            page,
            start_column
        );

    if (err != ESP_OK)
    {
        return err;
    }

    return oled_write_data(
        data,
        width
    );
}

/*
 * Small 5x7 font.
 *
 * Characters supported:
 * A-Z
 * 0-9
 * space
 * %
 * /
 * -
 * .
 * :
 */
static const uint8_t font_5x7[][5] =
{
    /* Space */
    {0x00,0x00,0x00,0x00,0x00},

    /* A-Z */
    {0x7E,0x11,0x11,0x11,0x7E},
    {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A},
    {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F},
    {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31},
    {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F},
    {0x7F,0x20,0x18,0x20,0x7F},
    {0x63,0x14,0x08,0x14,0x63},
    {0x03,0x04,0x78,0x04,0x03},
    {0x61,0x51,0x49,0x45,0x43},

    /* 0-9 */
    {0x3E,0x45,0x49,0x51,0x3E},
    {0x00,0x21,0x7F,0x01,0x00},
    {0x23,0x45,0x49,0x51,0x21},
    {0x42,0x41,0x51,0x69,0x46},
    {0x0C,0x14,0x24,0x7F,0x04},
    {0x72,0x51,0x51,0x51,0x4E},
    {0x1E,0x29,0x49,0x49,0x06},
    {0x40,0x47,0x48,0x50,0x60},
    {0x36,0x49,0x49,0x49,0x36},
    {0x30,0x49,0x49,0x4A,0x3C},

    /* % */
    {0x63,0x13,0x08,0x64,0x63},

    /* / */
    {0x20,0x10,0x08,0x04,0x02},

    /* - */
    {0x08,0x08,0x08,0x08,0x08},

    /* . */
    {0x00,0x60,0x60,0x00,0x00},

    /* : */
    {0x00,0x36,0x36,0x00,0x00}
};

static const uint8_t *font_for_char(char c)
{
    if (c == ' ')
    {
        return font_5x7[0];
    }

    if (c >= 'A' && c <= 'Z')
    {
        return font_5x7[1 + (c - 'A')];
    }

    if (c >= '0' && c <= '9')
    {
        return font_5x7[27 + (c - '0')];
    }

    if (c == '%')
    {
        return font_5x7[37];
    }

    if (c == '/')
    {
        return font_5x7[38];
    }

    if (c == '-')
    {
        return font_5x7[39];
    }

    if (c == '.')
    {
        return font_5x7[40];
    }

    if (c == ':')
    {
        return font_5x7[41];
    }

    return font_5x7[0];
}

static esp_err_t oled_write_text(
    uint8_t page,
    uint8_t column,
    const char *text)
{
    if (text == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err =
        oled_set_cursor(
            page,
            column
        );

    if (err != ESP_OK)
    {
        return err;
    }

    while (*text != '\0')
    {
        const uint8_t *glyph =
            font_for_char(*text);

        uint8_t data[6];

        memcpy(
            data,
            glyph,
            5U
        );

        data[5] = 0x00U;

        err =
            oled_write_data(
                data,
                sizeof(data)
            );

        if (err != ESP_OK)
        {
            return err;
        }

        text++;
    }

    return ESP_OK;
}

void oled_init(void)
{
    if (s_initialized)
    {
        return;
    }

    i2c_master_bus_config_t bus_config = {
        .i2c_port = OLED_I2C_PORT,
        .sda_io_num = OTA_GPIO_OLED_SDA,
        .scl_io_num = OTA_GPIO_OLED_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err =
        i2c_new_master_bus(
            &bus_config,
            &s_i2c_bus
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "I2C bus initialization failed: %s",
            esp_err_to_name(err)
        );

        return;
    }

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OTA_OLED_I2C_ADDRESS,
        .scl_speed_hz = OTA_OLED_I2C_FREQUENCY,
    };

    err =
        i2c_master_bus_add_device(
            s_i2c_bus,
            &device_config,
            &s_oled
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "OLED device initialization failed: %s",
            esp_err_to_name(err)
        );

        return;
    }

    const uint8_t init_commands[] =
    {
        SSD1306_CMD_DISPLAY_OFF,

        SSD1306_CMD_CLOCK_DIV,
        0x80,

        SSD1306_CMD_SET_MULTIPLEX,
        0x3F,

        SSD1306_CMD_SET_OFFSET,
        0x00,

        SSD1306_CMD_SET_START_LINE,

        SSD1306_CMD_CHARGE_PUMP,
        0x14,

        SSD1306_CMD_MEMORY_MODE,
        0x00,

        SSD1306_CMD_SEG_REMAP,

        SSD1306_CMD_COM_SCAN,

        SSD1306_CMD_COM_PINS,
        0x12,

        SSD1306_CMD_SET_CONTRAST,
        0x7F,

        SSD1306_CMD_PRECHARGE,
        0xF1,

        SSD1306_CMD_VCOM_DETECT,
        0x40,

        SSD1306_CMD_NORMAL_DISPLAY,

        SSD1306_CMD_DISPLAY_ON,
    };

    err =
        oled_write_commands(
            init_commands,
            sizeof(init_commands)
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "SSD1306 initialization failed: %s",
            esp_err_to_name(err)
        );

        return;
    }

    s_initialized = true;

    oled_clear();

    ESP_LOGI(
        TAG,
        "SSD1306 OLED initialized"
    );
}

void oled_clear(void)
{
    if (!s_initialized)
    {
        return;
    }

    oled_fill(0x00U);
}

void oled_show_startup(void)
{
    if (!s_initialized)
    {
        return;
    }

    oled_clear();

    oled_write_text(
        1,
        20,
        "ORB DRIVE OTA"
    );

    oled_write_text(
        3,
        34,
        "CONNECTING"
    );
}

void oled_show_node_found(void)
{
    if (!s_initialized)
    {
        return;
    }

    oled_clear();

    oled_write_text(
        1,
        34,
        "NODE FOUND"
    );

    oled_write_text(
        3,
        38,
        "LORA-NODE"
    );
}

void oled_show_node_connected(void)
{
    if (!s_initialized)
    {
        return;
    }

    oled_clear();

    oled_write_text(
        1,
        28,
        "NODE CONNECTED"
    );

    oled_write_text(
        3,
        22,
        "192.168.4.1"
    );
}

void oled_show_upload_progress(
    size_t bytes_sent,
    size_t total_bytes)
{
    if (!s_initialized ||
        total_bytes == 0U)
    {
        return;
    }

    if (bytes_sent > total_bytes)
    {
        bytes_sent = total_bytes;
    }

    uint32_t percentage =
        (uint32_t)(
            ((uint64_t)bytes_sent * 100U) /
            total_bytes
        );

    uint8_t blocks =
        (uint8_t)(
            (percentage * 10U) / 100U
        );

    oled_clear();

    oled_write_text(
        0,
        34,
        "OTA UPDATE"
    );

    oled_write_text(
        2,
        42,
        "UPLOADING"
    );

    /*
     * Ten-block progress bar.
     */
    for (uint8_t i = 0; i < 10U; i++)
    {
        oled_draw_block(
            4,
            18U + (i * 9U),
            7U,
            i < blocks
        );
    }

    char percentage_text[8];

    snprintf(
        percentage_text,
        sizeof(percentage_text),
        "%lu%%",
        (unsigned long)percentage
    );

    oled_write_text(
        5,
        52,
        percentage_text
    );

    char size_text[32];

    snprintf(
        size_text,
        sizeof(size_text),
        "%u / %u KB",
        (unsigned int)(bytes_sent / 1024U),
        (unsigned int)(total_bytes / 1024U)
    );

    oled_write_text(
        7,
        8,
        size_text
    );
}

void oled_show_upload_complete(void)
{
    if (!s_initialized)
    {
        return;
    }

    oled_clear();

    oled_write_text(
        1,
        28,
        "OTA UPDATE"
    );

    oled_write_text(
        3,
        16,
        "UPLOAD COMPLETE"
    );

    oled_write_text(
        5,
        52,
        "100%"
    );
}

void oled_show_failure(const char *reason)
{
    if (!s_initialized)
    {
        return;
    }

    oled_clear();

    oled_write_text(
        1,
        28,
        "OTA UPDATE"
    );

    oled_write_text(
        3,
        46,
        "FAILED"
    );

    if (reason != NULL)
    {
        /*
         * Failure reasons will later be mapped to
         * concise display strings by the OTA manager.
         */
        oled_write_text(
            5,
            4,
            reason
        );
    }
}
