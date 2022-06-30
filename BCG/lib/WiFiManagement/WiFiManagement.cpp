// INCLUDES ----------
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>
#include <WiFi.h>

#include "Defines.h"
#include "WiFiManagement.h"
// -------------------


// WiFi Event Manager ----------
wifiEvent   STAConnectEvent = {.handler = stationConnect, .type = SYSTEM_EVENT_STA_CONNECTED}, \
            STADisconnectEvent = {.handler = stationDisconnect, .type = SYSTEM_EVENT_STA_DISCONNECTED}, \
            STAIPEvent = {.handler = stationGotIP, .type = SYSTEM_EVENT_STA_GOT_IP}, \
            APStartEvent = {.handler = apStart, .type = SYSTEM_EVENT_AP_START}, \
            APStopEvent = {.handler = apStop, .type = SYSTEM_EVENT_AP_STOP}, \
            APClientConnectEvent = {.handler = apClientConnect, .type = SYSTEM_EVENT_AP_STACONNECTED};
// -----------------------------


// Route Control ---------------
route   base = {"/", HTTP_GET, introduceServer}, \
        auth = {"/auth", HTTP_POST, getWiFiConfig};
// -----------------------------


// WiFiEvent Callbacks ---------
void stationConnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG("CONNECTED", "Device SSID: %s", info.connected.ssid);
    stopServer();
    if (WiFi.getMode() != WIFI_STA)
        WiFi.mode(WIFI_STA);
    if (ticker.active())
        ticker.detach();
    if (STADisconnectEvent.id > 0)
        WiFi.removeEvent(STADisconnectEvent.id);
    STADisconnectEvent.id = WiFi.onEvent(stationDisconnect, SYSTEM_EVENT_STA_DISCONNECTED);
}


void stationDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG("DISCONNECTED", "Device SSID: %s", info.disconnected.ssid);
    ticker.once(AP_REACTIVATION_TIMEOUT, checkConnect);
    WiFi.removeEvent(STADisconnectEvent.id);
}


void stationGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG("GOT IP", "IP: %s", IPAddress(info.got_ip.ip_info.ip.addr).toString().c_str());
}


void apStart(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG_OPERATION("AP started w/ IP: %s", WiFi.softAPIP().toString().c_str());
    setupServer();
    APStopEvent.id =  WiFi.onEvent(APStopEvent.handler, APStopEvent.type);
    APClientConnectEvent.id = WiFi.onEvent(APClientConnectEvent.handler, APClientConnectEvent.type);
}


void apStop(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG_OPERATION("AP stopped");
    WiFi.removeEvent(APStopEvent.id);
    WiFi.removeEvent(APClientConnectEvent.id);
}


void apClientConnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG("AP CLIENT CONNECTED", "Device MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
    info.sta_connected.mac[0], info.sta_connected.mac[1], info.sta_connected.mac[2],
    info.sta_connected.mac[3], info.sta_connected.mac[4], info.sta_connected.mac[5]);
}


void checkConnect() {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR("Unable to connect to SSID, retrying in background");
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(AP_NAME);
        setupServer();
    }
}
// -----------------------------


// WebServer Handlers ----------
void introduceServer(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", F("ESP32 Dev Device: Client Request Received, please send credentials to router"));
}


void getWiFiConfig(AsyncWebServerRequest *request) {
    if (request->args() > 0) {
        if (request->hasArg("ssid") && request->hasArg("pwd")) {
            ssid = request->arg("ssid");
            pwd = request->arg("pwd");
            request->send(200, "text/plain", "ESP32 Dev Device: Client Request Received, WiFi configuration complete");
            LOG("INFO", "SSID: %s |---| PWD: %s", ssid.c_str(), pwd.c_str());
            WiFi.begin(ssid.c_str(), pwd.c_str());
            if (ticker.active())
                ticker.detach();
            ticker.once(AP_REACTIVATION_TIMEOUT, checkConnect);
        } else {
            LOG_ERROR("Bad request!");
            request->send(400);
        }
    } else {
        request->send(400);
    }
}


void notFoundHandler(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", F("404: page not valid!"));
}


void setupServer() {
    server.on(base.name, base.method, base.handler);
    server.on(auth.name, auth.method, auth.handler);
    server.onNotFound(notFoundHandler);
    server.begin();
}


void stopServer() {
    server.onNotFound(NULL);
    server.reset();
    server.end();
}
// -----------------------------