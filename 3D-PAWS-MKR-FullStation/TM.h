/*
 * ======================================================================================================================
 *  TM.h - Time Management
 *  
 *  Functions below depend on libraries
 *     #include <SdFat.h>  For defining: struct tm 
 *     #include <RTClib.h> For defining: DateTime
 *
 *  class RTC_DS3231 : RTC_I2C
 *  public:
 *    bool begin(TwoWire *wireInstance = &Wire);
 *    void adjust(const DateTime &dt);
 *    bool lostPower(void);
 *    DateTime now();
 *    Ds3231SqwPinMode readSqwPinMode();
 *    void writeSqwPinMode(Ds3231SqwPinMode mode);
 *    bool setAlarm1(const DateTime &dt, Ds3231Alarm1Mode alarm_mode);
 *    bool setAlarm2(const DateTime &dt, Ds3231Alarm2Mode alarm_mode);
 *    void disableAlarm(uint8_t alarm_num);
 *    void clearAlarm(uint8_t alarm_num);
 *    bool alarmFired(uint8_t alarm_num);
 *    void enable32K(void);
 *    void disable32K(void);
 *    bool isEnabled32K(void);
 *    float getTemperature(); // in Celsius degree
 * ======================================================================================================================
 */

unsigned long GetCellEpochTime();  // Prototype this function to aviod compile function unknown issue.

DateTime now;
char timestamp[32];
bool RTC_exists = false;
bool RTC_enabled = false;
bool RTC_valid = false;
bool STC_valid = false;
unsigned long LastTimeUpdate = 0;
unsigned long NoClockRecheckTime = 0;


/* 
 *=======================================================================================================================
 * rtc_timestamp() - Read from RTC and set timestamp string
 *=======================================================================================================================
 */
void rtc_timestamp() {
  now = rtc.now(); // get the current rtc date-time

  // ISO_8601 Time Format
  sprintf (timestamp, "%d-%02d-%02dT%02d:%02d:%02d", 
    now.year(), now.month(), now.day(),
    now.hour(), now.minute(), now.second());
}

/* 
 *====================================================:1===================================================================
 * stc_timestamp() - Read from STC and set timestamp string
 *=======================================================================================================================
 */
void stc_timestamp() {
  // ISO_8601 Time Format
  sprintf (timestamp, "%d-%02d-%02dT%02d:%02d:%02d", 
    stc.getYear()+2000, stc.getMonth(), stc.getDay(),
    stc.getHours(), stc.getMinutes(), stc.getSeconds());
}

/* 
 *=======================================================================================================================
 * rtc_initialize()
 *=======================================================================================================================
 */
void rtc_initialize() {

  if (!rtc.begin()) { // Always returns true
     Output("RTC:NF ERR");
     SystemStatusBits |= SSB_RTC; // Turn on Bit
     return;
  }
 
  if (!I2C_Device_Exist(0x68)) {
    Output(F("RTC:I2C NF ERR"));
    SystemStatusBits |= SSB_RTC; // Turn on Bit
    delay (5000);
    return;
  }

  RTC_exists = true; // We have a clock hardware connected

  rtc_timestamp();
  sprintf (msgbuf, "%sR", timestamp);
  Output (msgbuf);

  // Do a validation check on the year. 
  // Asumption is: If RTC not set, it will not have the current year.
  
  if ((now.year() >= 2025) && (now.year() <= 2035)) {
    RTC_valid = true;
    Output(F("RTC:VALID"));
    
    now = rtc.now();
    stc.setEpoch(now.unixtime());
    STC_valid = true;
    Output(F("STC:VALID"));
  }
  else {
    Output (F("RTC:TIME ERR"));
  }
}

/*
 * ======================================================================================================================
 * NetworkTimeManagement() - Check if we need to Set or Update the RTC clock from the Cell Network   
 * 
 * Note: If you ask the modem for network time each loop cycle the modem will crash. It's just too much.
 * ======================================================================================================================
 */
