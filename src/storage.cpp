#include "storage.hpp"
#include "wifi_manager.hpp"
#include "secrets.hpp"
#include "esp_log.h"

static const char *TAG = "storage";

void init_storage()
{
    NVS.begin("hydroponics");
}

void load_credentials()
{
    ESP_LOGI(TAG, "Loading WiFi credentials");
    auto res = NVS.getString("my_ssid", my_ssid);
    if (!res)
    {
        ESP_LOGE(TAG, "Failed to load SSID");
    }
    res = NVS.getString("my_password", my_password);
    if (!res)
    {
        ESP_LOGE(TAG, "Failed to load password");
    }
}

void save_credentials()
{
    ESP_LOGI(TAG, "Saving WiFi credentials");
    auto res = NVS.setString("my_ssid", my_ssid);
    if (!res)
    {
        ESP_LOGE(TAG, "Failed to save SSID");
    }
    res = NVS.setString("my_password", my_password);
    if (!res)
    {
        ESP_LOGE(TAG, "Failed to save password");
    }
}

String get_wifi_ssid() {
    return my_ssid;
}

String get_wifi_password() {
    return my_password;
}