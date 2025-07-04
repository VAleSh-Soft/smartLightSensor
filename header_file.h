#pragma once

#include <shButton.h>
#include <FastLED.h>

// ==== Настройки ====================================

constexpr uint8_t LIGHT_SENSOR_PIN = 4; // пин датчика освещенности
constexpr uint8_t IGNITION_PIN = 5;     // пин, на который приходит сигнал с линии зажигания; при появлении на этом пине высокого уровня МК выходит из глубокого сна, поэтому для esp32c3 допустимые значения 0..5
constexpr uint8_t ENGINE_RUN_PIN = 6;   // пин, на который приходит сигнал с вывода D генератора или HIGH при запущенном двигателе
constexpr uint8_t BTN_MODE_PIN = 3;     // пин кнопки режима работы
constexpr uint8_t LEDS_DATA_PIN = 7;    // пин выхода для светодиодов
constexpr uint8_t RELAY_FOR_LB_PIN = 8; // пин реле ближнего света
constexpr uint8_t RELAY_FOR_PL_PIN = 9; // пин реле габаритных огней

#define EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD 1 // индекс для хранения порога включения ближнего света, uint16_t
#define EEPROM_INDEX_FOR_CURRENT_MODE 3           // индекс для хранения текущего режима работы, uint8_t
#define EEPROM_INDEX_FOR_TURN_ON_DELAY 4          // индекс для хранения задержки включения ближнего света после запуска двигателя, uint8_t
#define EEPROM_INDEX_FOR_TURN_OFF_DELAY 5         // индекс для хранения задержки выключения ближнего света после перехода порога датчика освещенности, uint8_t
#define EEPROM_INDEX_FOR_RUN_SLEEP_DELAY 6        // индекс для хранения задержки перехода в спящий режим после выключения зажигания, uint16_t
#define EEPROM_INDEX_FOR_AP_SSID 8                // индекс для хранения имени точки доступа, 33 байта; первый байт - размер строки
#define EEPROM_INDEX_FOR_AP_PASSWORD 41           // индекс для хранения пароля точки доступа, 65 байт; первый байт - размер строки

// ==== Значения по умолчанию ========================

constexpr uint16_t DEFAULT_LIGHT_SENSOR_THRESHOLD = 3000; // значение датчика света по умолчанию, при котором включается ближний свет

constexpr uint8_t MAX_TURN_ON_DELAY = 10;    // максимальная задержка включения ближнего света после запуска двигателя, в секундах
constexpr uint8_t DEFAULT_TURN_ON_DELAY = 3; // задержка по умолчанию включения ближнего света после запуска двигателя, в секундах

constexpr uint8_t MAX_TURN_OFF_DELAY = 60; // максимальная задержка выключения ближнего света после перехода порога датчика освещенности, в секундах
constexpr uint8_t MIN_TURN_OFF_DELAY = 5;  // минимальная задержка выключения ближнего света после перехода порога датчика освещенности, в секундах
constexpr uint8_t DEFAULT_TURN_OFF_DELAY = 30;
// задержка по умолчанию выключения ближнего света после перехода порога датчика освещенности, в секундах
constexpr uint16_t MAX_RUN_SLEEP_DELAY = 60;     // максимальная задержка перехода в спящий режим после выключения зажигания, в секундах
constexpr uint16_t DEFAULT_RUN_SLEEP_DELAY = 10; // задержка по умолчанию перехода в спящий режим после выключения зажигания, в секундах

constexpr uint16_t LIGHT_SENSOR_THRESHOLD_HISTERESIS = 200; // гистрезис порога датчика света

constexpr uint8_t MAX_LED_BRIGHTNESS = 250; // максимальная яркость светодиода
constexpr uint8_t MIN_LED_BRIGHTNESS = 50;  // минимальная яркость светодиода

constexpr char *DEFAULT_AP_SSID = "shSmartLight"; // имя точки доступа по умолчанию
constexpr char *DEFAULT_AP_PASSWORD = "12345678"; // пароль точки доступа по умолчанию

// ===================================================

enum AutoLightMode : uint8_t
{
  MODE_MANUAL, // ручной режим
  MODE_AUTO    // автоматический режим
};

enum RelayState : uint8_t
{
  RELAY_ALL, // все реле
  RELAY_LB,  // реле ближнего света
  RELAY_PL   // реле габаритных огней
};

enum WiFiState : uint8_t
{
  WIFI_OFF,     // WiFi отключен
  WIFI_CONNECT, // включение WiFi
  WIFI_AP       // WiFi в режиме точки доступа
};

// ===================================================

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

// ===================================================

void setCurrentMode(AutoLightMode _mode);
AutoLightMode getCurrentMode();
void setEngineRunFlag(bool _flag);
bool getEngineRunFlag();
void setRelayState(RelayState _rel, uint8_t state);
uint8_t getRelayState(RelayState _rel);
void setWiFiState(WiFiState _state);
WiFiState getWiFiState();
void semaphoreInit();

// ===================================================

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
