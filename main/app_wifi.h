#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

void app_wifi_initialise();
bool app_wifi_wait_connected();
void app_wifi_start();
void app_wifi_stop();
void app_wifi_get_mac(char *macStr);

#endif
