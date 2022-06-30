#ifndef RTCWRAPPER_H
#define RTCWRAPPER_H

// INCLUDES ----------
#include "Defines.h"
// -------------------

#define TAG "RTC"
#ifdef V2

// INCLUDES ----------
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <exception>
// -------------------


// DEFINES -----------
// [PINS] ------
#define IO 32
#define CLK 33
#define CE 16

// [PARAMS] ----
#define UTC_OFFSET 5.5*60*60 // +5:30 for IST
#define NTP_SERVER "pool.ntp.org"
#define DAYLIGHT_SAVINGS 0
// -------------------

// VARS --------------
static ThreeWire myWire(IO, CLK, CE);
static RtcDS1302<ThreeWire> Rtc(myWire);
// -------------------


// Error Control -----
typedef enum rtc_error {
    RTC_NTP_FAILURE,
    RTC_DATETIME_INVALID,
    RTC_WRITE_PROTECTED,
    RTC_NOT_RUNNING,
    RTC_INVALID_LEAP
} rtc_error;
// -------------------

// Exception class ---
class RTCException : public std::exception {
    private:
    rtc_error err;

    
    public:
    RTCException(rtc_error err) : err(err) {}

    rtc_error getErr() {
        return err;
    }


    void log() const throw () {
        switch (err) {
            case RTC_NTP_FAILURE:
                LOG_ERROR("Failed to contact NTP server");
            break;
            case RTC_DATETIME_INVALID:
                LOG_ERROR("Date Time format is invalid");
            break;
            case RTC_WRITE_PROTECTED:
                LOG_ERROR("Chip in write protected state");
            break;
            case RTC_NOT_RUNNING:
                LOG_ERROR("RTC not running");
            break;
            case RTC_INVALID_LEAP:
                LOG_ERROR("Value contains invalid leap year");
            break;
            default:
                LOG_ERROR("Unknown error");
        }
    }
};
// -------------------


class RTCWrapper {
    public:
    static void begin();

    // setters
    static void configureTime();

    // getters
    static int isDateTimeValid();
    static int isWriteProtected();
    static int isRunning();
    static int isValidLeap(RtcDateTime check);
    static RtcDateTime getDateTime();
};

#endif

#endif