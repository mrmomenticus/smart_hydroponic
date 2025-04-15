#pragma once

#include <driver/gpio.h>

// Hostname
#define HOSTNAME "hydroponic-controller"

namespace HYDROPONIC_GPIO
{
    // GPIO Definitions
#define PUMP GPIO_NUM_17
#define LEVEL_ONE GPIO_NUM_39
#define LEVEL_TWO GPIO_NUM_38
#define LEVEL_THREE GPIO_NUM_37
#define LEVEL_FOUR GPIO_NUM_35
#define SDA 5
#define SCL 6
}

// Timing Constants
#define ON_TIME 1000
#define OFF_TIME 10000
#define CONFIG_DATASEND_QUEUE_SIZE 16
#define DATASEND_QUEUE_ITEM_SIZE sizeof(water_level_t)

// Water Level Enum
typedef enum
{
    LOW_ONE = 1,
    LOW_TWO,
    LOW_THREE,
    LOW_FOUR
} water_level_t;

// Function Prototypes
void setup_gpio();