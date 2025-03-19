#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define PUMP GPIO_NUM_17
#define LEVEL_ONE GPIO_NUM_39
#define LEVEL_TWO GPIO_NUM_38
#define LEVEL_THREE GPIO_NUM_37
#define LEVEL_FOUR GPIO_NUM_35

#define ON_TIME 1000
#define OFF_TIME 10000
#define CONFIG_DATASEND_QUEUE_SIZE 16
#define DATASEND_QUEUE_ITEM_SIZE sizeof(water_level_t)

typedef enum
{
    LOW_ONE = 1,
    LOW_TWO,
    LOW_THREE,
    LOW_FOUR
} water_level_t;

static StaticQueue_t water_level_buffer;
uint8_t water_level_storage[CONFIG_DATASEND_QUEUE_SIZE * DATASEND_QUEUE_ITEM_SIZE];

void loop_pump(QueueHandle_t *queue)
{
    water_level_t level;
    while (1)
    {
        if (xQueuePeek(*queue, &level, 0) == pdTRUE)
        {
            xQueueReceive(*queue, &level, portMAX_DELAY);
            if (level == LOW_FOUR)
            {
                ESP_LOGW("main", "CRITICAL LEVEL");
                gpio_set_level(PUMP, 0);
                continue;
            }
        }
        gpio_set_level(PUMP, 1);
        vTaskDelay(ON_TIME / portTICK_PERIOD_MS);
        gpio_set_level(PUMP, 0);
        vTaskDelay(OFF_TIME / portTICK_PERIOD_MS);
    }
}

void loop_level_water(QueueHandle_t *_dataSendQueue)
{
    water_level_t level;
    while (true)
    {
        if (gpio_get_level(LEVEL_FOUR))
        {
            ESP_LOGI("main", "LEVEL FOUR");
            level = LOW_FOUR;
            xQueueSend(_dataSendQueue, &level, 0);
            continue;
        }
        if (gpio_get_level(LEVEL_THREE))
        {
            ESP_LOGI("main", "LEVEL THREE");
            level = LOW_THREE;
            xQueueSend(_dataSendQueue, &level, 0);
            continue;
        }
        if (gpio_get_level(LEVEL_TWO))
        {
            ESP_LOGI("main", "LEVEL TWO");
            level = LOW_TWO;
            xQueueSend(_dataSendQueue, &level, 0);
            continue;

        }
        if (gpio_get_level(LEVEL_ONE))
        {
            ESP_LOGI("main", "LEVEL ONE");
            level = LOW_ONE;
            xQueueSend(_dataSendQueue, &level, 0);
            continue;
        }
    }
}

void setup()
{
    gpio_set_direction(PUMP, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(PUMP, GPIO_PULLUP_ONLY);
    gpio_set_direction(LEVEL_ONE, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LEVEL_ONE, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(LEVEL_TWO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LEVEL_TWO, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(LEVEL_THREE, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LEVEL_THREE, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(LEVEL_FOUR, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LEVEL_FOUR, GPIO_PULLDOWN_ONLY);
    ESP_LOGI("main", "init gpio done");

}

void app_main() {
    QueueHandle_t _water_level = xQueueCreateStatic(CONFIG_DATASEND_QUEUE_SIZE, DATASEND_QUEUE_ITEM_SIZE, water_level_storage, &water_level_buffer);
    setup();

    if (xTaskCreate(loop_level_water, "loop_level_water", 4096, &_water_level, 1, NULL) != pdPASS) {
        ESP_LOGE("main", "Failed to create loop_level_water task");
    }

    if (xTaskCreate(loop_pump, "loop_pump", 4096, &_water_level, 2, NULL) != pdPASS) {
        ESP_LOGE("main", "Failed to create loop_pump task");
    }

    // Start the scheduler
    vTaskStartScheduler();
}