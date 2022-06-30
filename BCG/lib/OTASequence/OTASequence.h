#ifndef OTASEQUENCE_H
#define OTASEQUENCE_H

#define TAG "OTASequence"

// INCLUDES ----------
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <exception>
#include <WiFiClientSecure.h>

#include "Defines.h"
#include "esp_ota_ops.h"
// -------------------


// Runnable ----------
void runOTA(void *params);
// -------------------


// Request Control ---
typedef struct request_buffer {
    String host, endpoint, headers;
    char *response;
} request_buffer;
// -------------------


// Error Control -----
typedef enum ota_request_err {
    SEQ_OK,
    SEQ_CLIENT_DISCONNECTED,
    SEQ_WRONG_METHOD,
    SEQ_EMPTY_HEADERS,
    SEQ_EMPTY_LINK,
    SEQ_RESPONSE_TIMEOUT,
    SEQ_INVALID_RESPONSE,
    SEQ_OUTDATED_VERSION,
    SEQ_IAP_FAILED,
    SEQ_IMAGE_SIZE_MISMATCH
} ota_request_err;
// -------------------

// Exception class ---
class SeqException : public std::exception {
    private:
    ota_request_err err;

    
    public:
    SeqException(ota_request_err err) : err(err) {}

    ota_request_err getErr() {
        return err;
    }


    void log() const throw () {
        switch (err) {
            case SEQ_CLIENT_DISCONNECTED:
                LOG_ERROR("WiFi client disconnected");
            break;
            case SEQ_WRONG_METHOD:
                LOG_ERROR("Wrong request method");
            break;
            case SEQ_EMPTY_HEADERS:
                LOG_ERROR("Empty headers");
            break;
            case SEQ_EMPTY_LINK:
                LOG_ERROR("Host/Endpoint missing");
            break;
            case SEQ_RESPONSE_TIMEOUT:
                LOG_ERROR("Response timeout");
            break;
            case SEQ_INVALID_RESPONSE:
                LOG_ERROR("Invalid Response");
            break;
            case SEQ_OUTDATED_VERSION:
                LOG_ERROR("Outdated Version");
            break;
            case SEQ_IAP_FAILED:
                LOG_ERROR("IAP failed");
            break;
            case SEQ_IMAGE_SIZE_MISMATCH:
                LOG_ERROR("Image size mismatch");
            break;
            default:
                LOG_ERROR("Unknown error");
        }
    }
};
// -------------------

#endif