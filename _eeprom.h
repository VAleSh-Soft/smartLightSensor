#pragma once

#include <EEPROM.h>
#include "header_file.h"

// ===================================================

#define EEPROM_SIZE 128

// ===================================================

void eeprom_init();
uint8_t read_eeprom_8(uint16_t _index);
void write_eeprom_8(uint16_t _index, uint8_t _data);
uint16_t read_eeprom_16(uint16_t _index);
void write_eeprom_16(uint16_t _index, uint16_t _data);

// ===================================================

void eeprom_init()
{
  EEPROM.begin(EEPROM_SIZE);

  if (read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) > 1023 ||
      read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) == 0)
  {
    write_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD, 3000);
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE) > uint8_t(MODE_AUTO))
  {
    write_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE, uint8_t(MODE_MANUAL));
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY) > 10)
  {
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY, 3);
  }

  if (read_eeprom_16(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY) > 60)
  {
    write_eeprom_16(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY, 10);
  }
}

// ===================================================

uint8_t read_eeprom_8(uint16_t _index)
{
  return EEPROM.read(_index);
}

void write_eeprom_8(uint16_t _index, uint8_t _data)
{
  if (EEPROM.read(_index) != _data)
  {
    EEPROM.write(_index, _data);
  }
  EEPROM.commit();
}

uint16_t read_eeprom_16(uint16_t _index)
{
  uint16_t _data;
  EEPROM.get(_index, _data);
  return (_data);
}

void write_eeprom_16(uint16_t _index, uint16_t _data)
{
  EEPROM.put(_index, _data);
  EEPROM.commit();
}
