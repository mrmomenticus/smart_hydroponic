#include "wifi_manager.hpp"
#include "config.hpp"
#include "display.hpp"
#include "storage.hpp"
#include "esp_log.h"

static const char *TAG = "wifi_manager";

void setup_wifi()
{
    auto password = get_wifi_password();
    auto ssid = get_wifi_ssid();
    ESP_LOGI(TAG, "Starting WiFi in AP mode. SSID: %s", ssid.c_str());
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        ESP_LOGI(TAG, "Connecting to WiFi..");
        display_message("Connecting to WiFi...");
    }
    auto ip = WiFi.localIP().toString();
    ESP_LOGI(TAG, "WiFi connected. IP address: %s", ip.c_str());
    display_message(("IP: " + ip).c_str());
    delay(2500);
}