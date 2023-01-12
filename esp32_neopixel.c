#include "esp32_neopixel.h"

// Private vars

// The current SPI Handle
spi_device_handle_t _rgb_spi = NULL;
// The current SPI Host device
spi_host_device_t _rgb_device;
rgb_s *_rgb_colors = NULL;
uint8_t _rgb_brightness = 128;

// Public vars
neopixel_status_t neopixel_status = NEOPIXEL_STATUS_DISABLED;


// Private functions

uint8_t _uint8_float_clamp(float in)
{
    int tmp = (int) in;
    if (tmp >= 255)
    {
        return (uint8_t) 255;
    }
    else if (tmp <= 0)
    {
        return (uint8_t) 0;
    }

    return (uint8_t) tmp;
}

uint32_t _rgb_brightadjust(rgb_s color)
{
    float b = _rgb_brightness;
    float ratio = b / 255.0f;
    float new_r = color.red;
    float new_g = color.green;
    float new_b = color.blue;

    new_r = new_r * ratio;
    new_g = new_g * ratio;
    new_b = new_b * ratio;

    rgb_s new_color = {
        .red    = _uint8_float_clamp(new_r),
        .green  = _uint8_float_clamp(new_g),
        .blue   = _uint8_float_clamp(new_b),
    };

    return new_color.rgb;
}

void _rgb_create_packet(uint8_t *buffer)
{
    const char* TAG = "_rgb_create_packet";

    // Clear the buffer
    memset(buffer, 0, CONFIG_NP_RGB_COUNT*RGB_BYTE_MULTIPLIER);
    // Create buffer index
    uint8_t buffer_idx = 0;

    // Create an empty color array matching the size of our real array an zero the values.
    rgb_s adjusted_colors[CONFIG_NP_RGB_COUNT] = {0};

    // Loop through each LED
    for (uint8_t s = 0; s < CONFIG_NP_RGB_COUNT; s++)
    {   
        // Get the adjusted brightness of each LED color
        adjusted_colors[s].rgb = _rgb_brightadjust(_rgb_colors[s]);

        // Set up splitters for each color channel
        rgb_splitter_s s_red    = {0};
        rgb_splitter_s s_green  = {0};
        rgb_splitter_s s_blue   = {0};

        // There are three bytes to contend with
        // for each LED. Each color is split up to occupy three SPI bytes.
        // We have to cycle through each bit of each color and shift in the
        // appropriate data.

        // Keep track of which bit
        // we are setting with an index.
        uint8_t s_idx   = 0;

        // Loop through all 8 bits of each color
        for(uint8_t b = 0; b < 8; b++)
        {
            uint8_t red_bit = 0;
            uint8_t green_bit = 0;
            uint8_t blue_bit = 0;

            // Account for GRB mode (From rightmost bit to left)
            if (CONFIG_NP_GRB_ORDER)
            {
                red_bit     = (adjusted_colors[s].green  >> (b)) & 1;
                green_bit   = (adjusted_colors[s].red    >> (b)) & 1;
            }
            else
            {
                red_bit     = (adjusted_colors[s].red    >> (b)) & 1;
                green_bit   = (adjusted_colors[s].green  >> (b)) & 1;
            }
            
            blue_bit   =    (adjusted_colors[s].blue  >> (b)) & 1;
            
            // Set the bits in the splitter from least significant to most.
            if (red_bit)
            {
                s_red.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_red.splitter      |= (RGB_LOW << (s_idx));
            }

            if (green_bit)
            {
                s_green.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_green.splitter      |= (RGB_LOW << (s_idx));
            }

            if (blue_bit)
            {
                s_blue.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_blue.splitter      |= (RGB_LOW << (s_idx));
            }

            s_idx   += 3;
        }

        // Once we've processed all 8 bits of the three colors, copy to our SPI buffer
        // On ESP32-S3 we have to invert the byte : )
        #if CONFIG_IDF_TARGET_ESP32S3
            // Once we've processed all 8 bits of the three colors, copy to our SPI buffer
            buffer[buffer_idx]  = ~s_red.byte0;
            buffer[buffer_idx+1] = ~s_red.byte1;
            buffer[buffer_idx+2] = ~s_red.byte2;

            buffer[buffer_idx+3] = ~s_green.byte0;
            buffer[buffer_idx+4] = ~s_green.byte1;
            buffer[buffer_idx+5] = ~s_green.byte2;

            buffer[buffer_idx+6] = ~s_blue.byte0;
            buffer[buffer_idx+7] = ~s_blue.byte1;
            buffer[buffer_idx+8] = ~s_blue.byte2;
        #else
            buffer[buffer_idx] = s_red.byte0;
            buffer[buffer_idx+1] = s_red.byte1;
            buffer[buffer_idx+2] = s_red.byte2;

            buffer[buffer_idx+3] = s_green.byte0;
            buffer[buffer_idx+4] = s_green.byte1;
            buffer[buffer_idx+5] = s_green.byte2;

            buffer[buffer_idx+6] = s_blue.byte0;
            buffer[buffer_idx+7] = s_blue.byte1;
            buffer[buffer_idx+8] = s_blue.byte2;
        #endif

        // Increase our buffer idx
        buffer_idx += 9;
    }

}

