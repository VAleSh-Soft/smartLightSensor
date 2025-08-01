#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>
#include "header_file.h"
#include "extras/c_page.h"

/*
 * строка для отправки на Web-страницу
 * {"ap_ssid":"test","ap_pass":"12345678","ap_ip":"192.168.4.1","threshold":90,"turn_on_delay":3,"max_turn_on_delay":10,"run_sleep_delay":20,"max_run_sleep_delay":60,"lb_shutown_delay":30, "max_lb_shutown_delay":60,"min_lb_shutown_delay":5,"led_br":10}
 *
 * строка от Web-страницы для сохранения параметров
 * {"ap_ssid":"test","ap_pass":"12345678","ap_ip":"192.168.4.1","threshold":90,"turn_on_delay":3,"run_sleep_delay":20,"lb_shutown_delay":30,"led_br":10}
 */

// ===================================================

const String ap_ssid = "ap_ssid";
const String ap_pass = "ap_pass";
const String ap_ip = "ap_ip";
const String threshold = "threshold";
const String turn_on_delay = "turn_on_delay";
const String max_turn_on_delay = "max_turn_on_delay";
const String run_sleep_delay = "run_sleep_delay";
const String max_run_sleep_delay = "max_run_sleep_delay";
const String lb_shutown_delay = "lb_shutown_delay";
const String max_lb_shutown_delay = "max_lb_shutown_delay";
const String min_lb_shutown_delay = "min_lb_shutown_delay";
const String led_brightness = "led_br";

// ===================================================

void http_init()
{
  // запрос стартовой страницы
  HTTP.on("/", HTTP_GET, handleGetConfigPage);
  // ответ 404
  HTTP.onNotFound([]()
                  { HTTP.send(404, "text/plan", F("404. Ooops!!! File not found.")); });
  // запрос текущих настроек
  HTTP.on("/_getconfig", HTTP_GET, handleGetConfig);
  // сохранение настроек
  HTTP.on("/_setconfig", HTTP_POST, handleSetConfig);
  // изменение яркости светодиода
  HTTP.on("/_ledbrightness", HTTP_POST, handleSetLedBrightness);
  // закрытие вкладки браузера и отключение WiFi
  HTTP.on("/_close", HTTP_GET, handleClose);
  // обновление прошивки через Web-интерфейс
  httpUpdater.setup(&HTTP, updateServerPage);
}

void handleGetConfigPage()
{
  SLS_PRINTLN(F("Web interface start"));
  HTTP.send(200, "text/html", FPSTR(config_page));
}

void handleGetConfig()
{
  DynamicJsonDocument doc(1024);

  doc[ap_ip] = IPAddress(read_eeprom_32(EEPROM_INDEX_FOR_AP_IP)).toString();
  doc[ap_ssid] = getApSsid();
  doc[ap_pass] = getApPassword();
  doc[turn_on_delay] = read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY);
  doc[max_turn_on_delay] = MAX_TURN_ON_DELAY;
  doc[lb_shutown_delay] = read_eeprom_8(EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY);
  doc[max_lb_shutown_delay] = MAX_LB_SHUTDOWN_DELAY;
  doc[min_lb_shutown_delay] = MIN_LB_SHUTDOWN_DELAY;
  doc[run_sleep_delay] = read_eeprom_8(EEPROM_INDEX_FOR_STARTING_SLEEP_DELAY);
  doc[max_run_sleep_delay] = MAX_RUN_SLEEP_DELAY;
  doc[threshold] = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) / 40;
  doc[led_brightness] = read_eeprom_8(EEPROM_INDEX_FOR_LED_BRIGHTNESS);

  String _res = "";
  serializeJson(doc, _res);

  SLS_PRINTLN(F("Requested data for the Web interface"));

  HTTP.send(200, "text/json", _res);
}

void handleSetConfig()
{
  if (HTTP.hasArg("plain") == false)
  {
    SLS_PRINTLN(F("Failed to save configuration data, no data"));
    HTTP.send(200, "text/plain", F("Body not received"));
    return;
  }

  String json = HTTP.arg("plain");

  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    SLS_PRINTLN(F("Failed to save configuration data, invalid json data"));
    SLS_PRINTLN(error.f_str());
  }
  else
  {
    String ip = doc[ap_ip].as<String>();
    write_eeprom_32(EEPROM_INDEX_FOR_AP_IP, (uint32_t)IPAddress(ip.c_str()));
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_SSID, doc[ap_ssid].as<String>().c_str());
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, doc[ap_pass].as<String>().c_str());
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY, doc[turn_on_delay].as<uint8_t>());
    write_eeprom_8(EEPROM_INDEX_FOR_LB_SHUTDOWN_DELAY, doc[lb_shutown_delay].as<uint8_t>());
    write_eeprom_8(EEPROM_INDEX_FOR_STARTING_SLEEP_DELAY, doc[run_sleep_delay].as<uint8_t>());
    write_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD, doc[threshold].as<uint16_t>() * 40);
    write_eeprom_8(EEPROM_INDEX_FOR_LED_BRIGHTNESS, doc[led_brightness].as<uint8_t>());
    HTTP.send(200, "text/html", F("<META http-equiv='refresh' content='1;URL=/_close'><p align='center'>Save settings...</p>"));
    SLS_PRINTLN();
    SLS_PRINTLN(F("The settings are saved"));
#if LOG_ON
    printCurrentSettings();
#endif
  }
}

void handleSetLedBrightness()
{
  if (HTTP.hasArg("plain") == false)
  {
    SLS_PRINTLN(F("Failed to led brightness data, no data"));
    HTTP.send(200, "text/plain", F("Body not received"));
    return;
  }

  String json = HTTP.arg("plain");

  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    SLS_PRINTLN(F("Failed to change the brightness of the led, invalid json data"));
    SLS_PRINTLN(error.f_str());
  }
  else
  {
    uint8_t _br = doc[led_brightness].as<uint8_t>();
    if (_br > 10)
    {
      _br = 10;
    }
    else if (_br == 0)
    {
      _br = 1;
    }
    write_eeprom_8(EEPROM_INDEX_FOR_LED_BRIGHTNESS, _br);
    HTTP.send(200, "text/plain", F("OK"));
  }
}

void handleClose()
{
  static const char close_page[] PROGMEM =
      R"(<!DOCTYPE html> <html> <body> <script> function closePage() { var request = new XMLHttpRequest(); request.open('GET', '/_getconfig', true); request.onload = function () { window.close(); }; request.send(); } document.addEventListener("DOMContentLoaded", closePage); </script> </body> </html>)";

  HTTP.send(200, "text/html", FPSTR(close_page));
  vTaskDelay(500);

  setWiFiState(SLS_WIFI_OFF);
}
