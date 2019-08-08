#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "app_wifi.h"
#include "app_http_client.h"
#include "app_adc.h"

static void http_post_task(void *pvParameters)
{
    for (;;)
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
            http_post(buff);
        }

        app_wifi_stop();
        vTaskDelay(pdMS_TO_TICKS(15000));
    }

    // vTaskDelete(NULL);
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    app_wifi_initialise();
    app_adc_initialize();

    xTaskCreate(&http_post_task, "http_post_task", 8192, NULL, 5, NULL);
}
