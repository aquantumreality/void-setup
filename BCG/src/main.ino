// INCLUDES =============================================
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <SPI.h>
#include <Ticker.h>
#include <WiFi.h>


#include "Defines.h"
#include "FS.h"
#include "OTASequence.h"
#include "POSTHandler.h"
#include "RTCWrapper.h"
#include "SensorHandler.h"
#include "WiFiManagement.h"
// ======================================================

#define TAG "main"

// EXTERN VARS ==========================================
extern wifiEvent STAConnectEvent, STADisconnectEvent, STAIPEvent, APStartEvent;
extern TaskHandle_t sensorTaskHandler, OTATaskHandler;
// ======================================================


// GLOBAL VARIABLES =====================================
char fileName[20];
String headers;
File myFile;
// ======================================================


// override functionalilty (ADDITION TO LIB)
bool Ticker::active() {
    return (bool)_timer; 
}


void setup() {
  SERIAL.begin(BAUD_RATE);
  SERIAL.println();


  // PRINT BUILD NUMBER
  LOG("VERSION", "<< %s >>", BUILD_NUMBER);


  // TURN ON WIFI ======================================
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pwd.c_str());
  // ===================================================


  // SD CARD INITIALIZE ================================
  LOG_OPERATION("Initializing SD card");
  if (!SD.begin(CHIPSELECT)) {
    LOG_ERROR("Initialization failed, restarting device...");
    esp_restart(); // restart the device
  }
  LOG_INTERRUPT("SD card initialized");
  // ===================================================


  // REGISTER WIFI EVENTS ==============================
  STAConnectEvent.id = WiFi.onEvent(STAConnectEvent.handler, STAConnectEvent.type);
  STADisconnectEvent.id = WiFi.onEvent(STADisconnectEvent.handler, STADisconnectEvent.type);
  STAIPEvent.id = WiFi.onEvent(STAIPEvent.handler, STAIPEvent.type);
  APStartEvent.id = WiFi.onEvent(APStartEvent.handler, APStartEvent.type);
  // ===================================================
  

  // set all the static headers (inc. the POST request)
  setHeaders(headers);


  // REGISTER INITIAL RTOS TASKS =======================
  xTaskCreate(
    recordData,
    "recordData",
    10240,
    NULL,
    1,
    &sensorTaskHandler
  );
  xTaskCreate(
    runOTA,
    "runOTA",
    5120,
    NULL,
    1,
    &OTATaskHandler
  );
  // ===================================================
}


void loop() {
  vTaskDelete(NULL);
}


void recordData(void *params) {
  while (true) {
    // Calculate elapsed time
    static unsigned long past = 0;
    unsigned long present = micros();
    unsigned long interval = present - past;
    past = present;

    // Run timer
    static long timer = 0;
    timer -= interval;

    // SAMPLING AND REPORT
    if (timer < 0 && WiFi.status() == WL_CONNECTED) {
      timer += 1000000UL / SAMPLE_RATE; // calculate the sample rate (preprocessed)

      for (int n = 0; n < NUM_FILES; n++) {
        // create the file and index its name
        sprintf(fileName, "/nightDemo%02d.txt", n);
        LOG_INTERRUPT("File name: %s", fileName);
        myFile = SD.open(fileName, FILE_WRITE);

        // WRITE TO FILE ====================================
        if (myFile) {
          // add the boundary entry
          myFile.print("--" + BOUNDARY + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\nContent-Type: text/plain\r\n\r\n");

          //filling j sensor datapoints into each files
          for (int j = 0; j < NUM_DP; j++) {
            float sensor_value = analogRead(INPUT_PIN);
            float signal = BCGFilter(sensor_value);

            #if (DEBUG == 3)
              LOG("SENSOR", "%f", signal);
            #endif

            myFile.println(signal); //printing to the respective file over SD card
            vTaskDelay(10 / portTICK_PERIOD_MS);              //tune this loop delay to vary waveform logging time
          }

          // add the boundary exit
          myFile.print("\r\n--" + BOUNDARY + "--\r\n");

          //close the file before opening new
          myFile.close();
          LOG_INTERRUPT("File saved");
        }
        else {
          LOG_ERROR("Cannot open file");
        }
        // ==================================================

        // POST SEQUENCE ====================================
        myFile = SD.open(fileName);
        if (myFile) {
          myFile.seek(0, SeekSet); // reset cursor
          WiFiClient client; //  create the client object
          
          // send sequence and get the response
          #if (DEBUG == 2)
            SERIAL.println("[RESPONSE] ----------");
            SERIAL.println(sendFile(myFile, client, headers));
            SERIAL.println("---------------------");
          #else
            sendFile(myFile, client, headers);
          #endif

          myFile.close(); // close file
        }
        else {
          LOG_ERROR("Cannot open file");
        }
        // ==================================================
      }
    }
  }
}