// Public functions
esp_err_t neopixel_init(rgb_s *led_colors, spi_host_device_t spi_device)
{
    const char* TAG = "neopixel_init";

    //#if CONFIG_NP_RGB_ENABLE
    esp_err_t err; 

    // Set up SPI for rgb
    
    // Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num    = (int) CONFIG_NP_RGB_GPIO,
        .miso_io_num    = -1,
        .sclk_io_num    = -1,
        .quadwp_io_num  = -1,
        .quadhd_io_num  = -1,
    };

    // Configuration for the SPI master interface
    spi_device_interface_config_t devcfg={
        .mode           = 0,
        .clock_speed_hz = 2500000, //2.5Mhz
        .spics_io_num   = -1,
        .queue_size     = 7,
        .input_delay_ns = 0,
    };

    err = spi_bus_initialize(spi_device, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI Bus.");
        neopixel_status = NEOPIXEL_STATUS_DISABLED;
        return ESP_FAIL;
    }

    err = spi_bus_add_device(spi_device, &devcfg, &_rgb_spi);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add SPI Device.");
        neopixel_status = NEOPIXEL_STATUS_DISABLED;
        return ESP_FAIL;
    }

    // Have to invert because some dumb reason on ESP32-S3 I don't understand : )
    #if CONFIG_IDF_TARGET_ESP32S3
        gpio_matrix_out(CONFIG_NP_RGB_GPIO, FSPID_OUT_IDX, true, false);
    #endif

    ESP_LOGI(TAG, "Started RGB Service OK.");
    neopixel_status = NEOPIXEL_STATUS_AVAILABLE;
    // Set current colors to the pointer referenced by user.
    _rgb_colors = led_colors;

    return ESP_OK;
    //#else
    //ESP_LOGE(TAG, "RGB Utility is disabled in SDK settings. Enable there.");
    //return HOJA_FAIL;
    //#endif
}

// Function taken from https://github.com/FastLED/FastLED/blob/master/src/hsv2rgb.cpp
/**
 * @brief Take in HSV and return rgb
 * @param h Hue
 * @param s Saturation
 * @param v Value (Brightness)
*/
uint32_t rgb_from_hsv(uint8_t h, int8_t s, uint8_t v)
{
    float scale = (float) h /255;
    float hf = scale * 191;
    uint8_t hue = (uint8_t) hf;

    rgb_s color_out = {
        .red = 0x00,
        .green = 0x00,
        .blue = 0x00,
    };

    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = v;
    uint8_t saturation = s;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = 255 - saturation;
    uint8_t brightness_floor = (value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hue / HSV_SECTION_3; // 0..2
    uint8_t offset = hue % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


    if( section ) {
        if( section == 1) {
            // section 1: 0x40..0x7F
            color_out.red = brightness_floor;
            color_out.green = rampdown_adj_with_floor;
            color_out.blue = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            color_out.red = rampup_adj_with_floor;
            color_out.green = brightness_floor;
            color_out.blue = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        color_out.red = rampdown_adj_with_floor;
        color_out.green = rampup_adj_with_floor;
        color_out.blue = brightness_floor;
    }

    return color_out.rgb;
}

void rgb_setbrightness(uint8_t brightness)
{
    _rgb_brightness = brightness;
}

rgb_s rgb_blend(rgb_s color1, rgb_s color2, uint8_t blend_amount)
{
    float ratio = blend_amount/255;
    int rdif = abs((color1.red + color2.red))    /2;
    int gdif = abs((color1.green + color2.green))/2;
    int bdif = abs((color1.blue + color2.blue))  /2;

    rdif = rdif * ratio;
    gdif = gdif * ratio;
    bdif = bdif * ratio;

    rgb_s output = {
        .red = color1.red + (uint8_t) rdif,
        .green = color1.green + (uint8_t) gdif,
        .blue = color1.blue + (uint8_t) bdif,
    };

    return output;
}

void rgb_setall(rgb_s color)
{
    const char* TAG = "rgb_setall";
    
    if (_rgb_colors == NULL)
    {
        ESP_LOGE(TAG, "Need to do neopixel_init first!");
        return;
    }

    for (uint8_t i = 0; i < CONFIG_NP_RGB_COUNT; i++)
    {
        _rgb_colors[i].rgb = color.rgb;
    }
}

void rgb_show()
{
    if(neopixel_status != NEOPIXEL_STATUS_AVAILABLE)
    {
        const char* TAG = "rgb_show";
        ESP_LOGE(TAG, "NeoPixel RGB Driver not initialized. Initialize before using this. (neopixel_init function)");
        return;
    }

    uint8_t rgb_spi_buffer[RGB_BYTE_MULTIPLIER * CONFIG_NP_RGB_COUNT];

    _rgb_create_packet(rgb_spi_buffer);

    spi_transaction_t trans = {
        .length = RGB_BYTE_MULTIPLIER*8*CONFIG_NP_RGB_COUNT,
        .tx_buffer = rgb_spi_buffer,
        .user=(void*)1,
    };

    spi_device_transmit(_rgb_spi, &trans);
}

