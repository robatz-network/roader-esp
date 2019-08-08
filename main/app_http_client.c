#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "app_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512

static const char *TAG = "HTTP_CLIENT";

void http_post(const char data[])
{
  esp_http_client_config_t config = {
      .url = "http://roader.herokuapp.com/api/devices/flush",
      .method = HTTP_METHOD_POST,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);

  const char *post_data = data;
  esp_http_client_set_post_field(client, post_data, strlen(post_data));
  esp_http_client_set_header(client, "Content-Type", "text/plain");
  esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK)
  {
    ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
             esp_http_client_get_status_code(client),
             esp_http_client_get_content_length(client));
  }
  else
  {
    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
  }
  esp_http_client_cleanup(client);
}
