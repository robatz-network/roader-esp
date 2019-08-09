#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "leds.h"

#define RST_PIN 22
#define SS_PIN 21
#define VOLTAGE_PIN 4

#define LED_RED 16
#define LED_GREEN 17
#define LED_BLUE 5

#define WIFI_SSID "PAVELTYK"
#define WIFI_PASS "veronica1"
#define AUTO_FLUSH_INTERVAL (60000UL)

MFRC522 mfrc522(SS_PIN, RST_PIN);

struct RfidScanEvent
{
    String uid;
    unsigned long time;
};

struct RfidScanEvent rfidScanEvents[200];
int nextRfidScanEventIdx = 0;
unsigned long lastFlushAttemptAt = 0;

void checkRFIDScanner(void *parameter);
void flushData(void *parameter);
String formatRfidScanEvent(const RfidScanEvent &wpt);
String formatBatteryVoltage(const uint16_t voltage);
uint16_t getBatteryVoltage();

void checkRfidScannerTask(void *parameter);
void flushDataTask(void *parameter);

void setup()
{
    SPI.begin();
    Serial.begin(115200);
    mfrc522.PCD_Init();
    mfrc522.PCD_DumpVersionToSerial();

    WiFi.mode(WIFI_STA);
    setupLed(LED_RED);
    setupLed(LED_GREEN);
    setupLed(LED_BLUE);
    pinMode(VOLTAGE_PIN, INPUT);

    xTaskCreate(&flushDataTask, "flushDataTask", 8192, NULL, 1, NULL);
    xTaskCreate(&checkRfidScannerTask, "checkRfidScannerTask", 8192, NULL, 5, NULL);
}

void loop()
{
}

void checkRfidScannerTask(void *parameter)
{
    for (;;)
    {
        checkRfidScanner();
        vTaskDelay(100);
    }
    vTaskDelete(NULL);
}

void flushDataTask(void *parameter)
{
    for (;;)
    {
        if (lastFlushAttemptAt == 0 || millis() - lastFlushAttemptAt >= AUTO_FLUSH_INTERVAL)
        {
            flushData();
        }
        vTaskDelay(1000);
    }
    vTaskDelete(NULL);
}

void checkRfidScanner()
{
    for (;;)
    {
        if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
        {
            return;
        }

        String uidString;

        for (int i = 0; i < 4; i++)
        {
            uidString += String(mfrc522.uid.uidByte[i]);
        }

        Serial.println("RFID Scanned: " + uidString);
        blinkLed(LED_GREEN, 500);

        if (nextRfidScanEventIdx == 0 || rfidScanEvents[nextRfidScanEventIdx - 1].uid != uidString)
        {
            rfidScanEvents[nextRfidScanEventIdx].uid = uidString;
            rfidScanEvents[nextRfidScanEventIdx].time = millis();
            nextRfidScanEventIdx++;
        }

        if (nextRfidScanEventIdx > 0)
        {
            flushData();
        }
    }
}

bool connectWifi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int timeout = 2000;

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        timeout -= 250;
        if (timeout <= 0)
        {
            WiFi.disconnect();
            return false;
        }
    }

    return true;
}

void flushData()
{
    onLed(LED_BLUE);
    lastFlushAttemptAt = millis();

    if (!connectWifi())
    {
        blinkLed(LED_BLUE, 500);
        blinkLed(LED_RED, 250, 3);
        return;
    }

    int httpResponseCode;
    HTTPClient http;
    http.begin("http://roader.herokuapp.com/api/devices/flush");
    http.addHeader("Content-Type", "text/plain");
    String data;

    for (int i = 0; i < nextRfidScanEventIdx; i++)
    {
        data += formatRfidScanEvent(rfidScanEvents[i]) + "\n";
    }

    data += formatBatteryVoltage(getBatteryVoltage()) + "\n";
    httpResponseCode = http.POST(data);
    http.end();
    WiFi.disconnect();
    offLed(LED_BLUE);

    if (httpResponseCode == 200)
    {
        nextRfidScanEventIdx = 0;
        blinkLed(LED_GREEN, 250, 3);
    }
    else
    {
        blinkLed(LED_RED, 250, 3);
    }
}

uint16_t getBatteryVoltage()
{
    return analogRead(VOLTAGE_PIN);
}

String formatRfidScanEvent(const RfidScanEvent &wpt)
{
    return "rfid_scanned\t" + WiFi.macAddress() + "\t" + wpt.uid + "\t" + wpt.time;
}

String formatBatteryVoltage(const uint16_t voltage)
{
    return "voltage\t" + WiFi.macAddress() + "\t" + voltage + "\t" + millis();
}
