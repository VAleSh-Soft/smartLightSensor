#include <EEPROM.h>
#include "header_file.h"

// ===================================================

void btnCheck(void *pvParameters)
{
  while (1)
  {
    switch (btnMode.getButtonState())
    {
    case BTN_ONECLICK:
      /* code */
      break;
    case BTN_LONGCLICK:
      /* code */
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
    if (cur_mode != MODE_AUTO)
    {
      leds[0] = CRGB::Red; // в неавтоматическом режиме светится красным
    }
    else
    {
      // в автоматическом режиме светится: зеленым - двигатель не запущен; синим - двигатель запущен, включен ближний свет; оранжевым - двигатель запущен, ближний свет выключен
      leds[0] = (!engine_run_flag) ? CRGB::Green
                                   : ((digitalRead(RELAY_FOR_LB)) ? CRGB::Blue : CRGB::Orange);
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
    light_sensor_data = (light_sensor_data * 3 + analogRead(LIGHT_SENSOR_PIN)) / 4;
    uint16_t t = 0;
    EEPROM.get(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD, t);

    // тут же управление светом в автоматическом режиме
    if (cur_mode == MODE_AUTO && engine_run_flag)
    {
      if (light_sensor_data <= t)
      { // если уровень снизился до порога включения БС, то включить БС и остановить таймер отключения БС
        // setRelayState(1, HIGH);
        // tasks.stopTask(turn_off_low_beam_timer);
      }
      else if (light_sensor_data > t + 50)
      { // если уровень превысил порог включения БС, реле БС включено, а таймер отключения БС еще не запущен, запустить его
        if (digitalRead(RELAY_FOR_LB) /*&& !tasks.getTaskState(turn_off_low_beam_timer)*/)
        {
          // tasks.startTask(turn_off_low_beam_timer);
        }
      }
    }

    // и здесь же управление яркостью светодиода - вне зависимость от режима работы
    if (light_sensor_data <= t)
    {
      FastLED.setBrightness(50);
    }
    else if (light_sensor_data > t + 50)
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
    if (!engine_run_flag)
    {
      if (digitalRead(ENGINE_RUN_PIN))
      { // поднимать флаг запуска двигателя и, соответственно, включать свет только по истечении времени задержки;
        vTaskDelay(EEPROM.read(EEPROM_INDEX_FOR_TURN_ON_DELAY));
        engine_run_flag = true;
        // runLightMode();
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

  light_sensor_data = analogRead(LIGHT_SENSOR_PIN);

  // =================================================

  FastLED.addLeds<WS2811, LEDS_DATA_PIN, RGB>(leds, LEDS_NUM);

  // =================================================

  xTaskCreatePinnedToCore(btnCheck, "check_button", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(setLeds, "set_led", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(lightSensorCheck, "light_sensor_check", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(engineRunCheck, "engine_run_check", 2048, NULL, 1, NULL, 1);
}

void loop()
{
}
