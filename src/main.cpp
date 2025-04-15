#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <esp32-hal-log.h>
#include "config.hpp"
#include "wifi_manager.hpp"
#include "ota_manager.hpp"
#include "display.hpp"
#include "water_level.hpp"
#include "pump_controller.hpp"
#include "storage.hpp"

static const char *TAG = "main";

// Queue Storage
static StaticQueue_t water_level_buffer;
uint8_t water_level_storage[CONFIG_DATASEND_QUEUE_SIZE * DATASEND_QUEUE_ITEM_SIZE];

void setup()
{
    // Setup Logging
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    // Initialize GPIO
    setup_gpio();
    setup_display();

    display_message("Hydroponic Controller");
    
    // Initialize storage
    init_storage();
    
    // Load or save WiFi credentials
    if (get_wifi_ssid() == "" || get_wifi_password() == "")
    {
        load_credentials();
    }
    else
    {
        save_credentials();
    }
    
    // Connect to WiFi
    setup_wifi();
    
    // Setup OTA updates
    setup_ota();

    // Create water level queue
    QueueHandle_t water_level = xQueueCreateStatic(
        CONFIG_DATASEND_QUEUE_SIZE, 
        DATASEND_QUEUE_ITEM_SIZE, 
        water_level_storage, 
        &water_level_buffer
    );

    // Start water level monitoring task
    if (xTaskCreate(water_level_task, "water_level_task", 4096, (void *)water_level, 1, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create water_level_task. Check system resources.");
    }

    // Start pump control task
    if (xTaskCreate(pump_task, "pump_task", 4096, (void *)water_level, 2, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create pump_task. Check system resources.");
    }
    
    set_display_scale(2);
}

void loop()
{
    handle_ota();
}