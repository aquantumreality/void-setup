// INCLUDES ----------
#include <Arduino.h>
#include <WiFi.h>

#include "Defines.h"
#include "FS.h"
// -------------------


// POST Handlers ---------------
void setHeaders(String& headers) {
  headers = headers + "POST http://" + HOST + ENDPOINT + "?" + QUERY_PARAMS + " HTTP/1.1\r\n";
  headers += "User-Agent: BCGDEV2\r\n";
  headers += "Accept: */*\r\n";
  headers += "Host: " + HOST + "\r\n";
  headers = headers + "Referer: http://" + HOST + ENDPOINT + "?" + QUERY_PARAMS + "\r\n";
  headers += "Content-Type: multipart/form-data; boundary=" + BOUNDARY + "\r\n";
  headers += "Connection: keep-alive\r\n"; // Content-Length appended later
}


String sendFile(File target, WiFiClient client, String& headers) {
  if (!client.connect((const char*) HOST.c_str(), PORT)) // connection check
    return "connection failed!";
  
  // SEND HEADERS ==================================
  client.print(headers);
  client.print("Content-Length: " + String(target.size()) + "\r\n\r\n");
  // ===============================================

  // CHUNKIFY FILE TO SEND =========================
  uint8_t buffer[CHUNK_SIZE];
  uint8_t stop; // indicates the final
  while (target.available()) {
    for (stop=0; stop<CHUNK_SIZE && target.available(); stop++)
      buffer[stop] = target.read();
    client.write(buffer, stop);
  }
  // ===============================================

  vTaskDelay(20 / portTICK_PERIOD_MS); // wait for a moment
  
  // CAPTURE RESPONSE ==============================
  String response = "";
  long interval = millis() + RESPONSE_TIMEOUT;
  while (client.connected() && interval > millis())
    if (client.available())
      response = response + (char)client.read();
  // ===============================================

  return response;
}
// -----------------------------