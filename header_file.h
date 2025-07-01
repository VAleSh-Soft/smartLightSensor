#pragma once

#include <shButton.h>
#include <FastLED.h>

// ===================================================

constexpr uint8_t LIGHT_SENSOR_PIN = 1; // пин датчика света
constexpr uint8_t ENGINE_RUN_PIN = 7;   // пин, на который приходит сигнал с вывода D генератора или HIGH при запущенном двигателе
constexpr uint8_t BTN_MODE_PIN = 2;     // пин кнопки режима работы
constexpr uint8_t LEDS_DATA_PIN = 4;    // пин выхода для светодиодов
constexpr uint8_t RELAY_FOR_LB = 5;     // пин реле ближнего света

#define EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD 10 // индекс для хранения порога включения ближнего света, uint16_t
#define EEPROM_INDEX_FOR_CURRENT_MODE 12           // индекс для хранения текущего режима работы, uint8_t
#define EEPROM_INDEX_FOR_TURN_ON_DELAY 13          // индекс для хранения задержки включения ближнего света после запуска двигателя, uint8_t

// ===================================================

enum AutoLightMode : uint8_t
{
  MODE_OFF,
  MODE_AUTO
};

// ===================================================

class slsButton : public shButton
{
private:
public:
  slsButton(byte button_pin) : shButton(button_pin)
  {
    shButton::setVirtualClickOn(true);
    shButton::setTimeoutOfLongClick(1000);
  }
};

// ===================================================

slsButton btnMode(BTN_MODE_PIN); // кнопка режима работы

const uint8_t LEDS_NUM = 1;

CRGB leds[LEDS_NUM]; // индикаторный светодиод;
/*
 * ручной режим:
 *             красный цвет;
 * автоматический режим:
 *       двигатель заглушен:
 *                         зеленый цвет;
 *       двигатель запущен:
 *             ближний свет выключен:
 *                                оранжевый цвет;
 *             ближний свет включен:
 *                                синий цвет;
 */

AutoLightMode cur_mode = MODE_OFF; // текущий режим работы

bool engine_run_flag = false; // флаг запуска двигателя
uint16_t light_sensor_data;   // текущие показания датчика света

// ===================================================

// void set_cur_mode(AutoLightMode _mode);

// опрос кнопки
void btnCheck(void *pvParameters);
// управление светодиодами
void setLeds(void *pvParameters);
// отслеживание показаний датчика света
void lightSensorCheck(void *pvParameters);
// отслеживание момента запуска двигателя
void engineRunCheck(void *pvParameters);

// // отключение ближнего света через 30 секунд после превышения порога датчика света
// void turnOffLowBeam(void *pvParameters);

// ===================================================

/// @brief установка заданного состояния для заданного реле
/// @param rel заданное реле; 1 - ближний свет, 2 - ДХО; 0 - оба реле
/// @param state заданное состояние
void setRelayState(uint8_t rel, uint8_t state);

/// @brief начальный старт головного света при включении любого режима при запущенном двигателе
void runLightMode();
