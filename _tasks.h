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
  const uint32_t LED_DELAY = 50ul;

  while (1)
  {
    CRGB col;
    if (getCurrentMode() != SLS_MODE_AUTO)
    {
      col = CRGB::Red; // в неавтоматическом режиме светится красным
    }
    else
    {
      // в автоматическом режиме светится: зеленым - двигатель не запущен; синим - двигатель запущен, включен ближний свет; оранжевым - двигатель запущен, ближний свет выключен
      col = (!getEngineRunFlag()) ? CRGB::Green
                                  : ((getRelayState(SLS_RELAY_LB)) ? CRGB::Blue
                                                                   : CRGB::Orange);
    }

    WiFiModuleState _state = getWiFiState();
    // при создании точки доступа светодиод мигает с частотой 10 Гц, в процессе работы точки доступа - 1 Гц
    if (_state != SLS_WIFI_OFF)
    {
      uint8_t max_num = (_state == SLS_WIFI_AP) ? 1000ul / LED_DELAY : 100ul / LED_DELAY;
      if (num >= max_num / 2)
      {
        col = CRGB::Black;
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

    fastLedShow(col);

    vTaskDelay(LED_DELAY);
  }
  vTaskDelete(NULL);
}

void lightSensorCheck(void *pvParameters)
{
  uint16_t sensor_data = analogRead(LIGHT_SENSOR_PIN);
  uint16_t t;
  bool timer = false;
  uint16_t timer_counter = 0;
  bool sensor_flag = false;

  const uint32_t LS_DELAY = 20ul;

  while (1)
  {
    sensor_data = (sensor_data * 3 + analogRead(LIGHT_SENSOR_PIN)) / 4;
    t = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD);

    if (sensor_data <= t)
    {
      if (sensor_flag)
      {
        SLS_PRINTLN(F("The light sensor is below the switching threshold"));
      }
      sensor_flag = false;
    }
    else if (sensor_data > (t + LIGHT_SENSOR_THRESHOLD_HISTERESIS))
    {
      if (!sensor_flag)
      {
        SLS_PRINTLN(F("The light sensor is above the switching threshold"));
      }
      sensor_flag = true;
    }

    // тут же управление светом в автоматическом режиме
    if (getCurrentMode() == SLS_MODE_AUTO && getEngineRunFlag())
    {
      if (!sensor_flag)
      {
        // если уровень снизился до порога включения БС, то сбросить флаг отключения БС и включить БС
        timer = false;
        if (!getRelayState(SLS_RELAY_LB) && getIgnitionState())
        {
          setRelayState(SLS_RELAY_ALL, true);
          SLS_PRINTLN(F("The low beam is ON"));
        }
      }
      else
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
        if (timer_counter >= read_eeprom_8(EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY) * 1000ul / LS_DELAY)
        {
          setRelayState(SLS_RELAY_ALL, false);
          timer = false;
          SLS_PRINTLN(F("The low beam is OFF"));
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
      SLS_PRINTLN(F("The low beam is OFF"));
    }

    // и здесь же управление яркостью светодиода - вне зависимость от режима работы
    uint8_t br = (sensor_flag) ? MAX_LED_BRIGHTNESS : MIN_LED_BRIGHTNESS;
    setLedBrightness(br);

    vTaskDelay(LS_DELAY);
  }
  vTaskDelete(NULL);
}

void engineRunCheck(void *pvParameters)
{
  while (1)
  {
    if (!getEngineRunFlag())
    {
      if (digitalRead(ENGINE_RUN_PIN) && getIgnitionState())
      {
        // поднимать флаг запуска двигателя и, соответственно, включать БС только по истечении времени задержки;
        SLS_PRINTLN(F("The engine is running"));
        vTaskDelay(read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY) * 1000ul);
        setEngineRunFlag(true);
#if USE_RELAY_FOR_DRL
        setRelayState(SLS_RELAY_ALL, false); // это чтобы сразу включилось реле ДХО
#endif
      }
    }

    vTaskDelay(100);
  }
  vTaskDelete(NULL);
}

void checkingForSleepMode(void *pvParameters)
{
  uint16_t timer = 0;
  bool _flag = false;

  while (1)
  {
    if (!_flag)
    {
      // если флаг сброшен, а зажигание выключено - флаг поднять и обнулить счетчик
      if (!getIgnitionState())
      {
        _flag = true;
        timer = 0;
        SLS_PRINTLN(F("The ignition is OFF"));
        SLS_PRINTLN(F("Pause before turning ON sleep mode"));
      }
    }
    else
    {
      // если флаг поднят, а зажигание включено - сбросить флаг
      if (getIgnitionState())
      {
        _flag = false;
        SLS_PRINTLN(F("The ignition is ON"));
        SLS_PRINTLN(F("Canceling sleep mode activation"));
      }
    }

    // если флаг поднят, отсчитываем заданный интервал и уходим в сон
    if (_flag)
    {
      if (timer >= read_eeprom_16(EEPROM_INDEX_FOR_STARTING_SLEEP_DELAY) * 10)
      {
        startSleep();
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

