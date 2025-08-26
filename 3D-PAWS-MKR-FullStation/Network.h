/*
 * ======================================================================================================================
 *  Network.h
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * FYI NB = NarrowBand 
 * FYI https://www.arduino.cc/reference/en/libraries/mkrnb/
 *  
 * FYI Below Defined in Arduino_ConnectionHandler.h
 * #ifdef ARDUINO_SAMD_MKRNB1500
 * #include <MKRNB.h>
 * #define BOARD_HAS_NB
 * #define NETWORK_HARDWARE_ERROR
 * #define NETWORK_IDLE_STATUS NB_NetworkStatus_t::IDLE
 * #define NETWORK_CONNECTED NB_NetworkStatus_t::GPRS_READY
 * #endif
 * 
 * #ifdef ARDUINO_SAMD_MKRGSM1400
 * #include <MKRGSM.h>
 * #define BOARD_HAS_GSM
 * #define NETWORK_HARDWARE_ERROR GPRS_PING_ERROR
 * #define NETWORK_IDLE_STATUS GSM3_NetworkStatus_t::IDLE
 * #define NETWORK_CONNECTED GSM3_NetworkStatus_t::GPRS_READY
 * #endif
 *  
 * NB_TIMEOUT = 30000;
 *  
 * If you see "SIM not present or wrong PIN" check the antenna connection. 
 *  
 * Check intervals when calling check() from your program
 *   INIT          100
 *   CONNECTING    500
 *   CONNECTED     10000
 *   DISCONNECTING 100
 *   DISCONNECTED  1000
 *   CLOSED        1000
 *   ERROR         1000
 * ======================================================================================================================
 */

unsigned long LastTimeConManCalled = 0;   // used to control how often we trey and restart the modem

#if defined(BOARD_HAS_NB)
//NBConnectionHandler conMan(cf_sim_pin, cf_sim_apn, cf_sim_username, cf_sim_password);   // Arduino_NBConnectionHandler.cpp
NBConnectionHandler conMan(cf_sim_pin);
NBClient client(false);    // client(true); for debug else false
NBScanner scanner(false);  // scanner(true); for debug else false
NBModem modem;
NB nb_gsm(false);              // nb(true); for debug else false

#elif defined(BOARD_HAS_GSM)
GSMConnectionHandler conMan(cf_sim_pin, cf_sim_apn, cf_sim_username, cf_sim_password);   // Arduino_GSMConnectionHandler.cpp
GSMClient client(false);    // client(true); for debug else false
GSMScanner scanner(false);  // scanner(true); for debug else false
GSMModem modem;
GSM nb_gsm(false);              // nb(true); for debug else false
#endif



/*
 * ======================================================================================================================
 *  GetCellEpochTime() --  MKRNB getTime() returns local time not GMT - This functions does not
 *  
 *  Cell Time
 *  at+CCLK?
 *  +CCLK: "22/07/10,09:15:11-24"
 *  In AT+CCLK? modem response field -24 is time zone information , I am in MT time zone
 *  To Decode Divide by four to get hours to offset. Aka 6 hrs behind GMT aka -6
 *  Length of returned string is 29
 *  
 *  AT Command Reference Manual Page 43
 *  "TZ": The Time Zone information is represented by two digits. The value is updated during the registration
 *  procedure when the automatic time zone update is enabled (using +CTZU AT command) and the network
 *  supports the time zone information.
 * ======================================================================================================================
 */
