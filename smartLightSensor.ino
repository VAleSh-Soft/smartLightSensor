#include <WiFi.h>
#include "header_file.h"
#include "_eeprom.h"
#include "_function.h"
#include "_tasks.h"
#include "_http.h"
#include "Arduino.h"

// ===================================================

void setup()
{
#if LOG_ON
  Serial.begin(115200);
  SLS_PRINTLN();
#endif

  // =================================================

  FastLED.addLeds<WS2811, LEDS_DATA_PIN, RGB>(leds, LEDS_NUM).setCorrection(Typical8mmPixel);
  leds[0] = CRGB::Black;
  FastLED.setBrightness(0);
  FastLED.show();

  // =================================================

  pinMode(IGNITION_PIN, INPUT);
  pinMode(ENGINE_RUN_PIN, INPUT);
  pinMode(RELAY_FOR_LB_PIN, OUTPUT);
  pinMode(RELAY_FOR_PL_PIN, OUTPUT);
#if USE_RELAY_FOR_DRL
  pinMode(RELAY_FOR_DRL_PIN, OUTPUT);
#endif

  // =================================================

  http_init();
  eeprom_init(!digitalRead(BTN_MODE_PIN)); // при зажатой при старте кнопке настройки сбрасываются к настройкам по умолчанию
  while (!digitalRead(BTN_MODE_PIN))       // ждем отпускания кнопки, если она была нажата при включении
  {
    leds[0] = CRGB::White; // если кнопка нажата, включаем белый цвет - пора отпускать
    FastLED.setBrightness(MIN_LED_BRIGHTNESS);
    FastLED.show();
    delay(100);
  }

  setCurrentMode(AutoLightSensorMode(read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE)));

  // =================================================

  xTaskCreate(btnCheck, "check_button", 4096, NULL, 1, NULL);
  xTaskCreate(setLeds, "set_led", 2048, NULL, 1, &xTask_leds);
  xTaskCreate(lightSensorCheck, "light_sensor_check", 2048, NULL, 1, NULL);
  xTaskCreate(engineRunCheck, "engine_run_check", 2048, NULL, 1, NULL);
  xTaskCreate(checkingForSleepMode, "start_sleep_mode", 2048, NULL, 1, NULL);
  xTaskCreate(wifiModuleManagement, "wifi_module_management", 4096, NULL, 1, NULL);

  // =================================================

#if LOG_ON
  printCurrentSettings();
  SLS_PRINTLN(F("Device Started"));
  SLS_PRINTLN();
#endif
}

void loop() {}
