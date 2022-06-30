// INCLUDES ----------
#include <Arduino.h>
#include <functional>

#include "Defines.h"
#include "iap.h"
#include "OTASequence.h"
// -------------------

#define TAG "OTASequence"

// OTA auth ----------
#define GITHUB_CA_ROOT_CERT \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
"+OkuE6N36B9K\n" \
"-----END CERTIFICATE-----\n"
#define TOKEN "<TOKEN>"
// -------------------


// Vars --------------
request_buffer req;
WiFiClientSecure client;
extern TaskHandle_t sensorTaskHandler;
// -------------------


// Prototypes --------
String initHeaders();
ota_request_err startRequest(WebRequestMethod method);
ota_request_err sendHeaders();
ota_request_err getResponse(size_t buffer_limit, std::function<void(char* response, size_t size)> parser, const uint16_t timeout=2000);
bool indexOf(const char* target, char* response, size_t response_size, uint16_t &i, int16_t &idx, uint16_t &counter);
bool captureUntil(char* buffer, char stop, char* response, size_t response_size, uint16_t &i, uint16_t &counter);
bool flashImage(uint8_t* buffer, size_t size, uint16_t &i, size_t image_size, size_t &image_size_counter);
// -------------------


// Definitions -------
String initHeaders() {
    if (!req.host && !req.endpoint)
        return "";
    
    return "Host: " + req.host + "\r\nUser-Agent: BCG_DEV_ESP32\r\n";
}


ota_request_err startRequest(WebRequestMethod method) {
    String buffer = "";

    if (client.connected())
        client.stop();
    client.connect(req.host.c_str(), 443);

    switch (method) {
        case HTTP_GET:
            buffer = "GET";
        break;
        case HTTP_POST:
            buffer = "POST";
        break;
        default:
            throw SeqException(SEQ_WRONG_METHOD);
    }
    
    if (!client.connected())
        throw SeqException(SEQ_CLIENT_DISCONNECTED);

    if (!req.endpoint.length())
        throw SeqException(SEQ_EMPTY_LINK);

    buffer += " " + req.endpoint + " HTTP/1.1\r\n";
    client.print(buffer);
    return SEQ_OK;
}


ota_request_err sendHeaders() {
    if (!req.headers.length())
        throw SeqException(SEQ_EMPTY_HEADERS);

    if (!client.connected())
        throw SeqException(SEQ_CLIENT_DISCONNECTED);

    String buffer = initHeaders();
    if (!buffer.length())
        throw SeqException(SEQ_EMPTY_LINK);

    client.print(buffer + req.headers + "\r\n");
    return SEQ_OK;
}


ota_request_err getResponse(size_t buffer_limit, std::function<void(char* response, size_t size)> parser, const uint16_t timeout) {
    if (!client.connected())
        throw SeqException(SEQ_CLIENT_DISCONNECTED);

    uint16_t interval = millis() + timeout;
    while (interval > millis() && !client.available());
    if (interval < millis())
        throw SeqException(SEQ_RESPONSE_TIMEOUT);
    
    char* response = (char*) malloc(sizeof(char) * buffer_limit);
    size_t size = 0;

    while (client.available()) {
        size = client.readBytes(response, buffer_limit);
        parser(response, size);
    }

    free(response);
    return SEQ_OK;
}


bool indexOf(const char* target, char* response, size_t response_size, uint16_t &i, int16_t &idx,  uint16_t &counter) {
    size_t target_size = strlen(target);

    for (; i<response_size; ++i) {
        if (target[counter] == response[i]) {
            ++counter;
            if (counter == target_size) {
                idx = i - 1;
                break;
            }
        } else {
            counter = 0;
        }
    }

    if (idx != -1)
        return true;
    return false;
}


bool captureUntil(char* buffer, char stop, char* response, size_t response_size, uint16_t &i, uint16_t &counter) {
    for (;i<response_size; ++i) {
        if (response[i] == stop) {
            buffer[counter] = '\0';
            return true;
        } else {
            buffer[counter++] = response[i];
        }
    }
    return false;
}