unsigned long GetCellEpochTime()
{
  String response;

  if (ConnectionState != NetworkConnectionState::CONNECTED) {
    Output("GNWT:NOT CONNECTED");
    return 0;    
  }

  // Do the below request in case Connection Handler is not current with true connection state.
  int NetworkAccessStatus = nb_gsm.isAccessAlive();  // Check network access status
  if (!NetworkAccessStatus) {
    Output(F("GNWT:NO ACCESS"));
    return 0;
  }

  MODEM.send("AT+CCLK?");
  if (MODEM.waitForResponse(100, &response) != 1) {
    Output(F("GNWT:TIMEOUT"));
    return 0;
  }

  // Have seen cases where we are out of sync with serial modem and are seeing AT commands returns
  // Lets output what we are reading from the modem.
  char s[64];
  response.toCharArray(s, 64);
  Serial_writeln(s);

  if (response.length() != 29) {
    Output(F("GNWT:WRONG LEN"));
    return 0;    
  }

  // I know use a regular expression for the below - got lazy.
  if  (isDigit(response.charAt(8)) &&
       isDigit(response.charAt(9)) &&
       (response.charAt(10) == '/') &&
       isDigit(response.charAt(11)) &&
       isDigit(response.charAt(12)) &&
       (response.charAt(13) == '/') &&
       isDigit(response.charAt(14)) &&
       isDigit(response.charAt(15)) &&
       (response.charAt(16) == ',') &&
       isDigit(response.charAt(17)) &&
       isDigit(response.charAt(18)) &&
       (response.charAt(19) == ':') &&
       isDigit(response.charAt(20)) &&
      isDigit(response.charAt(21)) &&
       (response.charAt(22) == ':') &&
       isDigit(response.charAt(23)) &&
       isDigit(response.charAt(24)) &&
       ((response.charAt(25) == '-') || (response.charAt(25) == '+')) &&
       isDigit(response.charAt(26)) &&
       isDigit(response.charAt(27)) 
      ) {           
    struct tm now;
  
    // +CCLK: "22/07/10,09:15:11-24
    // 0123456789012345678901234567
    // L0.0.00.00.05.06,A.02.01  if this version do not add the utc offset
    now.tm_year    = ((response.charAt(8)  - '0') * 10 + (response.charAt(9)  - '0')) + 100;
    now.tm_mon     = ((response.charAt(11) - '0') * 10 + (response.charAt(12) - '0')) - 1;
    now.tm_mday    = ((response.charAt(14) - '0') * 10 + (response.charAt(15) - '0'));
    now.tm_hour    = ((response.charAt(17) - '0') * 10 + (response.charAt(18) - '0'));
    now.tm_min     = ((response.charAt(20) - '0') * 10 + (response.charAt(21) - '0'));
    now.tm_sec     = ((response.charAt(23) - '0') * 10 + (response.charAt(24) - '0'));

    time_t result = mktime(&now);
    time_t delta = ((response.charAt(26) - '0') * 10 + (response.charAt(27) - '0')) * (15 * 60);

    // If old firmware (L0.0.00.00.05.06,A.02.01) do nothing, New firmware is (L0.0.00.00.05.12,A.02.21)
    // Old firmware is already at GMT, New is Localtime
    if (strcmp(ModemFirmwareVersion, "L0.0.00.00.05.06,A.02.01") != 0) {
      if (response.charAt(25) == '-') {
        result += delta;
      } else if (response.charAt(25) == '+') {
        result -= delta;
     }
    }
    else {
      Output(F("GNWT:Delta Not Added"));
    }

    // https://www.unixtimestamp.com/
    if (result < 1664604000) { // Chose a time that is in the past. Sat Oct 01 2022 06:00:00 GMT+0000
      Output(F("GNWT:<BAD DATE"));
      return(0);
    }
    if (result > 1980223200) { // Chose a time that is in the future. Fri Oct 01 2032 06:00:00 GMT+0000
      Output(F("GNWT:>BAD DATE"));
      return(0);     
    }
    sprintf (Buffer32Bytes, "GNWT:OK[%u]", result);
    Output (Buffer32Bytes);
    return result;
  }
  else {
    Output(F("GNWT:BAD FORMAT"));
    return (0);
  }
}

/*
 * ======================================================================================================================
 *  GetCellSignalStrength 
 *  
 *  returns strength and ber
 *  signal strength in 0-31 scale.
 *  0 113 dBm or less
 *  1 111 dBm
 *  2...30 109... 53 dBm
 *    2-9 Marginal 
 *    10-14 OK 
 *    15-19 Good 
 *    20+ Ecellent
 *  31 51 dBm or greater 
 *  99 not known or not detectable
 * ======================================================================================================================
 */
