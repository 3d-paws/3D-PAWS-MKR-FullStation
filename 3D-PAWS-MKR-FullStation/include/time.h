/*
 * ======================================================================================================================
 *  time.h - Time Management Definations
 * ======================================================================================================================
 */
#include <RTClib.h>
#include <RTCZero.h>            // System Clock

#define TM_VALID_YEAR_START    2026
#define TM_VALID_YEAR_END      2035

// Extern variables
extern RTC_DS3231 rtc;
extern RTCZero stc;
extern DateTime now;
extern char timestamp[32];
extern bool RTC_exists;
extern bool RTC_enabled;
extern bool RTC_valid;
extern bool STC_valid;
extern unsigned long LastTimeUpdate;
extern unsigned long NoClockRecheckTime;


// Function prototypes
uint32_t rtc_unixtime();
void rtc_timestamp();
void stc_timestamp();
void rtc_initialize();
void NetworkTimeManagement();
