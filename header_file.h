#pragma once

#include <shButton.h>
#include <FastLED.h>

// ===================================================

constexpr uint8_t LIGHT_SENSOR_PIN = 4; // пин датчика света
constexpr uint8_t IGNITION_PIN = 5;     // пин, на который приходит сигнал с линии зажигания; при появлении на этом пине высокого уровня МК выходит из глубокого сна, поэтому для esp32c3 допустимые значения 0..5
constexpr uint8_t ENGINE_RUN_PIN = 6;   // пин, на который приходит сигнал с вывода D генератора или HIGH при запущенном двигателе
constexpr uint8_t BTN_MODE_PIN = 3;     // пин кнопки режима работы
constexpr uint8_t LEDS_DATA_PIN = 7;    // пин выхода для светодиодов
constexpr uint8_t RELAY_FOR_LB = 8;     // пин реле ближнего света
constexpr uint8_t RELAY_FOR_PL = 9;     // пин реле габаритных огней

#define EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD 1 // индекс для хранения порога включения ближнего света, uint16_t
#define EEPROM_INDEX_FOR_CURRENT_MODE 3           // индекс для хранения текущего режима работы, uint8_t
#define EEPROM_INDEX_FOR_TURN_ON_DELAY 4          // индекс для хранения задержки включения ближнего света после запуска двигателя, uint8_t
#define EEPROM_INDEX_FOR_RUN_SLEEP_DELAY 5        // индекс для хранения задержки перехода в спящий режим после выключения зажигания, uint16_t

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
    shButton::setLongClickMode(LCM_ONLYONCE);
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

class SensorState
{
private:
  uint8_t sensor_pin;
  AutoLightMode cur_mode = MODE_OFF; // текущий режим работы
  bool engine_run_flag = false;      // флаг запуска двигателя
  uint16_t light_sensor_data;        // текущие показания датчика света

public:
  SensorState(uint8_t _sensor_pin)
  {
    sensor_pin = _sensor_pin;
    // light_sensor_data = analogRead(sensor_pin);
  }
  void setCurrentMode(AutoLightMode _mode) { cur_mode = _mode; }
  void setEngineRunFlag(bool _flag) { engine_run_flag = _flag; }
  AutoLightMode getCurrentMode() { return cur_mode; }
  bool getEngineRunFlag() { return engine_run_flag; }
  uint16_t getLightSensorData()
  {
    light_sensor_data = (light_sensor_data * 3 + analogRead(LIGHT_SENSOR_PIN)) / 4;
    return light_sensor_data;
  }
};

SensorState sys_data(LIGHT_SENSOR_PIN);

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
// переход в спящий режим
void startSleepMode(void *pvParameters);

// // отключение ближнего света через 30 секунд после превышения порога датчика света
// void turnOffLowBeam(void *pvParameters);

// ===================================================

/// @brief установка заданного состояния для заданного реле
/// @param rel заданное реле; 1 - ближний свет, 2 - ДХО; 0 - оба реле
/// @param state заданное состояние
void setRelayState(uint8_t rel, uint8_t state);

/// @brief начальный старт головного света при включении любого режима при запущенном двигателе
void runLightMode();
