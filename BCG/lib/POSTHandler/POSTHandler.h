#ifndef POSTHANDLER_H
#define POSTHANDLER_H

// INCLUDES ----------
#include <Arduino.h>
#include <FS.h>
#include <WiFi.h>
// -------------------


// POST Handlers ---------------
void setHeaders(String& headers);
String sendFile(File target, WiFiClient client, String& headers);
// -----------------------------

#endif