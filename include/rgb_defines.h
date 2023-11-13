#ifndef RGB_DEFINES_H
#define RGB_DEFINES_H

#include <inttypes.h>

// How many bytes per LED?
#define RGB_BYTE_MULTIPLIER 9

// Timings for bits
#define RGB_T0H     1
#define RGB_T0L     2
#define RGB_T1H     2
#define RGB_T1L     1

#define RGB_HIGH    0x6
#define RGB_LOW     0x4

// We use this type to set the bits
// appropriately for the SPI output.
// Easier to work with one 32 bit int
// instead of each individual byte.
// 3 bytes are allocated to each color
typedef struct 
{
    union
    {
        struct 
        {
            uint8_t byte2;
            uint8_t byte1;
            uint8_t byte0;
            uint8_t dummy;
        };
        uint32_t splitter; 
    };
} __attribute__ ((packed)) rgb_splitter_s;

/**
 *  @brief RGB Struct Type Definition
 */
typedef struct
{
    union
    {
        struct
        {
            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t dummy;
        };
        uint32_t rgb;
    };
    
} __attribute__ ((packed)) neo_rgb_s;

typedef enum
{
    NEOPIXEL_STATUS_IDLE        = 0,
    NEOPIXEL_STATUS_STARTED     = 1,
} neopixel_status_t;

typedef enum
{
    NEOPIXEL_RGB_MODE_RGB = 0,
    NEOPIXEL_RGB_MODE_GRB = 1,
} neopixel_rgb_mode_t;

#endif