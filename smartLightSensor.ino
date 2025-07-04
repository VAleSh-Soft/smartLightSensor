#include "header_file.h"
#include "_eeprom.h"
#include "_function.h"

// ==== Tasks ========================================

void btnCheck(void *pvParameters)
{
  while (1)
  {
    switch (btnMode.getButtonState())
    {
    case BTN_ONECLICK: // короткий клик включает автоматический режим работы
      if (getCurrentMode() != MODE_AUTO)
      {
        setCurrentMode(MODE_AUTO);
      }
      break;
    case BTN_LONGCLICK: // длинный клик выключает автоматический режим работы
      if (getCurrentMode() != MODE_MANUAL)
      {
        setCurrentMode(MODE_MANUAL);
      }
      break;
    case BTN_DBLCLICK: // двойной клик включает/выключает WiFi модуль
      if (getWiFiState() == WIFI_OFF)
      {
        setWiFiState(WIFI_CONNECT);
      }
      else
      {
        setWiFiState(WIFI_OFF);
      }
      break;
    }
    vTaskDelay(1);
  }

  vTaskDelete(NULL);
}

void setLeds(void *pvParameters)
{

  uint8_t num = 0;

  while (1)
  {
    if (getCurrentMode() != MODE_AUTO)
    {
      leds[0] = CRGB::Red; // в неавтоматическом режиме светится красным
    }
    else
    {
      // в автоматическом режиме светится: зеленым - двигатель не запущен; синим - двигатель запущен, включен ближний свет; оранжевым - двигатель запущен, ближний свет выключен
      leds[0] = (!getEngineRunFlag()) ? CRGB::Green
                                      : ((getRelayState(RELAY_LB)) ? CRGB::Blue
                                                                   : CRGB::Orange);
    }

    // если включен WiFi, светодиод мигает с частотой 1 Гц
    if (wifi_state == WIFI_AP)
    {
      if (num >= 10)
      {
        leds[0] = CRGB::Black;
      }

      if (++num >= 20)
      {
        num = 0;
      }
    }
    else
    {
      num = 0;
    }

    FastLED.show();

    vTaskDelay(50);
  }
  vTaskDelete(NULL);
}

void lightSensorCheck(void *pvParameters)
{
  uint16_t sensor_data = analogRead(LIGHT_SENSOR_PIN);
  uint16_t t;
  bool timer = false;
  uint16_t timer_counter = 0;

  const uint32_t LS_DELAY = 20ul;

  while (1)
  {
    sensor_data = (sensor_data * 3 + analogRead(LIGHT_SENSOR_PIN)) / 4;
    t = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD);

    // тут же управление светом в автоматическом режиме
    if (getCurrentMode() == MODE_AUTO && getEngineRunFlag())
    {
      if (sensor_data <= t)
      { // если уровень снизился до порога включения БС, то включить БС и сбросить флаг отключения БС
        setRelayState(RELAY_ALL, HIGH);
        timer = false;
      }
      else if (sensor_data > (t + LIGHT_SENSOR_THRESHOLD_HISTERESIS))
      { // если уровень превысил порог включения БС, реле БС включено, а флаг отключения БС еще не поднят, поднять его
        if (getRelayState(RELAY_LB) && !timer)
        {
          timer = true;
          timer_counter = 0;
        }
      }

      // тут же управление таймером отключения БС, если поднят флаг таймера
      if (timer)
      {
        if (timer_counter >= read_eeprom_8(EEPROM_INDEX_FOR_TURN_OFF_DELAY) * 1000ul / LS_DELAY)
        {
          setRelayState(RELAY_ALL, LOW);
          timer = false;
        }
        else
        {
          timer_counter++;
        }
      }
    }
    else if (getRelayState(RELAY_LB))
    {
      setRelayState(RELAY_ALL, LOW);
    }

    // и здесь же управление яркостью светодиода - вне зависимость от режима работы
    if (sensor_data <= t)
    {
      FastLED.setBrightness(MIN_LED_BRIGHTNESS);
    }
    else if (sensor_data > (t + LIGHT_SENSOR_THRESHOLD_HISTERESIS))
    {
      FastLED.setBrightness(MAX_LED_BRIGHTNESS);
    }

    vTaskDelay(LS_DELAY);
  }
  vTaskDelete(NULL);
}

void engineRunCheck(void *pvParameters)
{
  while (1)
  {
    if (!getEngineRunFlag())
    {
      if (digitalRead(ENGINE_RUN_PIN))
      { // поднимать флаг запуска двигателя и, соответственно, включать БС только по истечении времени задержки;
        vTaskDelay(read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY) * 1000ul);
        setEngineRunFlag(true);
      }
    }

    vTaskDelay(100);
  }
  vTaskDelete(NULL);
}

void startSleepMode(void *pvParameters)
{
  uint16_t t = 0;

  bool _flag = false;

  while (1)
  {
    if (_flag)
    {
      // если флаг поднят, а зажигание выключено - флаг сбросить
      if (!digitalRead(IGNITION_PIN))
      {
        _flag = false;
      }
    }
    else
    {
      // если флаг сброшен, а зажигание включено - поднять флаг и обнулить счетчик
      if (digitalRead(IGNITION_PIN))
      {
        _flag = true;
        t = 0;
      }
    }

    // если флаг сброшен, отсчитываем заданный интервал и уходим в сон
    if (!_flag)
    {
      if (t >= read_eeprom_16(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY) * 10)
      {
        // здесь делаем подготовку ко сну
        vTaskSuspend(xTask_leds);
        leds[0] = CRGB::Black;
        FastLED.show();

        uint64_t wakeup_pin_mask = 1;
        for (uint8_t i = 0; i < IGNITION_PIN; i++)
        {
          wakeup_pin_mask *= 2;
        }
        esp_deep_sleep_enable_gpio_wakeup(wakeup_pin_mask, ESP_GPIO_WAKEUP_GPIO_HIGH);
        esp_deep_sleep_start();
      }
      else
      {
        t++;
      }
    }

    vTaskDelay(100);
  }
  vTaskDelete(NULL);
}

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

  // =================================================

  semaphoreInit();
  eeprom_init();
  setCurrentMode(AutoLightMode(read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE)));
  wifi_ssid = read_string_from_eeprom(EEPROM_INDEX_FOR_AP_SSID, 32);
  wifi_pass = read_string_from_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, 64);

  // =================================================

  xTaskCreate(btnCheck, "check_button", 4096, NULL, 1, NULL);
  xTaskCreate(setLeds, "set_led", 2048, NULL, 1, &xTask_leds);
  xTaskCreate(lightSensorCheck, "light_sensor_check", 2048, NULL, 1, NULL);
  xTaskCreate(engineRunCheck, "engine_run_check", 2048, NULL, 1, NULL);
  xTaskCreate(startSleepMode, "start_sleep_mode", 2048, NULL, 1, NULL);
}

void loop()
{
}
