
# Newer ESP-IDF only registers the CPU component for the target's own
# architecture ("xtensa" or "riscv"), so require that one instead of both.
idf_build_get_property(arch IDF_TARGET_ARCH)

idf_component_register(
    SRCS "esp32_neopixel.c"
    INCLUDE_DIRS "include"
    REQUIRES "hal" "${arch}"
    "esp_rom" "driver" "esp_common"
    "soc" "esp_hw_support" "esp_system"
    "newlib" "log" "heap")
