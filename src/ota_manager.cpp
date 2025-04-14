#include "ota_manager.hpp"
#include "config.hpp"
#include "esp_log.h"

static const char *TAG = "ota_manager";

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

void handle_ota()
{
    ArduinoOTA.handle();
}