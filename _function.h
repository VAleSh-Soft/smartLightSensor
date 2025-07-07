#pragma once

#include "header_file.h"
#include "_eeprom.h"

// ===================================================

bool engine_run_flag = false;        // флаг запуска двигателя
WiFiModuleState wifi_state = SLS_WIFI_OFF; // состояние WiFi

// ===================================================

void setCurrentMode(AutoLightSensorMode _mode)
{
  write_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE, uint8_t(_mode));
}

AutoLightSensorMode getCurrentMode()
{
  return (AutoLightSensorMode)read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE);
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

void setRelayState(RelayState _rel, bool _state)
{
  if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
  {
    if (_rel == SLS_RELAY_PL || _rel == SLS_RELAY_ALL)
    {
      digitalWrite(RELAY_FOR_PL_PIN,
                   (CONTROL_LEVEL_FOR_PL ? (uint8_t)_state : (uint8_t)!_state));
    }
    if (_rel == SLS_RELAY_LB || _rel == SLS_RELAY_ALL)
    {
      digitalWrite(RELAY_FOR_LB_PIN,
                   (CONTROL_LEVEL_FOR_LB ? (uint8_t)_state : (uint8_t)!_state));
    }
#if USE_RELAY_FOR_DRL
#if USE_DRL_MANAGEMENT
    if (_rel == SLS_RELAY_DRL)
    {
      digitalWrite(RELAY_FOR_DRL_PIN,
                   (CONTROL_LEVEL_FOR_DRL ? (uint8_t)_state : (uint8_t)!_state));
    }
    else
    {
      bool x = (digitalRead(RELAY_FOR_PL_PIN) == CONTROL_LEVEL_FOR_PL);
      digitalWrite(RELAY_FOR_DRL_PIN,
                   (CONTROL_LEVEL_FOR_DRL ? (uint8_t)!x : (uint8_t)x));
    }
#else
    if (getEngineRunFlag())
    {
      digitalWrite(RELAY_FOR_DRL_PIN, CONTROL_LEVEL_FOR_DRL);
    }
#endif
#endif
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
      _state = (digitalRead(RELAY_FOR_LB_PIN) == CONTROL_LEVEL_FOR_LB);
      break;
    case SLS_RELAY_PL:
      _state = (digitalRead(RELAY_FOR_PL_PIN) == CONTROL_LEVEL_FOR_PL);
      break;
#if USE_RELAY_FOR_DRL
    case SLS_RELAY_DRL:
      _state = (digitalRead(RELAY_FOR_DRL_PIN) == CONTROL_LEVEL_FOR_DRL);
      break;
#endif
    default:
      break;
    }
    xSemaphoreGive(xSemaphore_relays);
  }
  return _state;
}

void setWiFiState(WiFiModuleState _state)
{
  if (xSemaphoreTake(xSemaphore_wifi, portMAX_DELAY) == pdTRUE)
  {
    wifi_state = _state;
    xSemaphoreGive(xSemaphore_wifi);
  }
}

WiFiModuleState getWiFiState()
{
  WiFiModuleState _state;
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
