
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Arduino.h>
#include <WiFi.h>
#include "esp_log.h"
#include <esp32-hal-log.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <PicoSyslog.h>
static const char *TAG = "main";
#define HOSTNAME "hydroponic-controller"
std::string my_ssid = "";
std::string my_password = "";
PicoSyslog::Logger syslog("esp");  // Use "esp" as the name/tag for syslog
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
typedef enum
{
    LOW_ONE = 1,
    LOW_TWO,
    LOW_THREE,
    LOW_FOUR
} water_level_t;

// Queue Storage
static StaticQueue_t water_level_buffer;
uint8_t water_level_storage[CONFIG_DATASEND_QUEUE_SIZE * DATASEND_QUEUE_ITEM_SIZE];

void load_credentials()
{
    ESP_LOGI(TAG, "Loading WiFi credentials");
    if (my_ssid == "" || my_password == "")
    {
        return;
    }
    else
    {
        Preferences preferences;
        String ssid;
        String pass;
        preferences.begin("wifi", true);
        preferences.getString("my_ssid", ssid);
        preferences.getString("my_password", pass);
        my_ssid = ssid.c_str();
        my_password = pass.c_str();
        ESP_LOGI(TAG, "Loaded WiFi credentials. SSID: %s, Password: %s", my_ssid.c_str(), my_password.c_str()); // TODO: удалить
        preferences.end();
    }
}

void save_credentials()
{
    ESP_LOGI(TAG, "Saving WiFi credentials");
    if (my_ssid != "" || my_password != "")
    {
        ESP_LOGI(TAG, "Saving WiFi credentials. SSID: %s, Password: %s", my_ssid.c_str(), my_password.c_str()); // TODO: удалить
        Preferences preferences;
        preferences.begin("wifi", false);
        preferences.putString("my_ssid", my_ssid.c_str());
        preferences.putString("my_password", my_password.c_str());
        preferences.end();
    }
}

void setup_wifi()
{
    ESP_LOGI(TAG, "Starting WiFi in AP mode. SSID: %s", my_ssid.c_str());
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(my_ssid.c_str(), my_password.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        ESP_LOGI(TAG, "Connecting to WiFi..");
    }
    ESP_LOGI(TAG, "WiFi connected. IP address: %s", WiFi.localIP().toString().c_str());
}

void setup_ota()
{

    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPort(3232);
    ArduinoOTA
        .onStart([]()
                 {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        ESP_LOGI(TAG, "Start updating %s", type.c_str()); })
        .onEnd([]()
               {
        ESP_LOGI(TAG, "End");
        ESP.restart(); })
        .onProgress([](unsigned int progress, unsigned int total)
                    { ESP_LOGI(TAG, "Progress: %u%%\r", (progress / (total / 100))); })
        .onError([](ota_error_t error)
                 {
        ESP_LOGE(TAG, "Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            ESP_LOGE(TAG, "Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            ESP_LOGE(TAG, "Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            ESP_LOGE(TAG, "Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            ESP_LOGE(TAG, "Receive Failed");
        else if (error == OTA_END_ERROR)
            ESP_LOGE(TAG, "End Failed"); });

    ArduinoOTA.begin();
}

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

void loop_pump(void *param)
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
        vTaskDelay(ON_TIME / portTICK_PERIOD_MS);
        gpio_set_level(PUMP, 0);
        ESP_LOGI(TAG, "Pump is OFF. Waiting for next cycle.");
        vTaskDelay(OFF_TIME / portTICK_PERIOD_MS);
    }
}

void loop_level_water(void *param)
{
    QueueHandle_t dataSendQueue = (QueueHandle_t)param;
    water_level_t level;
    while (true)
    {
        if (gpio_get_level(LEVEL_FOUR))
        {
            ESP_LOGI(TAG, "LEVEL FOUR detected. Sending alert to pump control.");
            level = LOW_FOUR;
            xQueueSend(dataSendQueue, &level, 0);
        }
        else if (gpio_get_level(LEVEL_THREE))
        {
            ESP_LOGI(TAG, "LEVEL THREE detected.");
            level = LOW_THREE;
            xQueueSend(dataSendQueue, &level, 0);
        }
        else if (gpio_get_level(LEVEL_TWO))
        {
            ESP_LOGI(TAG, "LEVEL TWO detected.");
            level = LOW_TWO;
            xQueueSend(dataSendQueue, &level, 0);
        }
        else if (gpio_get_level(LEVEL_ONE))
        {
            ESP_LOGI(TAG, "LEVEL ONE detected.");
            level = LOW_ONE;
            xQueueSend(dataSendQueue, &level, 0);
        }
        // Add delay to prevent watchdog trigger and yield CPU time
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup()
{

    // Setup Logging
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    // Set CPU Frequency
    setCpuFrequencyMhz(100);

    // Initialize GPIO
    setup_gpio();

    // Initialize WiFi
    if (my_ssid.length() == 0 || my_password.length() == 0)
    {
        ESP_LOGI(TAG, "No WiFi credentials found. Starting in AP mode.");
        load_credentials();
    }
    else
    {
        ESP_LOGI(TAG, "WiFi credentials found. Starting in STA mode.");
        save_credentials();
    }

    setup_wifi();

    setup_ota();

    syslog.server = "192.168.3.246";

    QueueHandle_t water_level = xQueueCreateStatic(CONFIG_DATASEND_QUEUE_SIZE, DATASEND_QUEUE_ITEM_SIZE, water_level_storage, &water_level_buffer);

    // Controller water level
    if (xTaskCreate(loop_level_water, "loop_level_water", 4096, (void *)water_level, 1, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create loop_level_water task. Check system resources.");
    }

    // Pump
    if (xTaskCreate(loop_pump, "loop_pump", 4096, (void *)water_level, 2, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create loop_pump task. Check system resources.");
    }
}

void loop()
{
    ArduinoOTA.handle();
}
