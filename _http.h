#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>
#include "header_file.h"
#include "extras/c_page.h"

/*
 * строка для отправки на Web-страницу
 * {"ap_ssid":"test","ap_pass":"12345678","ap_ip":"192.168.4.1","threshold":90,"turn_on_delay":3,"max_turn_on_delay":10,"turn_off_delay":20,"max_turn_off_delay":60,"thresh_delay":30, "max_thresh_delay":60,"min_thresh_delay":5}
 *
 * строка с Web-страницы для сохранения параметров
 * {"ap_ssid":"test","ap_pass":"12345678","ap_ip":"192.168.4.1","threshold":90,"turn_on_delay":3,"turn_off_delay":20,"thresh_delay":30}
 */

void http_init()
{
  // запрос стартовой страницы
  HTTP.on("/", HTTP_GET, handleGetConfigPage);
  // запрос текущих настроек
  HTTP.on("/getconfig", HTTP_GET, handleGetConfig);
  // сохранение настроек
  HTTP.on("/setconfig", HTTP_POST, []()
          { handleSetConfig(); });
  // ответ 404
  HTTP.onNotFound([]()
                  { HTTP.send(404, "text/plan", F("404. Ooops!!! File not found.")); });
}

void handleGetConfigPage()
{
  HTTP.send(200, "text/html", FPSTR(config_page));
}

void handleGetConfig()
{
  DynamicJsonDocument doc(1024);

  doc["ap_ip"] = IPAddress(read_eeprom_32(EEPROM_INDEX_FOR_AP_IP)).toString();
  doc["ap_ssid"] = read_string_from_eeprom(EEPROM_INDEX_FOR_AP_SSID, 32);
  doc["ap_pass"] = read_string_from_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, 64);
  doc["turn_on_delay"] = read_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY);
  doc["max_turn_on_delay"] = MAX_TURN_ON_DELAY;
  doc["thresh_delay"] = read_eeprom_8(EEPROM_INDEX_FOR_TURN_OFF_DELAY);
  doc["max_thresh_delay"] = MAX_TURN_OFF_DELAY;
  doc["min_thresh_delay"] = MIN_TURN_OFF_DELAY;
  doc["turn_off_delay"] = read_eeprom_8(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY);
  doc["max_turn_off_delay"] = MAX_RUN_SLEEP_DELAY;
  doc["threshold"] = read_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD) / 40;

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
    const char *str = doc["ap_ip"].as<String>().c_str();
    Serial.println("IP: " + String(str));
    write_eeprom_32(EEPROM_INDEX_FOR_AP_IP, (uint32_t)IPAddress(str));
    const char *str1 = doc["ap_ssid"].as<String>().c_str();
    Serial.println("SSID: " + String(str1));
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_SSID, str1);
    const char *str2 = doc["ap_pass"].as<String>().c_str();
    Serial.println("PASS: " + String(str2));
    write_string_to_eeprom(EEPROM_INDEX_FOR_AP_PASSWORD, str2);
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_ON_DELAY, doc["turn_on_delay"].as<uint8_t>());
    write_eeprom_8(EEPROM_INDEX_FOR_TURN_OFF_DELAY, doc["thresh_delay"].as<uint8_t>());
    write_eeprom_8(EEPROM_INDEX_FOR_RUN_SLEEP_DELAY, doc["turn_off_delay"].as<uint8_t>());
    write_eeprom_16(EEPROM_INDEX_FOR_LIGHT_SENSOR_THRESHOLD, doc["threshold"].as<uint16_t>() * 40);
    HTTP.send(200, "text/html", F("<META http-equiv='refresh' content='1;URL=/'><p align='center'>Save settings...</p>"));
  }
  setWiFiState(SLS_WIFI_OFF);
}
