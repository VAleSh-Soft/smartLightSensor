#include <WiFi.h>
#include "header_file.h"
#include "_eeprom.h"
#include "_function.h"
#include "_tasks.h"
#include "_http.h"

// ===================================================

void setup()
{
  Serial.begin(115200);

  // =================================================

  FastLED.addLeds<WS2811, LEDS_DATA_PIN, RGB>(leds, LEDS_NUM);

  // =================================================

  pinMode(RELAY_FOR_LB_PIN, OUTPUT);
  pinMode(RELAY_FOR_PL_PIN, OUTPUT);
  pinMode(IGNITION_PIN, INPUT_PULLDOWN);
  pinMode(ENGINE_RUN_PIN, INPUT_PULLDOWN);
#if USE_RELAY_FOR_DRL
  pinMode(RELAY_FOR_DRL_PIN, OUTPUT);
#endif

  // =================================================

  semaphoreInit();
  eeprom_init(!digitalRead(BTN_MODE_PIN)); // при зажатой при старте кнопке настройки сбрасываются к настройкам по умолчанию
  setCurrentMode(AutoLightSensorMode(read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE)));
  http_init();

  // =================================================

  xTaskCreate(btnCheck, "check_button", 4096, NULL, 1, NULL);
  xTaskCreate(setLeds, "set_led", 2048, NULL, 1, &xTask_leds);
  xTaskCreate(lightSensorCheck, "light_sensor_check", 2048, NULL, 1, NULL);
  xTaskCreate(engineRunCheck, "engine_run_check", 2048, NULL, 1, NULL);
  xTaskCreate(startSleepMode, "start_sleep_mode", 2048, NULL, 1, NULL);
  xTaskCreate(wifiModuleManagement, "wifi_module_management", 4096, NULL, 1, NULL);
}

void loop() {}
