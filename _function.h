#pragma once

#include "header_file.h"
#include "_eeprom.h"

// ===================================================

bool engine_run_flag = false;    // флаг запуска двигателя
WiFiState wifi_state = SLS_WIFI_OFF; // состояние WiFi

// ===================================================

xTaskHandle xTask_leds;

xSemaphoreHandle xSemaphore_relays = NULL;
xSemaphoreHandle xSemaphore_eng_run = NULL;
xSemaphoreHandle xSemaphore_wifi = NULL;

// ===================================================

void setCurrentMode(AutoLightMode _mode)
{
  write_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE, uint8_t(_mode));
}

AutoLightMode getCurrentMode()
{
  return (AutoLightMode)read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE);
}

void setEngineRunFlag(bool _flag)
{
  if (xSemaphoreTake(xSemaphore_eng_run, portMAX_DELAY) == pdTRUE)
  {
    engine_run_flag = _flag;
    xSemaphoreGive(xSemaphore_eng_run);
  }
}

bool getEngineRunFlag()
{
  bool _flag = false;
  if (xSemaphoreTake(xSemaphore_eng_run, portMAX_DELAY) == pdTRUE)
  {
    _flag = engine_run_flag;
    xSemaphoreGive(xSemaphore_eng_run);
  }
  return _flag;
}

void setRelayState(RelayState _rel, uint8_t state)
{
  if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
  {
    if (_rel != SLS_RELAY_LB)
    {
      digitalWrite(RELAY_FOR_PL_PIN, state);
    }
    if (_rel != SLS_RELAY_PL)
    {
      digitalWrite(RELAY_FOR_LB_PIN, state);
    }
    xSemaphoreGive(xSemaphore_relays);
  }
}

uint8_t getRelayState(RelayState _rel)
{
  uint8_t _state = LOW;
  if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
  {
    switch (_rel)
    {
    case SLS_RELAY_LB:
      _state = digitalRead(RELAY_FOR_LB_PIN);
      break;
    case SLS_RELAY_PL:
      _state = digitalRead(RELAY_FOR_PL_PIN);
      break;
    default:
      break;
    }
    xSemaphoreGive(xSemaphore_relays);
  }
  return _state;
}

void setWiFiState(WiFiState _state)
{
  if (xSemaphoreTake(xSemaphore_wifi, portMAX_DELAY) == pdTRUE)
  {
    wifi_state = _state;
    xSemaphoreGive(xSemaphore_wifi);
  }
}

WiFiState getWiFiState()
{
  WiFiState _state;
  {
    _state = wifi_state;
    xSemaphoreGive(xSemaphore_wifi);
  }
  return _state;
}

void semaphoreInit()
{
  xSemaphore_relays = xSemaphoreCreateMutex();
  xSemaphore_eng_run = xSemaphoreCreateMutex();
  xSemaphore_wifi = xSemaphoreCreateMutex();
}

// ===================================================

class slsButton : public shButton
{
private:
public:
  slsButton(byte button_pin) : shButton(button_pin)
  {
    shButton::setVirtualClickOn(true);
    shButton::setLongClickMode(LCM_ONLYONCE);
    shButton::setTimeoutOfLongClick(1000);
  }
};

slsButton btnMode(BTN_MODE_PIN); // кнопка выбора режима работы

// ===================================================
