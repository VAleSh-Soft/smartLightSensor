#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>
#include "header_file.h"
#include "extras/c_page.h"

/*
 * строка для отправки на Web-страницу
 * {"ap_ssid":"test","ap_pass":"12345678","ap_ip":"192.168.4.1","threshold":90,"turn_on_delay":3,"max_turn_on_delay":10,"run_sleep_delay":20,"max_run_sleep_delay":60,"thresh_delay":30, "max_thresh_delay":60,"min_thresh_delay":5}
 *
 * строка с Web-страницы для сохранения параметров
 * {"ap_ssid":"test","ap_pass":"12345678","ap_ip":"192.168.4.1","threshold":90,"turn_on_delay":3,"run_sleep_delay":20,"thresh_delay":30}
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
 const String thresh_delay = "thresh_delay";
 const String max_thresh_delay = "max_thresh_delay";
 const String min_thresh_delay = "min_thresh_delay";

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
  // закрытие вкладки браузера и отключение WiFi
  HTTP.on("/_close", HTTP_GET, handleClose);
}

void handleGetConfigPage()
{
  HTTP.send(200, "text/html", FPSTR(config_page));
}

void handleGetConfig()
{
  DynamicJsonDocument doc(1024);

  doc[ap_ip] = IPAddress(read_eeprom_32(EEPROM_INDEX_FOR_AP_IP)).toString();
  doc[ap_ssid] = read_string_from_eeprom(EEPROM_INDEX_FOR_AP_SSID, 32);
  doc[ap_pass] = read_string_from_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, 64);
  doc[turn_on_delay] = read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY);
  doc[max_turn_on_delay] = MAX_TURN_ON_DELAY;
  doc[thresh_delay] = read_eeprom_8(EEPROM_INDEX_FOR_THRESH_DELAY);
  doc[max_thresh_delay] = MAX_THRESH_DELAY;
  doc[min_thresh_delay] = MIN_THRESH_DELAY;
  doc[run_sleep_delay] = read_eeprom_8(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY);
  doc[max_run_sleep_delay] = MAX_RUN_SLEEP_DELAY;
  doc[threshold] = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) / 40;

  String _res = "";
  serializeJson(doc, _res);
  Serial.println(_res);

  HTTP.send(200, "text/json", _res);
}

void handleSetConfig()
{
  if (HTTP.hasArg("plain") == false)
  {
    Serial.println(F("Failed to save configuration data, no data"));
    HTTP.send(200, "text/plain", F("Body not received"));
    return;
  }

  String json = HTTP.arg("plain");
  Serial.println(json);

  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.println(F("Failed to save configuration data, invalid json data"));
    Serial.println(error.f_str());
  }
  else
  {
    const char *_ip = doc[ap_ip].as<String>().c_str();
    write_eeprom_32(EEPROM_INDEX_FOR_AP_IP, (uint32_t)IPAddress(_ip));
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_SSID, doc[ap_ssid].as<String>().c_str());
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, doc[ap_pass].as<String>().c_str());
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY, doc[turn_on_delay].as<uint8_t>());
    write_eeprom_8(EEPROM_INDEX_FOR_THRESH_DELAY, doc[thresh_delay].as<uint8_t>());
    write_eeprom_8(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY, doc[run_sleep_delay].as<uint8_t>());
    write_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD, doc[threshold].as<uint16_t>() * 40);
    HTTP.send(200, "text/html", F("<META http-equiv='refresh' content='1;URL=/_close'><p align='center'>Save settings...</p>"));
  }
}

void handleClose()
{
  static const char close_page[] PROGMEM =
      R"(<!DOCTYPE html> <html> <body> <script> function closePage() { var request = new XMLHttpRequest(); request.open('GET', '/_getconfig', true); request.onload = function () { window.close(); }; request.send(); } document.addEventListener("DOMContentLoaded", closePage); </script> </body> </html>)";

  HTTP.send(200, "text/html", FPSTR(close_page));

  setWiFiState(SLS_WIFI_OFF);
}