void NetworkTimeManagement() {

    if (RTC_exists) {
      
      // We have a RTC and We have connected to the Cell network at some point
      if (!RTC_valid || (LastTimeUpdate == 0)) {
        if (ConnectionState == NetworkConnectionState::CONNECTED) {
          // Set Uninitialized RTC from Cell Network.
          // Also since the RTC was invalid we don't have a valid system clock. We need to set that too.
          unsigned long networktime = GetCellEpochTime(); // This is UTC time.
          if (networktime) {
            // We have network because we received time back
            sprintf (Buffer32Bytes, "NW:EPOCH %u", networktime);
            Serial_writeln(Buffer32Bytes);

            // Set Real Time Clock
            DateTime dt_networktime = DateTime(networktime);
            if ((dt_networktime.year() >= TM_VALID_YEAR_START) && (dt_networktime.year() <= TM_VALID_YEAR_END)) {
              rtc.adjust(dt_networktime);
              Output(F("RTC:SET"));
              rtc_timestamp();
              sprintf (msgbuf, "%sR", timestamp);
              Output (msgbuf);
              RTC_valid = true;

              // Set System Time Clock
              stc.setEpoch(networktime);
              Output(F("STC:SET"));
              stc_timestamp();
              sprintf (msgbuf, "%sS", timestamp);
              Output (msgbuf);
          
              STC_valid = true;  // Once this is set we can start taking observations. Because know the time.
        
              LastTimeUpdate = networktime;
            }
            else {
              Output(F("RTC:NOT SET-BAD YR"));
            }
          }
          else {
            if (RTC_valid) {
              Output(F("RTC:OK NWTM:BAD"));
            }
            else {
              Output(F("RTC:BAD NWTM:BAD"));
            }
          }
        }
      }
      // We have a valid system clock and real time clock, diff if we need to sync ntc to the rtc
      else if ((stc.getEpoch() - LastTimeUpdate) >= 2*3600) { // 2hrs    
        unsigned long networktime = GetCellEpochTime(); // This is UTC time. 
        if (networktime) { // Chose a time that is in the past. 
          // Update Real Time Clock
          DateTime dt_networktime = DateTime(networktime);
          if ((dt_networktime.year() >= TM_VALID_YEAR_START) && (dt_networktime.year() <= TM_VALID_YEAR_END)) {
            rtc.adjust(dt_networktime);
            Output(F("RTC:UPDATED"));
            rtc_timestamp();
            sprintf (msgbuf, "%sR", timestamp);
            Output (msgbuf);
          
            RTC_valid = true;  // just because

            // Set System Time Clock
            stc.setEpoch(networktime);
            Output(F("STC:UPDATED"));
          
            STC_valid = true; // just because   
          }
          else {
            Output(F("RTC:NOT UPDATED-BAD YR"));           
          }             
        }
        else {
          Output(F("RTC:NOT UPDATED"));
        }
        // If the network clock was good or bad set up to do another stc update in 2 hours
        LastTimeUpdate = stc.getEpoch();
      }
    }
    
    else { // No Real Time Clock, So we are running with Network and System Clocks, waiting for network clock to be valid
      
      if ( (!STC_valid && (LastTimeUpdate == 0)) || 
           (!STC_valid && ( (stc.getEpoch() - NoClockRecheckTime) >= 0)) // 60s backoff check for network time
         ){
        unsigned long networktime = GetCellEpochTime(); // This is UTC time. 
        if (networktime) {
          DateTime dt_networktime = DateTime(networktime);
          if ((dt_networktime.year() >= TM_VALID_YEAR_START) && (dt_networktime.year() <= TM_VALID_YEAR_END)) {
            // Set System Time Clock
            stc.setEpoch(networktime);
            Output(F("STC:SET"));
            stc_timestamp();
            sprintf (msgbuf, "%sS", timestamp);
            Output (msgbuf);
          
            STC_valid = true; // Once this is set we can start taking observations. Because know the time.

            LastTimeUpdate = networktime;
          }
          else {
            NoClockRecheckTime = 60 + stc.getEpoch(); // Recheck for network time in 60 seconds
          }
        }
        else {
          NoClockRecheckTime = 60 + stc.getEpoch(); // Recheck for network time in 60 seconds
        }
      }

      // STC Valid but no RTC so we use Network Time to update system clock every 2 hours to prevent system clock drift.
      else if ((stc.getEpoch() - LastTimeUpdate) >= 2*3600) {  
        
        // It's been 2 hours since last NTC=>RTC update
        unsigned long networktime = GetCellEpochTime(); // This is UTC time. 
        if (networktime) {
          DateTime dt_networktime = DateTime(networktime);
          if ((dt_networktime.year() >= TM_VALID_YEAR_START) && (dt_networktime.year() <= TM_VALID_YEAR_END)) {
            // Set System Time Clock
            stc.setEpoch(networktime);
            Output(F("STC:UPDATED"));
            STC_valid = true; // just because
                  
            LastTimeUpdate = networktime; 
          }
          else {
            Output(F("STC:UPD BAD-YR"));
            LastTimeUpdate = stc.getEpoch(); // we set this to prevent asking for network time each loop cycle.
          }
        }
        else {
          Output(F("STC:UPD NO NC")); // No Network Clock
          LastTimeUpdate = stc.getEpoch(); // we set this to prevent asking for network time each loop cycle.
        }
      }    
    }
}
