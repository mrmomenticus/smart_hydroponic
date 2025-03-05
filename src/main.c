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

typedef enum
{
    LOW_ONE = 1,
    LOW_TWO = 2,
    LOW_THREE = 3,
    LOW_FOUR = 4
} water_level_t;

#define CONFIG_DATASEND_QUEUE_SIZE 16
#define DATASEND_QUEUE_ITEM_SIZE sizeof(water_level_t)

static StaticQueue_t water_level_buffer;
uint8_t water_level_storage[CONFIG_DATASEND_QUEUE_SIZE * DATASEND_QUEUE_ITEM_SIZE];

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    ESP_LOGI("test", "GPIO interrupt");
    gpio_set_level(GPIO_NUM_8, 1);
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
    // gpio_set_direction(GPIO_NUM_8, GPIO_MODE_OUTPUT);
    // gpio_set_pull_mode(GPIO_NUM_8, GPIO_PULLUP_ONLY);
    // gpio_set_direction(GPIO_NUM_36, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(GPIO_NUM_36, GPIO_PULLDOWN_ONLY);
    //   esp_err_t err = gpio_install_isr_service(0);
    // if (err == ESP_ERR_INVALID_STATE)
    // {
    //     ESP_LOGW("ISR", "GPIO isr service already installed");
    // };
    // // Регистрируем обработчик прерывания на нажатие кнопки
    // gpio_isr_handler_add(GPIO_NUM_36, gpio_isr_handler, NULL);
    // // Устанавливаем тип события для генерации прерывания - по низкому уровню
    // gpio_set_intr_type(GPIO_NUM_36, GPIO_INTR_NEGEDGE);
    // // Разрешаем использование прерываний
    // gpio_intr_enable(GPIO_NUM_36);
}

void loop_pump(QueueHandle_t *queue)
{
    water_level_t level;
    while (1)
    {
        if (xQueuePeek(*queue, level, 0) != NULL)
        {
            level = xQueueReceive(*queue, &level, 10);
            if (level == LOW_FOUR)
            {
                ESP_LOGW("main", "CRITICAL LEVEL");
                gpio_set_level(PUMP, 0);
                continue;
            }
        }
        else
        {
            vTaskDelay(OFF_TIME / portTICK_PERIOD_MS);
            gpio_set_level(PUMP, 1);
            vTaskDelay(ON_TIME / portTICK_PERIOD_MS);
            gpio_set_level(PUMP, 0);
        }
    }
}

void loop_level_water(QueueHandle_t *_dataSendQueue)
{

    while (true)
    {
        int status = gpio_get_level(LEVEL_ONE);

        if (status == 1)
        {
        }
    }
}
void app_main()
{
    QueueHandle_t _water_level_critical = xQueueCreateStatic(CONFIG_DATASEND_QUEUE_SIZE, DATASEND_QUEUE_ITEM_SIZE, &water_level_storage[0], &water_level_buffer);
    QueueHandle_t _water_level_other = xQueueCreateStatic(CONFIG_DATASEND_QUEUE_SIZE, DATASEND_QUEUE_ITEM_SIZE, &water_level_storage[0], &water_level_buffer);
    TaskHandle_t xHandle = NULL;
    setup();
    BaseType_t err = xTaskCreate(loop_pump, "loop_pump", 4096, &_water_level_critical, 1, &xHandle);

    if (err != pdPASS)
    {
        ESP_LOGE("main", "Failed to create loop_pump task");
        vTaskDelete(xHandle);
    }
}
