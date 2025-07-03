#pragma once

#include "header_file.h"
#include "_eeprom.h"

// ===================================================

bool engine_run_flag = false; // флаг запуска двигателя

// ===================================================

xTaskHandle xTask_leds;

xSemaphoreHandle xSemaphore_relays = NULL;

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
  engine_run_flag = _flag;
}

bool getEngineRunFlag()
{
  return engine_run_flag;
}

void setRelayState(RelayState _rel, uint8_t state)
{
  if (_rel != RELAY_LB)
  {
    if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
    {
      digitalWrite(RELAY_FOR_PL_PIN, state);
      xSemaphoreGive(xSemaphore_relays);
    }
  }
  if (_rel != RELAY_PL)
  {
    if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
    {
      digitalWrite(RELAY_FOR_LB_PIN, state);
      xSemaphoreGive(xSemaphore_relays);
    }
  }
}

uint8_t getRelayState(RelayState _rel)
{
  uint8_t state = LOW;

  switch (_rel)
  {
  case RELAY_LB:
    if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
    {
      state = digitalRead(RELAY_FOR_LB_PIN);
      xSemaphoreGive(xSemaphore_relays);
    }
    break;
  case RELAY_PL:
    if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
    {
      state = digitalRead(RELAY_FOR_PL_PIN);
      xSemaphoreGive(xSemaphore_relays);
    }
    break;
  default:
    break;
  }
  return state;
}

void semaphoreInit()
{
  xSemaphore_relays = xSemaphoreCreateMutex();
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
