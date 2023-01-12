// Example supports ESP32 and ESP32-S3

#include "esp32_neopixel.h"

#define RGB_SPIDEV SPI2_HOST

rgb_s led_colors[CONFIG_NP_RGB_COUNT] = {0};

#define TIMESTAMP 20
#define MAXBRIGHT 100

uint8_t bright = MAXBRIGHT;
uint8_t flip = 0;

int hue = 0;
uint8_t rainbowflip = 0;

// Loops through the rainbow however many times as count
// Uses HSV values as an example :)
void color_loop(uint8_t count)
{
    while (rainbowflip < count)
    {
        rgb_s tmp = 
        { 
            .rgb = rgb_from_hsv((uint8_t) hue, 255, 255)
        };
        rgb_setall(tmp);
        rgb_show();
        vTaskDelay(TIMESTAMP/portTICK_PERIOD_MS);
        hue+=1;
        if (hue > 255)
        {
            hue = 0;
            rainbowflip += 1;
        }
    }

    rainbowflip = 0;
}

void fade_out()
{
    while (bright > 0)
    {
        bright -= 1;
        rgb_setbrightness(bright);
        rgb_show();
        vTaskDelay(TIMESTAMP/portTICK_PERIOD_MS);
    }
}

void fade_in()
{
    while (bright < MAXBRIGHT)
    {
        bright += 1;
        rgb_setbrightness(bright);
        rgb_show();
        vTaskDelay(TIMESTAMP/portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Initialize the neopixel driver
    neopixel_init(led_colors, RGB_SPIDEV);

    // Set the starting brightness
    rgb_setbrightness(MAXBRIGHT);

    // Show the LEDs
    rgb_show();

    // We loop through some different animation examples
    for(;;)
    {
        color_loop(1);
        fade_out();
        rgb_setall(COLOR_TEAL);
        fade_in();
        fade_out();
        rgb_setall(COLOR_GREEN);
        fade_in();
        fade_out();
        rgb_setall(COLOR_RED);
        fade_in();
    }
    
}