idf_component_register(
    SRCS
        "main.c"

        "app/ota_app.c"

        "hardware/oled.c"
        "hardware/buzzer.c"
        "hardware/buttons.c"

        "wifi/wifi_manager.c"

        "ota/ota_manager.c"

        "storage/firmware_storage.c"

    INCLUDE_DIRS
        "."
        "app"
        "hardware"
        "wifi"
        "ota"
        "storage"

    EMBED_FILES
        "../firmware/node/orbdrive_firmware.bin"
)
