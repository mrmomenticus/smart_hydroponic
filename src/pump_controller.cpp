#include "pump_controller.hpp"
#include "display.hpp"
#include "esp_log.h"
#include "config.hpp"
#include <driver/gpio.h>

static const char *TAG = "pump_controller";

void pump_task(void *param)
{
    QueueHandle_t queue = (QueueHandle_t)param;
    water_level_t level;
    while (1)
    {
        if (xQueuePeek(queue, &level, 0) == pdTRUE)
        {
            xQueueReceive(queue, &level, portMAX_DELAY);
            if (level == LOW_FOUR)
            {
                ESP_LOGW(TAG, "CRITICAL LEVEL detected! Pump is turned OFF to prevent overflow.");
                gpio_set_level(PUMP, 0);
                vTaskDelay(100 / portTICK_PERIOD_MS); // Add small delay when in critical level
                continue;
            }
        }
        gpio_set_level(PUMP, 1);
        ESP_LOGI(TAG, "Pump is ON. Water is being pumped.");
        display_message("PUMP ON");
        vTaskDelay(ON_TIME / portTICK_PERIOD_MS);
        gpio_set_level(PUMP, 0);
        ESP_LOGI(TAG, "Pump is OFF. Waiting for next cycle.");
        display_message("PUMP OFF");
        vTaskDelay(OFF_TIME / portTICK_PERIOD_MS);
    }
}