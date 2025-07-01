#pragma once

#include "header_file.h"
#include "_eeprom.h"

// ===================================================

AutoLightMode cur_mode = MODE_MANUAL; // текущий режим работы
bool engine_run_flag = false;         // флаг запуска двигателя
uint16_t light_sensor_data;           // текущие показания датчика света

const uint16_t LIGHT_SENSOR_THRESHOLD_HISTERESIS = 200; // гистрезис порога датчика света
const uint8_t MAX_LED_BRIGHTNESS = 250;                 // максимальная яркость светодиода
const uint8_t MIN_LED_BRIGHTNESS = 50;                  // минимальная яркость светодиода

void setCurrentMode(AutoLightMode _mode)
{
  cur_mode = _mode;
  write_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE, uint8_t(_mode));

  // включаем/выключаем реле согласно текущего режима работы
  switch (cur_mode)
  {
  case MODE_AUTO:
    if (getEngineRunFlag())
    {
      uint16_t t, d;
      t = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD);
      d = getLightSensorData();
      if (d <= t)
      {
        setRelayState(RELAY_ALL, HIGH);
      }
      else if (d > t + LIGHT_SENSOR_THRESHOLD_HISTERESIS)
      {
        setRelayState(RELAY_ALL, LOW);
      }
    }
    break;
  default:
    setRelayState(RELAY_ALL, LOW);
    break;
  }
}

AutoLightMode getCurrentMode()
{
  return cur_mode;
}

void setEngineRunFlag(bool _flag)
{
  engine_run_flag = _flag;
}

bool getEngineRunFlag()
{
  return engine_run_flag;
}
uint16_t getLightSensorData()
{
  light_sensor_data = (light_sensor_data * 3 + analogRead(LIGHT_SENSOR_PIN)) / 4;
  return light_sensor_data;
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
