#pragma once

#include <WebServer.h>
#include <shButton.h>
#include <FastLED.h>
#include "_updateServer.h"

// ==== Настройки ====================================

#define LOG_ON 0             // использовать вывод отладочной информации через UART
#define USE_RELAY_FOR_DRL 0  // использовать реле для ходовых огней; 0 - не использовать, 1 - использовать реле (ДХО будут включаться при старте двигателя с заданной в настройках задержкой);
#define USE_DRL_MANAGEMENT 0 // управлять ходовыми огнями; это нужно, если ходовые огни не отключаются автоматически при включении ближнего света; 0 - не управлять, 1 - управлять (ДХО будут включаться и выключаться в зависимости от состояния реле габаритных огней)

constexpr uint8_t LIGHT_SENSOR_PIN = 1;  // пин датчика освещенности
constexpr uint8_t IGNITION_PIN = 5;      // пин, на который приходит сигнал с линии зажигания; при появлении на этом пине высокого уровня МК выходит из глубокого сна, поэтому для esp32c3 допустимые значения 0..5
constexpr uint8_t ENGINE_RUN_PIN = 6;    // пин, на который приходит сигнал с вывода D генератора или HIGH при запущенном двигателе
constexpr uint8_t BTN_MODE_PIN = 4;      // пин кнопки режима работы
constexpr uint8_t LEDS_DATA_PIN = 3;     // пин выхода для светодиодов
constexpr uint8_t RELAY_FOR_LB_PIN = 10; // пин реле ближнего света
constexpr uint8_t RELAY_FOR_PL_PIN = 8;  // пин реле габаритных огней
#if USE_RELAY_FOR_DRL
constexpr uint8_t RELAY_FOR_DRL_PIN = 9; // пин реле ходовых огней
#endif

constexpr uint8_t CONTROL_LEVEL_FOR_LB = HIGH; // уровень управления реле ближнего света; HIGH или LOW
constexpr uint8_t CONTROL_LEVEL_FOR_PL = HIGH; // уровень управления реле габаритных огней; HIGH или LOW
#if USE_RELAY_FOR_DRL
constexpr uint8_t CONTROL_LEVEL_FOR_DRL = HIGH; // уровень управления реле ходовых огней; HIGH или LOW
#endif

// ==== Значения по умолчанию ========================

constexpr uint16_t DEFAULT_LIGHT_SENSOR_THRESHOLD = 3000; // значение датчика света по умолчанию, при котором включается ближний свет (для 12 бит ADC это примерно 75%)

constexpr uint8_t MAX_TURN_ON_DELAY = 10;    // максимальная задержка включения ближнего света после запуска двигателя, в секундах
constexpr uint8_t DEFAULT_TURN_ON_DELAY = 3; // задержка по умолчанию включения ближнего света после запуска двигателя, в секундах

constexpr uint8_t MAX_LB_SHUTDOWN_DELAY = 60;     // максимальная задержка выключения ближнего света после перехода порога датчика освещенности, в секундах
constexpr uint8_t MIN_LB_SHUTDOWN_DELAY = 5;      // минимальная задержка выключения ближнего света после перехода порога датчика освещенности, в секундах
constexpr uint8_t DEFAULT_LB_SHUTDOWN_DELAY = 30; // задержка по умолчанию выключения ближнего света после перехода порога датчика освещенности, в секундах
constexpr uint16_t MAX_RUN_SLEEP_DELAY = 60;      // максимальная задержка перехода в спящий режим после выключения зажигания, в секундах
constexpr uint16_t DEFAULT_RUN_SLEEP_DELAY = 10;  // задержка по умолчанию перехода в спящий режим после выключения зажигания, в секундах

constexpr uint16_t LIGHT_SENSOR_THRESHOLD_HISTERESIS = 200; // гистрезис порога датчика света (для 12 бит ADC это примерно 5%)

constexpr uint8_t MAX_LED_BRIGHTNESS = 250; // максимальная яркость светодиода (максимум 255)
constexpr uint8_t MIN_LED_BRIGHTNESS = 50;  // минимальная яркость светодиода (0 - полное отключение)

constexpr char *DEFAULT_AP_SSID = "shSmartLight"; // имя точки доступа по умолчанию
constexpr char *DEFAULT_AP_PASSWORD = "12345678"; // пароль точки доступа по умолчанию
constexpr char *DEFAULT_AP_IP = "192.168.4.1";    // ip адрес точки доступа по умолчанию

// ==== EEPROM =======================================

#define EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD 1 // индекс для хранения порога включения ближнего света, uint16_t
#define EEPROM_INDEX_FOR_CURRENT_MODE 3           // индекс для хранения текущего режима работы, uint8_t
#define EEPROM_INDEX_FOR_TURN_ON_DELAY 4          // индекс для хранения задержки включения ближнего света после запуска двигателя, uint8_t
#define EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY 5      // индекс для хранения задержки выключения ближнего света после перехода порога датчика освещенности, uint8_t
#define EEPROM_INDEX_FOR_STARTING_SLEEP_DELAY 6   // индекс для хранения задержки перехода в спящий режим после выключения зажигания, uint16_t
#define EEPROM_INDEX_FOR_AP_SSID 8                // индекс для хранения имени точки доступа, 33 байта; первый байт - размер строки
#define EEPROM_INDEX_FOR_AP_PASSWORD 41           // индекс для хранения пароля точки доступа, 65 байт; первый байт - размер строки
#define EEPROM_INDEX_FOR_AP_IP 106                // индекс для хранения ip адреса точки доступа, uint32_t
#define EEPROM_INDEX_FOR_LED_BRIGHTNESS_LEVEL 110 // индекс для хранения уровня яркости индикаторного светодиода, uint8_t

