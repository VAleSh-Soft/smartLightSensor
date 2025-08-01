#pragma once

#include <EEPROM.h>
#include "header_file.h"

// ===================================================

#define EEPROM_SIZE 128

// ===================================================

void eeprom_init(bool _reset)
{
  EEPROM.begin(EEPROM_SIZE);

  if (_reset)
  {
    SLS_PRINTLN(F("Resetting the settings to default values"));
  }

  if (read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) > 4095 ||
      read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) == 0 || _reset)
  {
    write_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD, DEFAULT_LIGHT_SENSOR_THRESHOLD);
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE) > uint8_t(SLS_MODE_AUTO) || _reset)
  {
    write_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE, uint8_t(SLS_MODE_MANUAL));
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY) > MAX_TURN_ON_DELAY || _reset)
  {
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY, DEFAULT_TURN_ON_DELAY);
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY) > MAX_LB_SHUTDOWN_DELAY ||
      read_eeprom_8(EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY) < MIN_LB_SHUTDOWN_DELAY || _reset)
  {
    write_eeprom_8(EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY, DEFAULT_LB_SHUTDOWN_DELAY);
  }

  if (read_eeprom_16(EEPROM_INDEX_FOR_STARTING_SLEEP_DELAY) > MAX_RUN_SLEEP_DELAY || _reset)
  {
    write_eeprom_16(EEPROM_INDEX_FOR_STARTING_SLEEP_DELAY, DEFAULT_RUN_SLEEP_DELAY);
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_AP_SSID) > MAX_AP_SSID_LENGHT || _reset)
  {
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_SSID, DEFAULT_AP_SSID);
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_AP_PASSWORD) > MAX_AP_PASSWORD_LENGHT || _reset)
  {
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, DEFAULT_AP_PASSWORD);
  }

  if (read_eeprom_32(EEPROM_INDEX_FOR_AP_IP) == 0xffffffff ||
      read_eeprom_32(EEPROM_INDEX_FOR_AP_IP) == 0 || _reset)
  {
    write_eeprom_32(EEPROM_INDEX_FOR_AP_IP, (uint32_t)IPAddress(DEFAULT_AP_IP));
  }

  if (read_eeprom_8(EEPROM_INDEX_FOR_LED_BRIGHTNESS_LEVEL) > 10 ||
      read_eeprom_8(EEPROM_INDEX_FOR_LED_BRIGHTNESS_LEVEL) == 0 || _reset)
  {
    write_eeprom_8(EEPROM_INDEX_FOR_LED_BRIGHTNESS_LEVEL, 10);
  }
}

// ===================================================

uint8_t read_eeprom_8(uint16_t _index)
{
  uint8_t _data = 0;
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    _data = EEPROM.read(_index);
    xSemaphoreGive(xSemaphore_eeprom);
  }
  return _data;
}

void write_eeprom_8(uint16_t _index, uint8_t _data)
{
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    if (EEPROM.read(_index) != _data)
    {
      EEPROM.write(_index, _data);
    }
    EEPROM.commit();
    xSemaphoreGive(xSemaphore_eeprom);
  }
}

uint16_t read_eeprom_16(uint16_t _index)
{
  uint16_t _data;
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    EEPROM.get(_index, _data);
    xSemaphoreGive(xSemaphore_eeprom);
  }
  return (_data);
}

void write_eeprom_16(uint16_t _index, uint16_t _data)
{
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    EEPROM.put(_index, _data);
    EEPROM.commit();
    xSemaphoreGive(xSemaphore_eeprom);
  }
}

uint32_t read_eeprom_32(uint16_t _index)
{
  uint32_t _data;
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    EEPROM.get(_index, _data);
    xSemaphoreGive(xSemaphore_eeprom);
  }
  return (_data);
}

void write_eeprom_32(uint16_t _index, uint32_t _data)
{
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    EEPROM.put(_index, _data);
    EEPROM.commit();
    xSemaphoreGive(xSemaphore_eeprom);
  }
}

void write_string_to_eeprom(uint16_t _index, const char *_string)
{
  uint8_t len = strlen(_string);
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    EEPROM.write(_index, len); // сохраняем размер строки - первый байт
    _index++;                  // саму строку пишем следом за размером

    for (uint16_t i = 0; i < len && (_index + i) < EEPROM_SIZE; i++) // собственно, пишем
    {
      EEPROM.write(_index + i, _string[i]);
    }
    EEPROM.commit();
    xSemaphoreGive(xSemaphore_eeprom);
  }
}

char *read_string_from_eeprom(uint16_t _index, const uint8_t _max_len)
{
  if (xSemaphoreTake(xSemaphore_eeprom, portMAX_DELAY) == pdTRUE)
  {
    uint8_t len = EEPROM.read(_index); // считываем размер строки
    if (len >= _max_len)
    {
      len = _max_len;
    }

    char *data = (char *)calloc(len + 1, sizeof(char)); // выделяем память под строку - размер строки + нулевой символ в конце
    if (data)
    {
      _index++; // пропускаем первый байт - там записан размер строки

      for (uint16_t i = 0; i < len && (_index + i) < EEPROM_SIZE; i++) // читаем строку
      {
        data[i] = EEPROM.read(_index + i);
      }
      data[len] = '\0'; // добавляем нулевой символ в конец строки
    }
    xSemaphoreGive(xSemaphore_eeprom);
    return data;
  }
  else
  {
    return nullptr;
  }
}
