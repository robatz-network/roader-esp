#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "app_wifi.h"
#include "app_http_client.h"
#include "app_adc.h"

#define AUTO_FLUSH_INTERVAL 15000

static void flush_data_task(void *pvParameters);
void app_nvs_flash_initialize();

void app_main()
{

    app_nvs_flash_initialize();
    app_wifi_initialize();
    app_adc_initialize();

    xTaskCreate(&flush_data_task, "flush_data_task", 8192, NULL, 5, NULL);
}

void flush_data()
{
    app_wifi_start();

    if (app_wifi_wait_connected())
    {
        uint32_t voltage = read_adc();
        char buff[50];
        char macStr[19] = {0};
        unsigned long time = esp_timer_get_time() / 1000ULL;

        ESP_ERROR_CHECK(app_wifi_get_mac(macStr));
        sprintf(buff, "voltage %s %dmV %ld", macStr, voltage, time);
        int status_code = http_post(buff);

        if (status_code == 200)
        {
            // TODO: move uploadedIdx
        }
    }

    app_wifi_stop();
}

static void flush_data_task(void *pvParameters)
{
    for (;;)
    {
        flush_data();
        vTaskDelay(pdMS_TO_TICKS(AUTO_FLUSH_INTERVAL));
    }

    // vTaskDelete(NULL);
}

void app_nvs_flash_initialize()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}
