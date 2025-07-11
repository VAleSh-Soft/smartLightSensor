#pragma once

#include <WebServer.h>
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
  HTTP.on("/setconfig", HTTP_POST, handleSetConfig);
  // ответ 404
  HTTP.onNotFound([]()
                  { HTTP.send(404, "text/plan", F("404. File not found.")); });
}

void handleGetConfigPage()
{
  HTTP.send(200, "text/html", FPSTR(config_page));
}

void handleGetConfig()
{
  static const char str[] PROGMEM =
      R"({"ap_ssid":"test","ap_pass":"12345678","ap_ip":"192.168.4.1","threshold":90,"turn_on_delay":3,"max_turn_on_delay":10,"turn_off_delay":20,"max_turn_off_delay":60,"thresh_delay":30, "max_thresh_delay":60,"min_thresh_delay":5})";
  HTTP.send(200, "text/json", FPSTR(str));
}

void handleSetConfig()
{
  setWiFiState(SLS_WIFI_OFF);
}
