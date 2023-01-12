// Example supports ESP32 and ESP32-S3

#include "esp32_neopixel.h"

#define RGB_SPIDEV SPI2_HOST

rgb_s led_colors[CONFIG_NP_RGB_COUNT] = {0};
rgb_s last_color = {.rgb=0x0};
rgb_s current_color = {.rgb = 0x0};
rgb_s next_color = {.rgb=0x0};

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

// Demo rgb_blend
void fade_to(rgb_s color)
{
    last_color.rgb = current_color.rgb;
    next_color.rgb = color.rgb;
    uint8_t count_down = 26;
    while(count_down > 0)
    {
        int fader = 256 - (count_down * 10);
        if (fader > 255)
        {
            fader = 255;
        }
        else if (fader <= 0)
        {
            fader = 0;
        }

        rgb_blend(&current_color, last_color, next_color, (uint8_t) fader);
        rgb_setall(current_color);
        rgb_show();
        //ESP_LOGI("COL", "%x", (unsigned int) current_color.rgb);
        vTaskDelay(30/portTICK_PERIOD_MS);
        count_down -= 1;
    }
    current_color.rgb = next_color.rgb;
    rgb_setall(current_color);
    rgb_show();
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
        fade_out();
        rgb_setall(COLOR_BLACK);
        rgb_setbrightness(MAXBRIGHT);
        fade_to(COLOR_RED);
        fade_to(COLOR_BLACK);
        fade_to(COLOR_BLUE);
        fade_to(COLOR_BLACK);
        fade_to(COLOR_GREEN);
        fade_to(COLOR_BLACK);
        fade_to(COLOR_RED);
    }
    
}