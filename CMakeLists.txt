
idf_component_register(
    SRCS "esp32_neopixel.c"
    INCLUDE_DIRS "include"
    REQUIRES "hal" "xtensa"
    "riscv" "esp_rom" "driver" "esp_common" 
    "soc" "esp_hw_support" "esp_system"
    "newlib" "log" "heap")