bool flashImage(uint8_t* buffer, size_t size, uint16_t &i, size_t image_size, size_t &image_size_counter) {
    if ((image_size_counter+(size-i)) > image_size) {
        LOG_ERROR("Mismatch size: %d", image_size_counter+(size-i));
        vTaskResume(sensorTaskHandler);
        throw SeqException(SEQ_IMAGE_SIZE_MISMATCH);
    }
    if (iap_write(buffer+i, size-i) != IAP_OK) { // write with offset
        vTaskResume(sensorTaskHandler);
        throw SeqException(SEQ_IAP_FAILED);
    }

    image_size_counter += (size-i); // update image_size_counter
    i = size; // update for next

    // check if complete
    if (image_size_counter == image_size) {
        return true;
    }
    return false;
}
// -------------------


// Runnable ----------
void runOTA(void *params) {
    while(WiFi.status() != WL_CONNECTED);

    client.setCACert(GITHUB_CA_ROOT_CERT);

    for (;;) {
        LOG_OPERATION("Checking for update...");

        char* buffer = (char*) malloc(sizeof(char) * 600);

        try {
            int16_t idx = -1;
            uint16_t counter = 0;
            uint8_t sequence_code = 0;

            // request for latest release
            req = {
                .host = "api.github.com",
                .endpoint = "/repos/Blackbeard-Technologies/hardware-Iot/releases/latest",
                .headers = "Accept: */*\r\nAuthorization: token " + String(TOKEN) + "\r\n"
            };
            startRequest(HTTP_GET);
            sendHeaders();
            getResponse(200, [&buffer, &idx, &counter, &sequence_code](char* response, size_t size) {
                for (uint16_t i=0; i<size;) {
                    switch (sequence_code) {
                        case 0: // get the tag name
                            if (indexOf("\"tag_name\":", response, size, i, idx, counter)) {
                                idx = -1;
                                counter = 0;
                                ++sequence_code;
                                i+=2;
                            }
                        break;
                        case 1: // capture the tag name
                            if (captureUntil(buffer, '\"', response, size, i, counter)) {
                                LOG_INTERRUPT("Tag Name: %s", buffer);
                                if (!strcmp(buffer, BUILD_NUMBER)) // compare versions
                                    throw SeqException(SEQ_OUTDATED_VERSION);
                                counter = 0;
                                ++sequence_code;
                            }
                        break;
                        case 2: // find the assets
                            if (indexOf("\"assets\"", response, size, i, idx, counter)) {
                                idx = -1;
                                counter = 0;
                                ++sequence_code;
                            }
                        break;
                        case 3: // find the url
                            if (indexOf("\"url\":", response, size, i, idx, counter)) {
                                idx = -1;
                                counter = 0;
                                ++sequence_code;
                                i+=2; // skipping " char
                            }
                        break;
                        case 4: // capture the url
                            if (captureUntil(buffer, '\"', response, size, i, counter)) {
                                #if (DEBUG==2)
                                    LOG_INTERRUPT("Asset URL: %s", buffer);
                                #endif
                                counter = 0;
                                ++sequence_code;
                            }
                        break;
                        default:
                            ++i;
                    }
                }
            });
            if (sequence_code == 4) // in case delimiter not found
                #if (DEBUG==2)
                    LOG_INTERRUPT("Asset URL: %s", buffer);
                #endif
            if (sequence_code < 4)
                throw SeqException(SEQ_INVALID_RESPONSE);

            // request the download link
            sequence_code = 0;
            req = {
                .host = "api.github.com",
                .endpoint = String(buffer).substring(22),
                .headers = "Accept: application/octet-stream\r\nAuthorization: token " + String(TOKEN) + "\r\n"
            };
            startRequest(HTTP_GET);
            sendHeaders();
            getResponse(200, [&buffer, &idx, &counter, &sequence_code](char* response, size_t size) {
                for (uint16_t i=0; i<size;) {
                    switch (sequence_code) {
                        case 0:
                            if (indexOf("Location: ", response, size, i, idx, counter)) {
                                idx = -1;
                                counter = 0;
                                ++sequence_code;
                                ++i;
                            }
                        break;
                        case 1:
                            if (captureUntil(buffer, '\r', response, size, i, counter)) {
                                #if (DEBUG==2)
                                    LOG_INTERRUPT("Location: %s", buffer);
                                #endif
                                counter = 0;
                                ++sequence_code;
                            }
                        break;
                        default:
                            ++i;
                    }
                }
            });
            if (sequence_code == 1)
                #if (DEBUG==2) 
                    LOG_INTERRUPT("Download Location: %s", buffer); 
                #endif
            if (sequence_code < 1)
                throw SeqException(SEQ_INVALID_RESPONSE);

            // download the image and save in flash
            size_t image_size_counter = 0;
            req = {
                .host = "github-releases.githubusercontent.com",
                .endpoint = String(buffer).substring(45),
                .headers = "Accept: application/octet-stream\r\nConnection: keep-alive\r\n"
            };
            startRequest(HTTP_GET);
            sendHeaders();
            size_t image_size = 0;
            sequence_code = 0;
            getResponse(200, [&buffer, &image_size, &image_size_counter, &idx, &counter, &sequence_code](char *response, size_t size) {
                for (uint16_t i=0; i<size;) {
                    switch (sequence_code) {
                        case 0: // get the size of the image
                            if (indexOf("Content-Length:", response, size, i, idx, counter)) {
                                idx = -1;
                                counter = 0;
                                ++sequence_code;
                                i+=2;
                            }
                        break;
                        case 1:
                            if (captureUntil(buffer, '\r', response, size, i, counter)) {
                                LOG_INTERRUPT("Content-Length: %sB", buffer);
                                image_size = String(buffer).toInt();
                                counter = 0;
                                ++sequence_code;
                            }
                        break;
                        case 2:
                            if (indexOf("\r\n\r\n", response, size, i, idx, counter)) {
                                #if (DEBUG==2)
                                    LOG_INTERRUPT("Double CRLF detected");
                                #endif
                                idx = -1;
                                counter = 0;
                                ++i;
                                ++sequence_code;
                            }
                        break;
                        case 3: // init iap
                            LOG("IAP_INIT", "Initializing IAP");
                            vTaskSuspend(sensorTaskHandler); // stop other tasks
                            iap_init();
                            if (iap_begin() == IAP_OK) {
                                ++sequence_code;
                                LOG("WRITE", "Writing to partition");
                            } else {
                                iap_abort();
                                vTaskResume(sensorTaskHandler);
                                throw SeqException(SEQ_IAP_FAILED);
                            }
                        break;
                        case 4:
                            vTaskDelay(10 / portTICK_PERIOD_MS);
                            if (flashImage((uint8_t*)response, size, i, image_size, image_size_counter)) { // flash the buffer
                                 ++sequence_code;
                                 LOG("WRITE", "Flash Written");
                            }
                        break;
                        default:
                            ++i;
                    }
                }
            }, 10000);
            if (sequence_code == 4) {
                iap_abort();
                vTaskResume(sensorTaskHandler);
                throw SeqException(SEQ_IAP_FAILED);
            }
            if (sequence_code == 5) {
                LOG("COMMIT", "Commiting new firmware");
                if (iap_commit() == IAP_OK) {
                    LOG_INTERRUPT("Restarting in 3 seconds...");
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    esp_restart();
                } else {
                    vTaskResume(sensorTaskHandler);
                    throw SeqException(SEQ_IAP_FAILED);
                }
            }

            free(buffer);
        } catch (SeqException &err) {
            err.log();
            free(buffer);
            if (err.getErr() == SEQ_IMAGE_SIZE_MISMATCH || err.getErr() == SEQ_IAP_FAILED)
                iap_abort();
        }

        // try after some time
        vTaskDelay((DEBUG ? 10000 : 1000*60*15) / portTICK_PERIOD_MS);
    }
}
// -------------------
