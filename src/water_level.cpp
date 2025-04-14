#include "water_level.hpp"
#include "display.hpp"
#include "esp_log.h"
#include <driver/gpio.h>
#include "config.hpp"

static const char *TAG = "water_level";

void water_level_task(void *param)
{
    QueueHandle_t dataSendQueue = (QueueHandle_t)param;
    water_level_t level;
    while (true)
    {
        if (gpio_get_level(LEVEL_FOUR))
        {
            ESP_LOGI(TAG, "LEVEL FOUR detected. Sending alert to pump control.");
            display_message("LEVEL: 4");
            level = LOW_FOUR;
            xQueueSend(dataSendQueue, &level, 0);
        }
        else if (gpio_get_level(LEVEL_THREE))
        {
            ESP_LOGI(TAG, "LEVEL THREE detected.");
            display_message("LEVEL: 3");
            level = LOW_THREE;
            xQueueSend(dataSendQueue, &level, 0);
        }
        else if (gpio_get_level(LEVEL_TWO))
        {
            ESP_LOGI(TAG, "LEVEL TWO detected.");
            display_message("LEVEL: 2");
            level = LOW_TWO;
            xQueueSend(dataSendQueue, &level, 0);
        }
        else if (gpio_get_level(LEVEL_ONE))
        {
            ESP_LOGI(TAG, "LEVEL ONE detected.");
            display_message("LEVEL: 1");
            level = LOW_ONE;
            xQueueSend(dataSendQueue, &level, 0);
        }
        // Add delay to prevent watchdog trigger and yield CPU time
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}