// ===================================================

enum AutoLightSensorMode : uint8_t
{
  SLS_MODE_MANUAL, // ручной режим
  SLS_MODE_AUTO    // автоматический режим
};

enum RelayState : uint8_t
{
  SLS_RELAY_ALL, // все реле
  SLS_RELAY_LB,  // реле ближнего света
  SLS_RELAY_PL   // реле габаритных огней
#if USE_RELAY_FOR_DRL
  ,
  SLS_RELAY_DRL // реле ходовых огней
#endif
};

enum WiFiModuleState : uint8_t
{
  SLS_WIFI_OFF,     // WiFi отключен
  SLS_WIFI_CONNECT, // включение WiFi
  SLS_WIFI_AP       // WiFi в режиме точки доступа
};

// ===================================================

const uint8_t LEDS_NUM = 1;
uint8_t ledBrightness = MIN_LED_BRIGHTNESS;

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

#if LOG_ON
#define SLS_PRINTLN(x)                                          \
  if (xSemaphoreTake(xSemaphore_uart, portMAX_DELAY) == pdTRUE) \
  {                                                             \
    Serial.println(x);                                          \
    xSemaphoreGive(xSemaphore_uart);                            \
  }
#define SLS_PRINT(x)                                            \
  if (xSemaphoreTake(xSemaphore_uart, portMAX_DELAY) == pdTRUE) \
  {                                                             \
    Serial.print(x);                                            \
    xSemaphoreGive(xSemaphore_uart);                            \
  }
#else
#define SLS_PRINTLN(x)
#define SLS_PRINT(x)
#endif

// ===================================================

xTaskHandle xTask_leds;

xSemaphoreHandle xSemaphore_relays = xSemaphoreCreateMutex();
xSemaphoreHandle xSemaphore_eng_run = xSemaphoreCreateMutex();
xSemaphoreHandle xSemaphore_wifi = xSemaphoreCreateMutex();
xSemaphoreHandle xSemaphore_eeprom = xSemaphoreCreateMutex();
xSemaphoreHandle xSemaphore_ign_flag = xSemaphoreCreateMutex();
xSemaphoreHandle xSemaphore_fastled = xSemaphoreCreateMutex();
xSemaphoreHandle xSemaphore_uart = xSemaphoreCreateMutex();

// ===================================================

constexpr uint8_t MAX_AP_SSID_LENGHT = 32;
constexpr uint8_t MAX_AP_PASSWORD_LENGHT = 64;

// ===================================================

// Web интерфейс для устройства
WebServer HTTP(80);

// сервер обновления по воздуху через web-интерфейс
#if LOG_ON
bool _log_on = true;
#else
bool _log_on = false;
#endif
shHTTPUpdateServer httpUpdater(_log_on);
String updateServerPage = "/firmware";

// ==== _function.h ==================================

void setCurrentMode(AutoLightSensorMode _mode);
AutoLightSensorMode getCurrentMode();
void setEngineRunFlag(bool _flag);
bool getEngineRunFlag();
void setRelayState(RelayState _rel, bool _state);
uint8_t getRelayState(RelayState _rel);
void setWiFiState(WiFiModuleState _state);
WiFiModuleState getWiFiState();
void startSleep();
void wifiStop();
bool getIgnitionState();
inline char *getApSsid();
inline char *getApPassword();
void fastLedShow(CRGB _col);
void setLedBrightness(uint8_t _br);
#if LOG_ON
void printCurrentSettings();
void printWiFiSetting();
#endif

// ==== _tasks.h =====================================

// опрос кнопки
void btnCheck(void *pvParameters);
// управление светодиодами
void setLeds(void *pvParameters);
// отслеживание показаний датчика света
void lightSensorCheck(void *pvParameters);
// отслеживание момента запуска двигателя
void engineRunCheck(void *pvParameters);
// переход в спящий режим
void checkingForSleepMode(void *pvParameters);
// управление WiFi
void wifiModuleManagement(void *pvParameters);

// ==== _eeprom.h ====================================

void eeprom_init(bool _reset = false);
uint8_t read_eeprom_8(uint16_t _index);
void write_eeprom_8(uint16_t _index, uint8_t _data);
uint16_t read_eeprom_16(uint16_t _index);
void write_eeprom_16(uint16_t _index, uint16_t _data);
uint32_t read_eeprom_32(uint16_t _index);
void write_eeprom_32(uint16_t _index, uint32_t _data);
void write_string_to_eeprom(uint16_t _index, const char *_string);
char *read_string_from_eeprom(uint16_t _index, const uint8_t _max_len);

// ==== _http.h ======================================

void http_init();
void handleGetConfigPage();
void handleGetConfig();
void handleSetConfig();
void handleClose();
void handleSetLedBrightness();
