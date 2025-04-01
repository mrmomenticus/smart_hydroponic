#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Arduino.h>
#include <WiFi.h>

// Function Prototypes
void setup_wifi();
void setup_gpio();

// GPIO Definitions
#define PUMP GPIO_NUM_17
#define LEVEL_ONE GPIO_NUM_39
#define LEVEL_TWO GPIO_NUM_38
#define LEVEL_THREE GPIO_NUM_37
#define LEVEL_FOUR GPIO_NUM_35

// Timing Constants
#define ON_TIME 1000
#define OFF_TIME 10000
#define CONFIG_DATASEND_QUEUE_SIZE 16
#define DATASEND_QUEUE_ITEM_SIZE sizeof(water_level_t)

// Water Level Enum
typedef enum {
    LOW_ONE = 1,
    LOW_TWO,
    LOW_THREE,
    LOW_FOUR
} water_level_t;

// Queue Storage
static StaticQueue_t water_level_buffer;
uint8_t water_level_storage[CONFIG_DATASEND_QUEUE_SIZE * DATASEND_QUEUE_ITEM_SIZE];

// WiFi Credentials
const char *ssid = "hydropone";
const char *password = "123456789";

// WiFi Server
WiFiServer server(80);
void setup_wifi() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(ssid, password)) {
        printf("WiFi AP started successfully. AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
        server.begin();
        printf("WiFi Server started and listening for connections.\n");
    } else {
        printf("ERROR: Failed to start WiFi AP. Please check credentials and try again.\n");
    }
}

void setup_gpio() {
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

    printf("INFO: GPIO setup complete. All pins configured successfully.\n");
}
void loop_pump(void *param) {
    QueueHandle_t queue = (QueueHandle_t)param;
    water_level_t level;
    while (1) {
        if (xQueuePeek(queue, &level, 0) == pdTRUE) {
            xQueueReceive(queue, &level, portMAX_DELAY);
            if (level == LOW_FOUR) {
                printf("WARNING: CRITICAL LEVEL detected! Pump is turned OFF to prevent overflow.\n");
                gpio_set_level(PUMP, 0);
                vTaskDelay(100 / portTICK_PERIOD_MS); // Add small delay when in critical level
                continue;
            }
        }
        gpio_set_level(PUMP, 1);
        printf("INFO: Pump is ON. Water is being pumped.\n");
        vTaskDelay(ON_TIME / portTICK_PERIOD_MS);
        gpio_set_level(PUMP, 0);
        printf("INFO: Pump is OFF. Waiting for next cycle.\n");
        vTaskDelay(OFF_TIME / portTICK_PERIOD_MS);
    }
}

void loop_level_water(void *param) {
    QueueHandle_t dataSendQueue = (QueueHandle_t)param;
    water_level_t level;
    while (true) {
        if (gpio_get_level(LEVEL_FOUR)) {
            printf("INFO: LEVEL FOUR detected. Sending alert to pump control.\n");
            level = LOW_FOUR;
            xQueueSend(dataSendQueue, &level, 0);
        } else if (gpio_get_level(LEVEL_THREE)) {
            printf("INFO: LEVEL THREE detected.\n");
            level = LOW_THREE;
            xQueueSend(dataSendQueue, &level, 0);
        } else if (gpio_get_level(LEVEL_TWO)) {
            printf("INFO: LEVEL TWO detected.\n");
            level = LOW_TWO;
            xQueueSend(dataSendQueue, &level, 0);
        } else if (gpio_get_level(LEVEL_ONE)) {
            printf("INFO: LEVEL ONE detected.\n");
            level = LOW_ONE;
            xQueueSend(dataSendQueue, &level, 0);
        }
        // Add delay to prevent watchdog trigger and yield CPU time
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup() {
    setup_gpio();
    setup_wifi();

    QueueHandle_t water_level = xQueueCreateStatic(CONFIG_DATASEND_QUEUE_SIZE, DATASEND_QUEUE_ITEM_SIZE, water_level_storage, &water_level_buffer);

    if (xTaskCreate(loop_level_water, "loop_level_water", 4096, (void *)water_level, 1, NULL) != pdPASS) {
        printf("ERROR: Failed to create loop_level_water task. Check system resources.\n");
    }

    if (xTaskCreate(loop_pump, "loop_pump", 4096, (void *)water_level, 2, NULL) != pdPASS) {
        printf("ERROR: Failed to create loop_pump task. Check system resources.\n");
    }
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        printf("INFO: New client connected to the server.\n");
    }
}
