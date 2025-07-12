#pragma once
#include "header_file.h"

// ===================================================

void btnCheck(void *pvParameters)
{
  while (1)
  {
    switch (btnMode.getButtonState())
    {
    case BTN_ONECLICK: // короткий клик включает автоматический режим работы
      if (getCurrentMode() != SLS_MODE_AUTO)
      {
        setCurrentMode(SLS_MODE_AUTO);
      }
      break;
    case BTN_LONGCLICK: // длинный клик выключает автоматический режим работы
      if (getCurrentMode() != SLS_MODE_MANUAL)
      {
        setCurrentMode(SLS_MODE_MANUAL);
      }
      break;
    case BTN_DBLCLICK: // двойной клик включает/выключает WiFi модуль
      if (getWiFiState() == SLS_WIFI_OFF)
      {
        setWiFiState(SLS_WIFI_CONNECT);
      }
      else
      {
        setWiFiState(SLS_WIFI_OFF);
      }
      break;
    }
    vTaskDelay(1);
  }

  vTaskDelete(NULL);
}

void setLeds(void *pvParameters)
{

  uint8_t num = 0;

  while (1)
  {
    if (getCurrentMode() != SLS_MODE_AUTO)
    {
      leds[0] = CRGB::Red; // в неавтоматическом режиме светится красным
    }
    else
    {
      // в автоматическом режиме светится: зеленым - двигатель не запущен; синим - двигатель запущен, включен ближний свет; оранжевым - двигатель запущен, ближний свет выключен
      leds[0] = (!getEngineRunFlag()) ? CRGB::Green
                                      : ((getRelayState(SLS_RELAY_LB)) ? CRGB::Blue
                                                                       : CRGB::Orange);
    }

    // если включен WiFi, светодиод мигает с частотой 1 Гц
    if (wifi_state != SLS_WIFI_OFF)
    {
      uint8_t max_num = (wifi_state == SLS_WIFI_AP) ? 20 : 2;
      if (num >= max_num / 2)
      {
        leds[0] = CRGB::Black;
      }

      if (++num >= max_num)
      {
        num = 0;
      }
    }
    else
    {
      num = 0;
    }

    FastLED.show();

    vTaskDelay(50);
  }
  vTaskDelete(NULL);
}

void lightSensorCheck(void *pvParameters)
{
  uint16_t sensor_data = analogRead(LIGHT_SENSOR_PIN);
  uint16_t t;
  bool timer = false;
  uint16_t timer_counter = 0;

  const uint32_t SLS_DELAY = 20ul;

  while (1)
  {
    sensor_data = (sensor_data * 3 + analogRead(LIGHT_SENSOR_PIN)) / 4;
    t = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD);

    // тут же управление светом в автоматическом режиме
    if (getCurrentMode() == SLS_MODE_AUTO && getEngineRunFlag())
    {
      if (sensor_data <= t)
      { // если уровень снизился до порога включения БС, то включить БС и сбросить флаг отключения БС
        setRelayState(SLS_RELAY_ALL, true);
        timer = false;
      }
      else if (sensor_data > (t + LIGHT_SENSOR_THRESHOLD_HISTERESIS))
      { // если уровень превысил порог включения БС, реле БС включено, а флаг отключения БС еще не поднят, поднять его
        if (getRelayState(SLS_RELAY_LB) && !timer)
        {
          timer = true;
          timer_counter = 0;
        }
      }

      // тут же управление таймером отключения БС, если поднят флаг таймера
      if (timer)
      {
        if (timer_counter >= read_eeprom_8(EEPROM_INDEX_FOR_TURN_OFF_DELAY) * 1000ul / SLS_DELAY)
        {
          setRelayState(SLS_RELAY_ALL, false);
          timer = false;
        }
        else
        {
          timer_counter++;
        }
      }
    }
    else if (getRelayState(SLS_RELAY_LB))
    {
      setRelayState(SLS_RELAY_ALL, false);
    }

    // и здесь же управление яркостью светодиода - вне зависимость от режима работы
    if (sensor_data <= t)
    {
      FastLED.setBrightness(MIN_LED_BRIGHTNESS);
    }
    else if (sensor_data > (t + LIGHT_SENSOR_THRESHOLD_HISTERESIS))
    {
      FastLED.setBrightness(MAX_LED_BRIGHTNESS);
    }

    vTaskDelay(SLS_DELAY);
  }
  vTaskDelete(NULL);
}

void engineRunCheck(void *pvParameters)
{
  while (1)
  {
    if (!getEngineRunFlag())
    {
      if (digitalRead(ENGINE_RUN_PIN))
      { // поднимать флаг запуска двигателя и, соответственно, включать БС только по истечении времени задержки;
        vTaskDelay(read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY) * 1000ul);
        setEngineRunFlag(true);
      }
    }

    vTaskDelay(100);
  }
  vTaskDelete(NULL);
}

void startSleepMode(void *pvParameters)
{
  uint16_t timer = 0;
  bool _flag = false;

  while (1)
  {
    if (!_flag)
    {
      // если флаг сброшен, а зажигание выключено - флаг поднять и обнулить счетчик
      if (!digitalRead(IGNITION_PIN))
      {
        _flag = true;
        timer = 0;
      }
    }
    else
    {
      // если флаг поднят, а зажигание включено - сбросить флаг
      if (digitalRead(IGNITION_PIN))
      {
        _flag = false;
      }
    }

    // если флаг поднят, отсчитываем заданный интервал и уходим в сон
    if (_flag)
    {
      if (timer >= read_eeprom_16(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY) * 10)
      {
        // здесь делаем подготовку ко сну
        vTaskSuspend(xTask_leds);
        leds[0] = CRGB::Black;
        FastLED.show();

        uint64_t wakeup_pin_mask = 1;
        for (uint8_t i = 0; i < IGNITION_PIN; i++)
        {
          wakeup_pin_mask *= 2;
        }
        esp_deep_sleep_enable_gpio_wakeup(wakeup_pin_mask, ESP_GPIO_WAKEUP_GPIO_HIGH);
        esp_deep_sleep_start();
      }
      else
      {
        timer++;
      }
    }

    vTaskDelay(100);
  }
  vTaskDelete(NULL);
}

void wifiModuleManagement(void *pvParameters)
{
  uint32_t slsDelay = 100ul;

  while (1)
  {
    switch (wifi_state)
    {
    case SLS_WIFI_CONNECT:
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(IPAddress(read_eeprom_32(EEPROM_INDEX_FOR_AP_IP)),
                        IPAddress(read_eeprom_32(EEPROM_INDEX_FOR_AP_IP)),
                        IPAddress(255, 255, 255, 0));
      if (WiFi.softAP(read_string_from_eeprom(EEPROM_INDEX_FOR_AP_SSID, 32),
                      read_string_from_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, 64)))
      {
        slsDelay = 1ul; // в режиме точки доступа крутим быстро для нормальной реакции сервера
        HTTP.begin();
        setWiFiState(SLS_WIFI_AP);
      }
      break;
    case SLS_WIFI_OFF:
      if (WiFi.getMode() != WIFI_OFF)
      {
        HTTP.stop();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        slsDelay = 100ul;
      }
      break;
    case SLS_WIFI_AP:
      HTTP.handleClient();
      break;
    }

    vTaskDelay(slsDelay);
  }
  vTaskDelete(NULL);
}
