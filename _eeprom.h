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

  if (read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) > 4095 ||
      read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) == 0)
  {
    write_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD, DEFAULT_LIGHT_SENSOR_THRESHOLD);
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE) > uint8_t(MODE_AUTO))
  {
    write_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE, uint8_t(MODE_MANUAL));
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY) > MAX_TURN_ON_DELAY)
  {
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY, DEFAULT_TURN_ON_DELAY);
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_TURN_OFF_DELAY) > MAX_TURN_OFF_DELAY ||
      read_eeprom_8(EEPROM_INDEX_FOR_TURN_OFF_DELAY) < MIN_TURN_OFF_DELAY)
  {
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_OFF_DELAY, DEFAULT_TURN_OFF_DELAY);
  }

  if (read_eeprom_16(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY) > MAX_RUN_SLEEP_DELAY)
  {
    write_eeprom_16(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY, DEFAULT_RUN_SLEEP_DELAY);
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
