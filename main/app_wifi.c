#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "app_wifi.h"

static const char *TAG = "WIFI";

static EventGroupHandle_t wifi_event_group;

const int CONNECTED_BIT = BIT0;
const int DISCONNECTED_BIT = BIT1;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        xEventGroupSetBits(wifi_event_group, DISCONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void app_wifi_initialize()
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
}

void app_wifi_start()
{
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_wifi_stop()
{
    ESP_ERROR_CHECK(esp_wifi_stop());
}

bool app_wifi_wait_connected()
{
    EventBits_t ux_bits;
    unsigned long timeout = pdMS_TO_TICKS(5000);
    ux_bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT, true, false, timeout);

    return ((ux_bits & CONNECTED_BIT) != 0);
}

esp_err_t app_wifi_get_mac(char *macStr)
{
    uint8_t mac[6];
    esp_err_t err = esp_wifi_get_mac(WIFI_IF_STA, mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return err;
}
