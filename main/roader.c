#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "app_wifi.h"
#include "app_http_client.h"

static void http_post_task(void *pvParameters)
{
    app_wifi_start();
    app_wifi_wait_connected();
    http_post("Hello!!!");
    app_wifi_stop();
    vTaskDelete(NULL);
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

    xTaskCreate(&http_post_task, "http_post_task", 8192, NULL, 5, NULL);
}
