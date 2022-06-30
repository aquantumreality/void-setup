#ifndef WIFIMANAGEMENT_H
#define WIFIMANAGEMENT_H

#define TAG "wifiManagement"


// WiFi Event Handlers ---------
void stationConnect(WiFiEvent_t event, WiFiEventInfo_t info);
void stationDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);
void stationGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

void apStart(WiFiEvent_t event, WiFiEventInfo_t info);
void apStop(WiFiEvent_t event, WiFiEventInfo_t info);
void apClientConnect(WiFiEvent_t event, WiFiEventInfo_t info);

void checkConnect();
// -----------------------------


// WebServer Handlers ----------
void introduceServer(AsyncWebServerRequest *request);
void getWiFiConfig(AsyncWebServerRequest *request);
void notFoundHandler(AsyncWebServerRequest *request);

void setupServer();
void stopServer();
// -----------------------------


// WiFi Event Manager ----------
typedef struct wifiEvent {
    WiFiEventFuncCb handler;
    system_event_id_t type;
    WiFiEventId_t id; // none
} wifiEvent;
// -----------------------------


// Route Control ---------------
typedef struct route {
    char name[20];
    WebRequestMethod method;
    ArRequestHandlerFunction handler;
} route;
// -----------------------------

#endif