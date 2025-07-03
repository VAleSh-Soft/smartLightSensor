#pragma once

#include "header_file.h"
#include "_eeprom.h"

// ===================================================

bool engine_run_flag = false; // флаг запуска двигателя

// ===================================================

xTaskHandle xTask_leds;

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
    digitalWrite(RELAY_FOR_PL_PIN, state);
  }
  if (_rel != RELAY_PL)
  {
    digitalWrite(RELAY_FOR_LB_PIN, state);
  }
}

uint8_t getRelayState(RelayState _rel)
{
  switch (_rel)
  {
  case RELAY_LB:
    return digitalRead(RELAY_FOR_LB_PIN);
  case RELAY_PL:
    return digitalRead(RELAY_FOR_PL_PIN);
  default:
    return LOW;
  }
}

void semaphoreInit()
{
  // xSemaphore_leds = xSemaphoreCreateBinary();
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
