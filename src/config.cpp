#include "config.hpp"
#include "esp_log.h"

static const char *TAG = "config";

void setup_gpio()
{
    gpio_set_direction(PUMP, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(PUMP, GPIO_PULLUP_ONLY);

    gpio_set_direction(LEVEL_ONE, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LEVEL_ONE, GPIO_PULLDOWN_ONLY);

    gpio_set_direction(LEVEL_TWO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LEVEL_TWO, GPIO_PULLDOWN_ONLY);

    gpio_set_direction(LEVEL_THREE, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LEVEL_THREE, GPIO_PULLDOWN_ONLY);

    gpio_set_direction(LEVEL_FOUR, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LEVEL_FOUR, GPIO_PULLDOWN_ONLY);

    ESP_LOGI(TAG, "GPIO setup complete. All pins configured successfully.");
}