#pragma once

#include "header_file.h"
#include "_eeprom.h"

// ===================================================

bool engine_run_flag = false;              // флаг запуска двигателя
WiFiModuleState wifi_state = SLS_WIFI_OFF; // состояние WiFi

// ===================================================

void setCurrentMode(AutoLightSensorMode _mode)
{
  write_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE, uint8_t(_mode));
  SLS_PRINT(F("Setting The Current Mode: "));
  SLS_PRINTLN(((uint8_t)_mode) ? F("Auto") : F("Manual"));
}

AutoLightSensorMode getCurrentMode()
{
  return (AutoLightSensorMode)read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE);
}

void setEngineRunFlag(bool _flag)
{
  if (xSemaphoreTake(xSemaphore_eng_run, portMAX_DELAY) == pdTRUE)
  {
    engine_run_flag = _flag;
    xSemaphoreGive(xSemaphore_eng_run);
  }
}

bool getEngineRunFlag()
{
  bool _flag = false;
  if (xSemaphoreTake(xSemaphore_eng_run, portMAX_DELAY) == pdTRUE)
  {
    _flag = engine_run_flag;
    xSemaphoreGive(xSemaphore_eng_run);
  }
  return _flag;
}

void setRelayState(RelayState _rel, bool _state)
{
  if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
  {
    if (_rel == SLS_RELAY_PL || _rel == SLS_RELAY_ALL)
    {
      digitalWrite(RELAY_FOR_PL_PIN,
                   (CONTROL_LEVEL_FOR_PL ? (uint8_t)_state : (uint8_t)!_state));
    }
    if (_rel == SLS_RELAY_LB || _rel == SLS_RELAY_ALL)
    {
      digitalWrite(RELAY_FOR_LB_PIN,
                   (CONTROL_LEVEL_FOR_LB ? (uint8_t)_state : (uint8_t)!_state));
    }
#if USE_RELAY_FOR_DRL
#if USE_DRL_MANAGEMENT
    if (_rel == SLS_RELAY_DRL)
    {
      digitalWrite(RELAY_FOR_DRL_PIN,
                   (CONTROL_LEVEL_FOR_DRL ? (uint8_t)_state : (uint8_t)!_state));
    }
    else
    {
      bool x = (digitalRead(RELAY_FOR_PL_PIN) == CONTROL_LEVEL_FOR_PL);
      digitalWrite(RELAY_FOR_DRL_PIN,
                   (CONTROL_LEVEL_FOR_DRL ? (uint8_t)!x : (uint8_t)x));
    }
#else
    if (getEngineRunFlag())
    {
      digitalWrite(RELAY_FOR_DRL_PIN, CONTROL_LEVEL_FOR_DRL);
    }
#endif
#endif
    xSemaphoreGive(xSemaphore_relays);
  }
}

uint8_t getRelayState(RelayState _rel)
{
  uint8_t _state = LOW;
  if (xSemaphoreTake(xSemaphore_relays, portMAX_DELAY) == pdTRUE)
  {
    switch (_rel)
    {
    case SLS_RELAY_LB:
      _state = (digitalRead(RELAY_FOR_LB_PIN) == CONTROL_LEVEL_FOR_LB);
      break;
    case SLS_RELAY_PL:
      _state = (digitalRead(RELAY_FOR_PL_PIN) == CONTROL_LEVEL_FOR_PL);
      break;
#if USE_RELAY_FOR_DRL
    case SLS_RELAY_DRL:
      _state = (digitalRead(RELAY_FOR_DRL_PIN) == CONTROL_LEVEL_FOR_DRL);
      break;
#endif
    default:
      break;
    }
    xSemaphoreGive(xSemaphore_relays);
  }
  return _state;
}

void setWiFiState(WiFiModuleState _state)
{
  if (xSemaphoreTake(xSemaphore_wifi, portMAX_DELAY) == pdTRUE)
  {
    wifi_state = _state;
    xSemaphoreGive(xSemaphore_wifi);
  }
}

WiFiModuleState getWiFiState()
{
  WiFiModuleState _state;
  if (xSemaphoreTake(xSemaphore_wifi, portMAX_DELAY) == pdTRUE)
  {
    _state = wifi_state;
    xSemaphoreGive(xSemaphore_wifi);
  }
  return _state;
}

