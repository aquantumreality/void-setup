// INCLUDES ----------
#include <Arduino.h>

#include "Defines.h"
#include "RTCWrapper.h"
// -------------------

// CONSTRUCTORS -----------
void RTCWrapper::begin() {
    Rtc.Begin();
}
// ------------------------

// SETTERS ----------------
void RTCWrapper::configureTime() {
    configTime(UTC_OFFSET, DAYLIGHT_SAVINGS, NTP_SERVER);
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        throw RTCException(RTC_NTP_FAILURE);
    RtcDateTime current_time(timeinfo.tm_year - 100
                            ,timeinfo.tm_mon + 1
                            ,timeinfo.tm_mday
                            ,timeinfo.tm_hour
                            ,timeinfo.tm_min
                            ,timeinfo.tm_sec);
    Rtc.SetDateTime(current_time);
    #if (DEBUG == 1)
        SERIAL.println(&timeinfo, "NTP Time: %A, %B %d %Y %H:%M:%S");
    #endif
}
// ------------------------

// GETTERS ----------------
int RTCWrapper::isDateTimeValid() {
    if (!Rtc.IsDateTimeValid())
        throw RTCException(RTC_DATETIME_INVALID);
    return 1;
}


int RTCWrapper::isWriteProtected() {
    if (Rtc.GetIsWriteProtected())
        throw RTCException(RTC_WRITE_PROTECTED);
    return 0;
}


int RTCWrapper::isRunning() {
    if (!Rtc.GetIsRunning())
        throw RTCException(RTC_NOT_RUNNING);
    return 0;
}


int RTCWrapper::isValidLeap(RtcDateTime check) {
    if (!check.IsValid())
        throw RTCException(RTC_INVALID_LEAP);
    return 1;
}


RtcDateTime RTCWrapper::getDateTime() {
    RtcDateTime now = Rtc.GetDateTime();
    isValidLeap(now); // throws error on failure
    return now;
}
// ------------------------