#include "display.hpp"
#include "config.hpp"
#include "esp_log.h"

static const char *TAG = "display";

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

void setup_display()
{
    oled.clear();
    oled.init(SDA, SCL);
    oled.clear();
    oled.autoPrintln(true);
    oled.setScale(1);
    ESP_LOGI(TAG, "Oled setup complete.");
}

void display_message(const char *message)
{
    oled.clear();
    oled.home();
    oled.print(message);
}

void set_display_scale(uint8_t scale)
{
    oled.setScale(scale);
}