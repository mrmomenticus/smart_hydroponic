#include "controller_water.h"


static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    ESP_LOGI("test", "GPIO interrupt");
    gpio_set_level(GPIO_NUM_8, 1);
}

void setup()
{
    gpio_set_direction(GPIO_NUM_8, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(GPIO_NUM_8, GPIO_PULLUP_ONLY);
    gpio_set_direction(GPIO_NUM_36, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_36, GPIO_PULLDOWN_ONLY);
      esp_err_t err = gpio_install_isr_service(0);
    if (err == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW("ISR", "GPIO isr service already installed");
    };
    // Регистрируем обработчик прерывания на нажатие кнопки
    gpio_isr_handler_add(GPIO_NUM_36, gpio_isr_handler, NULL);
    // Устанавливаем тип события для генерации прерывания - по низкому уровню
    gpio_set_intr_type(GPIO_NUM_36, GPIO_INTR_NEGEDGE);
    // Разрешаем использование прерываний
    gpio_intr_enable(GPIO_NUM_36);
}