int GetCellSignalStrength() {
  String CSS="";
  int css;
  char s[32];
  uint32_t ts = stc.getEpoch();

  CSS=scanner.getSignalStrength();
  CSS.toCharArray(s, 32);
  css = atoi (s);
  return (css);
}

/*
 * ======================================================================================================================
 *  PrintCurrentCarrier
 * ======================================================================================================================
 */
int PrintCurrentCarrier() {
  String CCS="";  // Current Carrier String
  int ccs;
  char s[32];

  CCS=scanner.getCurrentCarrier();
  CCS.toCharArray(s, 20);
  sprintf (Buffer32Bytes, "NW:[%s]", s);
  Output (Buffer32Bytes);
}

/*
 * ======================================================================================================================
 *  PrintCellSignalStrength 
 * ======================================================================================================================
 */
void PrintCellSignalStrength() {
  sprintf (Buffer32Bytes, "CSS:%d", GetCellSignalStrength());
  Output (Buffer32Bytes);
}

/*
 * ======================================================================================================================
 *  PrintModemIMEI
 * ======================================================================================================================
 */
void PrintModemIMEI() {
  String IMEI="";
  char s[32];

  IMEI = modem.getIMEI();
  IMEI.replace("\n", "");
  IMEI.toCharArray(s, 32);
  sprintf (Buffer32Bytes, "IMEI:%s", s);  // Note: On network error this printed +CEREG: 0,0
  Output (Buffer32Bytes);
}

/*
 * ======================================================================================================================
 *  PrintModemFW
 * ======================================================================================================================
 */
void PrintModemFW() {
  String response;
  
  MODEM.send("ATI9");
  if (MODEM.waitForResponse(100, &response) != 1) {
    Output(F("PMFW:TIMEOUT"));
    return;
  }
  response.toCharArray(ModemFirmwareVersion, 32);
  Output (ModemFirmwareVersion);
}

/*
 * ======================================================================================================================
 * WaitForNetworkConnection() - 
 * ======================================================================================================================
 */
bool WaitForNetworkConnection(int seconds)
{
  int i=0;

  while (i++<(seconds*10)) {
    ConnectionState = conMan.check();
    delay(100);
  }
}

// Assumption: 
// It seems these routines are informational. We can go to a Connected state after reporting Disconnect with out 
// reporting Connected by the below routine. So we should not get network state from here. I guess we assume we are 
// always connected. Unless we go in to onNetworkError. I dont think we ever recover. At least with the 2018 firmware.
/*
 * ======================================================================================================================
 * onNetworkConnect() - 
 * ======================================================================================================================
 */
void onNetworkConnect() {
  Output(F("NW:Connect"));
  PrintCurrentCarrier();
}

/*
 * ======================================================================================================================
 * onNetworkDisconnect() - 
 * ======================================================================================================================
 */
void onNetworkDisconnect() {
  HeartBeat();
#if defined(BOARD_HAS_NB)
  Output(F("NW:Disconnect - Reseting Modem!"));
  modem.hardReset();
#else
  Output(F("NW:Disconnect - Restarting Modem!"));
  modem.restart();
#endif 
  
  Output(F("NW:Disconnect - Waiting 10s"));
  delay(10000); // Give time for modem to reset
}

/*
 * ======================================================================================================================
 * onNetworkError() - 
 * ======================================================================================================================
 */
void onNetworkError() {
  HeartBeat();
#if defined(BOARD_HAS_NB)
  Output(F("NW:Error - Reseting Modem!"));
  modem.hardReset();
#else
  Output(F("NW:Error - Restarting Modem!"));
  modem.restart();
#endif

  Output(F("NW:Error - Waiting 10s"));
  delay(10000); // Give time for modem to reset
  
  Output(F("NW:Error - Calling Connect"));
  conMan.connect(); // Try some modem recovery, so next call to check() we will run init() and start connection process all over
  PrintModemIMEI(); // Ask the modem something to see if it is alive
}