void startSleep()
{
  // здесь делаем подготовку ко сну
  vTaskSuspend(xTask_leds);
  fastLedShow(CRGB::Black);
  pinMode(RELAY_FOR_LB_PIN, INPUT);
  pinMode(RELAY_FOR_PL_PIN, INPUT);
#if USE_RELAY_FOR_DRL
  pinMode(RELAY_FOR_DRL_PIN, INPUT);
#endif

  if (getWiFiState() != SLS_WIFI_OFF)
  {
    wifiStop();
    setWiFiState(SLS_WIFI_OFF);
  }

  SLS_PRINTLN(F("Switching To Sleep Mode"));
  SLS_PRINTLN();
  esp_deep_sleep_enable_gpio_wakeup(1 << IGNITION_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);
  esp_deep_sleep_start();
}

void wifiStop()
{
  HTTP.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  SLS_PRINTLN(F("Access Point Stop"));
}

bool getIgnitionState()
{
  bool _state = false;
  if (xSemaphoreTake(xSemaphore_ign_flag, portMAX_DELAY) == pdTRUE)
  {
    _state = digitalRead(IGNITION_PIN);
    xSemaphoreGive(xSemaphore_ign_flag);
  }
  return _state;
}

inline char *getApSsid()
{
  return read_string_from_eeprom(EEPROM_INDEX_FOR_AP_SSID, MAX_AP_SSID_LENGHT);
}

inline char *getApPassword()
{
  return read_string_from_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, MAX_AP_PASSWORD_LENGHT);
}

void fastLedShow()
{
  if (xSemaphoreTake(xSemaphore_fastled, portMAX_DELAY) == pdTRUE)
  {
    FastLED.show();
    xSemaphoreGive(xSemaphore_fastled);
  }
}

void fastLedShow(CRGB _col)
{
  if (xSemaphoreTake(xSemaphore_fastled, portMAX_DELAY) == pdTRUE)
  {
    setLedColor(_col);
    FastLED.show();
    xSemaphoreGive(xSemaphore_fastled);
  }
}

void setLedColor(CRGB _col)
{
  if (xSemaphoreTake(xSemaphore_led_color, portMAX_DELAY) == pdTRUE)
  {
    leds[0] = _col;
    xSemaphoreGive(xSemaphore_led_color);
  }
}

bool compareCrgbData(CRGB _col)
{
  bool _res = false;
  if (xSemaphoreTake(xSemaphore_led_color, portMAX_DELAY) == pdTRUE)
  {
    _res = ((leds[0].r == _col.r) && (leds[0].g == _col.g) && (leds[0].b == _col.b));
    xSemaphoreGive(xSemaphore_led_color);
  }
  return _res;
}

#if LOG_ON
void printCurrentSettings()
{
  SLS_PRINTLN(F("================================="));
  SLS_PRINTLN(F("Current Settings:"));
  SLS_PRINTLN();
  SLS_PRINT(F("  Current Mode: "));
  SLS_PRINTLN(((uint8_t)getCurrentMode()) ? F("Auto") : F("Manual"));
  SLS_PRINT(F("  Liht Sensor Threshold: "));
  SLS_PRINT(read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) / 40);
  SLS_PRINTLN(F(" %"));
  SLS_PRINT(F("  Low Beam Shutdown Delay (seconds): "));
  SLS_PRINTLN(read_eeprom_8(EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY));
  SLS_PRINT(F("  Delay For Turn ON (seconds): "));
  SLS_PRINTLN(read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY));
  SLS_PRINT(F("  Delay For Starting Sleep Mode (seconds): "));
  SLS_PRINTLN(read_eeprom_8(EEPROM_INDEX_FOR_STARTING_SLEEP_DELAY));
  SLS_PRINTLN();
  SLS_PRINTLN(F("Settings For AP:"));
  SLS_PRINTLN();
  printWiFiSetting();
  SLS_PRINTLN(F("================================="));
  SLS_PRINTLN();
}
void printWiFiSetting()
{
  SLS_PRINT(F("  AP SSID: "));
  SLS_PRINTLN(getApSsid());
  SLS_PRINT(F("  AP Password: "));
  SLS_PRINTLN(getApPassword());
  SLS_PRINT(F("  AP IP: "));
  SLS_PRINTLN(IPAddress(read_eeprom_32(EEPROM_INDEX_FOR_AP_IP)).toString());
}
#endif

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
