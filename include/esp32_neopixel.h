#ifndef ESP32_NEOPIXEL_H
#define ESP32_NEOPIXEL_H

#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#include "sdkconfig.h"

#include "driver/spi_slave.h"
#include "driver/spi_master.h"

#include "rgb_defines.h"
#include "rgb_colors.h"

#include "esp_log.h"

#include "rom/gpio.h"
#include "soc/gpio_sig_map.h"

// Public variables
extern neopixel_status_t neopixel_status;

// Public functions

// Initialize NeoPixel Driver
esp_err_t neopixel_init(neo_rgb_s *led_colors, spi_host_device_t spi_device);

// Public CHSV color functions
#define HSV_SECTION_6 (0x20)
#define HSV_SECTION_3 (0x40)

uint32_t neo_rgb_from_hsv(uint8_t h, int8_t s, uint8_t v);

// Set RGB LED global brightness (Needs rgb_show after to see updated value)
void neo_rgb_setbrightness(uint8_t brightness);

// Set all RGB LEDs to one color
void neo_rgb_setall(neo_rgb_s color);

// Blends together two rgb_s colors and returns the blended color.
void neo_rgb_blend(neo_rgb_s *color_out, neo_rgb_s color1, neo_rgb_s color2, uint8_t blend_amount);

// Update the RGB leds with the current set colors.
void neo_rgb_show(void);

#endif