
#ifndef DEFINES_H
#define DEFINES_H

#define BUILD_NUMBER "v1.0.2"
#define V2 

// INCLUDES =============================================
#include <Arduino.h>
#include <Ticker.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
// ======================================================


// LOGGER ===============================================
#define DEBUG 1 // verbosity levels
#define SERIAL if(DEBUG) Serial
#define LOG(TYPE, MESSAGE) SERIAL.printf("%s:[%s] > %s\n", TAG, TYPE, MESSAGE)
#define LOG(TYPE, MESSAGE, ...) { SERIAL.printf("%s:[%s] > ", TAG, TYPE); SERIAL.printf(MESSAGE, ##__VA_ARGS__); SERIAL.println(); }
#define LOG_OPERATION(MESSAGE, ...) LOG("OPERATION", MESSAGE, ##__VA_ARGS__) 
#define LOG_ERROR(MESSAGE, ...) LOG("ERROR", MESSAGE, ##__VA_ARGS__)
#define LOG_INTERRUPT(MESSAGE, ...) LOG("INTERRUPT", MESSAGE, ##__VA_ARGS__)
// ======================================================


// DEFINES ==============================================
#define SAMPLE_RATE 75
#define BAUD_RATE 115200
#define INPUT_PIN 34
#define CHIPSELECT 5
#define NUM_FILES 100 // total number of files needed
#define NUM_DP ((DEBUG) ? 1000 : 50000) // total number per file
#define PORT 80 // default server port
#define RESPONSE_TIMEOUT 2000 
#define CHUNK_SIZE 100 // maximum allowable chunk size
#define AP_NAME "BBTech-AP"
#define AP_REACTIVATION_TIMEOUT 10
// ======================================================


// VARIABLES AVAILABLE FOR EXTERNAL USE =================
static const String QUERY_PARAMS = "type=file";
static const String ENDPOINT = "/api/v1/aws/upload/";
static const String HOST = "api.tscc.live";
static const String BOUNDARY = "----------------BOUNDARY";

static String ssid = "<default-ssid>", pwd = "<default-pwd>";
static Ticker ticker;
static AsyncWebServer server(80);
// ======================================================

#endif