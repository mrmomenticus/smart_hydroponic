#pragma once

#include <GyverOLED.h>

extern GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

// Function Prototypes
void setup_display();
void display_message(const char *message);
void set_display_scale(uint8_t scale);