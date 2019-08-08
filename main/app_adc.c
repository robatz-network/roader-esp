#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF 1100
#define NO_OF_SAMPLES 64

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

void app_adc_initialize(void)
{
  if (unit == ADC_UNIT_1)
  {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, atten);
  }
  else
  {
    adc2_config_channel_atten((adc2_channel_t)channel, atten);
  }

  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

uint32_t read_adc(void)
{
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++)
  {
    if (unit == ADC_UNIT_1)
    {
      adc_reading += adc1_get_raw((adc1_channel_t)channel);
    }
    else
    {
      int raw;
      adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
      adc_reading += raw;
    }
  }
  adc_reading /= NO_OF_SAMPLES;

  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
  return voltage;
}
