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
    case BTN_ONECLICK:
      setCurrentMode(MODE_AUTO);
      break;
    case BTN_LONGCLICK:
      setCurrentMode(MODE_MANUAL);
      break;
    case BTN_DBLCLICK:
      // здесь включение WiFi
      break;
    }
    vTaskDelay(1);
  }

  vTaskDelete(NULL);
}

void setLeds(void *pvParameters)
{
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

    FastLED.show();

    vTaskDelay(50);
  }
  vTaskDelete(NULL);
}

void lightSensorCheck(void *pvParameters)
{
  while (1)
  {
    uint16_t sensor_data = getLightSensorData();
    uint16_t t = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD);

    // тут же управление светом в автоматическом режиме
    if (getCurrentMode() == MODE_AUTO && getEngineRunFlag())
    {
      if (sensor_data <= t)
      { // если уровень снизился до порога включения БС, то включить БС и остановить таймер отключения БС
        setRelayState(RELAY_LB, HIGH);
        // tasks.stopTask(turn_off_low_beam_timer);
      }
      else if (sensor_data > t + 50)
      { // если уровень превысил порог включения БС, реле БС включено, а таймер отключения БС еще не запущен, запустить его
        if (getRelayState(RELAY_LB) /*&& !tasks.getTaskState(turn_off_low_beam_timer)*/)
        {
          // tasks.startTask(turn_off_low_beam_timer);
        }
      }
    }

    // и здесь же управление яркостью светодиода - вне зависимость от режима работы
    if (sensor_data <= t)
    {
      FastLED.setBrightness(50);
    }
    else if (sensor_data > t + 50)
    {
      FastLED.setBrightness(255);
    }

    vTaskDelay(50);
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
      { // поднимать флаг запуска двигателя и, соответственно, включать свет только по истечении времени задержки;
        vTaskDelay(read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY));
        setEngineRunFlag(true);
        // runLightMode();
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

  eeprom_init();
  setCurrentMode(AutoLightMode(read_eeprom_8(EEPROM_INDEX_FOR_CURRENT_MODE)));
  light_sensor_data = analogRead(LIGHT_SENSOR_PIN);

  // =================================================

  xTaskCreate(btnCheck, "check_button", 4096, NULL, 1, NULL);
  xTaskCreate(setLeds, "set_led", 2048, NULL, 1, NULL);
  xTaskCreate(lightSensorCheck, "light_sensor_check", 2048, NULL, 1, NULL);
  xTaskCreate(engineRunCheck, "engine_run_check", 2048, NULL, 1, NULL);
  xTaskCreate(startSleepMode, "start_sleep_mode", 2048, NULL, 1, NULL);
}

void loop()
{
}
