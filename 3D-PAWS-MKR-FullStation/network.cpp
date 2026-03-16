/*
 * ======================================================================================================================
 *  network.cpp - MKR modem network related Functions and Definations
 * ======================================================================================================================
 */
#include <time.h>

#include "include/cf.h"
#include "include/time.h"
#include "include/support.h"
#include "include/output.h"
#include "include/network.h"
#include "include/main.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
char ModemFirmwareVersion[32];

NetworkConnectionState ConnectionState = NetworkConnectionState::DISCONNECTED;
unsigned int NoNetworkLoopCycleCount   = 0;
bool NetworkHasBeenOperational         = false;  // Get set if we have successfully transmitted OBS since power on

#if defined(BOARD_HAS_NB)
// Global: declare as pointer (no constructor yet)
NBConnectionHandler* conMan = nullptr;
// NBConnectionHandler conMan(cf_sim_pin, cf_sim_apn, cf_sim_username, cf_sim_password);   // Arduino_NBConnectionHandler.cpp
// NBConnectionHandler conMan(cf_sim_pin);

// NBClient client(false);    // client(true); for debug else false
NBScanner scanner(false);  // scanner(true); for debug else false

NBClient client(true);
// NBScanner scanner(true);

NBModem modem;
NB nb_gsm(false);              // nb_gsm(true); for debug else false
NB nbAccess;

#elif defined(BOARD_HAS_GSM)

// Global: declare as pointer (no constructor yet)
GSMConnectionHandler* conMan = nullptr;
// GSMConnectionHandler conMan(cf_sim_pin, cf_sim_apn, cf_sim_username, cf_sim_password);   // Arduino_GSMConnectionHandler.cpp

GSMClient client(false);    // client(true); for debug else false
GSMScanner scanner(false);  // scanner(true); for debug else false
GSMModem modem;
GSM nb_gsm(false);              // nb(true); for debug else false

#endif


/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * GetCellEpochTime() --  MKRNB getTime() returns local time not GMT - This functions does not
 *
 *   Cell Time
 *   at+CCLK?
 *   +CCLK: "22/07/10,09:15:11-24"
 *   In AT+CCLK? modem response field -24 is time zone information , I am in MT time zone
 *   To Decode Divide by four to get hours to offset. Aka 6 hrs behind GMT aka -6
 *   Length of returned string is 29
 *
 *   AT Command Reference Manual Page 43
 *   "TZ": The Time Zone information is represented by two digits. The value is updated during the registration
 *   procedure when the automatic time zone update is enabled (using +CTZU AT command) and the network
 *   supports the time zone information.
 *  ======================================================================================================================
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
      return (0);
    }
    if (result > 1980223200) { // Chose a time that is in the future. Fri Oct 01 2032 06:00:00 GMT+0000
      Output(F("GNWT:>BAD DATE"));
      return (0);
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
 * GetCellSignalStrength --  return CSS
 *   
 *   scanner.getSignalStrength() -- returns strength and ber
 *     signal strength in 0-31 scale.
 *     0      = 113 dBm or less
 *     1      = 111 dBm
 *     2...30 = 109...53 dBm
 *              2-9   Marginal
 *              10-14 OK
 *              15-19 Good
 *              20+   Excellent
 *     31     = 51 dBm or greater
 *     99     = not known or not detectable
 * ======================================================================================================================
 */
int GetCellSignalStrength() {
  String CSS = "";
  int css;
  char s[32];
  uint32_t ts = stc.getEpoch();

  CSS = scanner.getSignalStrength();
  CSS.toCharArray(s, 32);
  css = atoi (s);
  return (css);
}

/*
 * ======================================================================================================================
 * PrintCurrentCarrier
 * ======================================================================================================================
 */
int PrintCurrentCarrier() {
  String CCS = ""; // Current Carrier String
  int ccs;
  char s[32];

  CCS = scanner.getCurrentCarrier();
  CCS.toCharArray(s, 20);
  sprintf (Buffer32Bytes, "NW:[%s]", s);
  Output (Buffer32Bytes);
}

/*
 * ======================================================================================================================
 * PrintCellSignalStrength
 * ======================================================================================================================
 */
void PrintCellSignalStrength() {
  sprintf (Buffer32Bytes, "CSS:%d", GetCellSignalStrength());
  Output (Buffer32Bytes);
}

/*
 * ======================================================================================================================
 * PrintModemIMEI
 *   An IMEI (International Mobile Equipment Identity) is always 15 decimal digits long:
 *  Digits 1-8: TAC (Type Allocation Code - model/manufacturer)
 *   Digits 9-14: Serial number
 *   Digit 15: Check digit (Luhn algorithm)
 *   char imei[16];  // 15 digits + null terminator
 * ======================================================================================================================
 */
void PrintModemIMEI() {
  String IMEI = "";
  char s[32];

  IMEI = modem.getIMEI();
  IMEI.replace("\n", "");
  IMEI.toCharArray(s, 32);
  sprintf (Buffer32Bytes, "IMEI:%s", s);  // Note: On network error this printed +CEREG: 0,0
  Output (Buffer32Bytes);
}

/*
 * ======================================================================================================================
 * PrintModemFW
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
bool WaitForNetworkConnection(int seconds) {
  int i = 0;

  while (i++ < (seconds * 10)) {
    ConnectionState = conMan->check();
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
  Output(F("NW:Disconnect - Resetting Modem!"));
  modem.hardReset();
  Output(F("NW:Disconnect - Waiting 12s"));
  delay(12000); // Give time for modem to reset
}

/*
 * ======================================================================================================================
 * onNetworkError() -
 * ======================================================================================================================
 */
void onNetworkError() {
  HeartBeat();
  if ((millis() - Time_of_last_hardreset) > (60*5*1000)) {
     
    Output(F("NW:Error - Resetting Modem!"));
    modem.hardReset();
    Output(F("NW:Error - Waiting 12s"));
    delay(12000); // Give time for modem to reset
    Time_of_last_hardreset = millis();
  }

  Output(F("NW:Error - Calling Connect"));
  conMan->connect(); // Try some modem recovery, so next call to check() we will run init() and start connection process all over
  PrintModemIMEI(); // Ask the modem something to see if it is alive
}

/*
 * ======================================================================================================================
 * CM_initialize() - Connection Manager Initialize 
 * ======================================================================================================================
 */
void CM_initialize(){
  Output(F("CM:INIT"));

#if defined(BOARD_HAS_NB)
  conMan = new NBConnectionHandler (cf_sim_pin, cf_sim_apn, cf_sim_username, cf_sim_password);  // Arduino_NBConnectionHandler.cpp
#elif defined(BOARD_HAS_GSM)
  conMan = new GSMConnectionHandler (cf_sim_pin, cf_sim_apn, cf_sim_username, cf_sim_password); // Arduino_GSMConnectionHandler.cpp
#endif

  conMan->addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
  conMan->addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);
  conMan->addCallback(NetworkConnectionEvent::ERROR, onNetworkError);  

  /* 
   * By using updateTimeoutInterval I can change the timeout value for a specific
   * state of the connection handler
   */
                                                                                // Defaults in Milliseconds
  //conMan->updateTimeoutInterval(NetworkConnectionState::INIT, 8000);          // 4000
  //conMan->updateTimeoutInterval(NetworkConnectionState::CONNECTING, 1000);    // 500
  //conMan->updateTimeoutInterval(NetworkConnectionState::CONNECTED, 20000);    // 10000
  //conMan->updateTimeoutInterval(NetworkConnectionState::DISCONNECTING, 200);  // 100
  //conMan->updateTimeoutInterval(NetworkConnectionState::DISCONNECTED, 2000);  // 1000
  //conMan->updateTimeoutInterval(NetworkConnectionState::CLOSED, 2000);        // 1000
  //conMan->updateTimeoutInterval(NetworkConnectionState::ERROR, 2000);         // 1000
        
  Output(F("CM:Connect"));
  
#if defined(BOARD_HAS_NB)
  nbAccess.setTimeout(10000);
#endif

  conMan->connect();  
  Output(F("CM:Connect After"));
}

/*
 * ======================================================================================================================
 * Send_http() - Do a GET/POST request to log observation, process returned text for result code and set return status
 * 
 * We need to return a status of what happened. This status will be used to determine next actions
 *   Posted = do not add to the n2s file
 *   What if faild to connect because network is down - look at source code for client.connect() see if it tests network
 *   Failed to Connect
 *   Timeout waiting for a response
 *   
 * HTTP Response Example
 *   HTTP/1.1 200 OK
 *   Date: Sat, 07 May 2022 17:07:22 GMT
 *   Server: Apache/2.4.29 (Unix) OpenSSL/1.0.2k-fips PHP/7.2.3
 * 
 * SEE https://www.tutorialspoint.com/http/http_responses.htm 
 * SEE https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#successful_responses
 * 
 * Example /measurements/url_create?key=FOOBAR&instrument_id=53&at=2022-05-17T17%3A40%3A04&hth=8770
 * ======================================================================================================================
 */
bool Send_http(char *msg, char *webserver, int webserver_port, char *webserver_path, int webserver_method, char *webserver_xapikey) {
  char response[64];
  char buf[96];
  int r, i=0, exit_timer=0;
  bool posted = false;

  // Convert the JSON to A GET 
  if (webserver_method == METHOD_GET) { 
    if (!json_to_get_string_inplace(webserver_path, msg)) {
      Output(F("OBS:JSON->GET ERR"));
      Output(F("OBS:LOST"));
      return (true);
    }
  }
  
  unsigned long nt = GetCellEpochTime(); // Getting the cell network time does a bunch of checks on if the Network is up.
                                         // So a return of 0 says there are problems, and we should not try to transmit.
  if (!nt) {
    sprintf (Buffer32Bytes,"OBS:SEND->NWTIME BAD");
    Output(Buffer32Bytes);  
    NoNetworkLoopCycleCount++;   // reset modem and reboot if count gets to our set max - done in main loop()
  }
  else {
    Output(F("OBS:SEND->HTTP"));
    if (!client.connect(webserver, webserver_port)) {
      NoNetworkLoopCycleCount++;   // reset modem and reboot if count gets to our set max - done in main loop()
      Output(F("OBS:HTTP FAILED"));
    }
    else {
      NoNetworkLoopCycleCount = 0;  // reset the counter to prevent rebooting 
      Output(F("OBS:HTTP CONNECTED"));

      // Make a HTTP request:
      if (webserver_method == METHOD_GET) { 
        Serial_writeln (msg);

        // Make a HTTP GET request:
        client.print("GET ");
        client.print(msg); // path
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(webserver);
        client.println(String("X-API-Key: ") + webserver_xapikey);
        client.println("Connection: close");
        client.println(); // blank line ends headers
      }
      else { 
        // Construct HTTP POST request    
        client.println(String("POST ") + webserver_path + " HTTP/1.1");
        client.println(String("Host: ") + webserver);
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(strlen(msg));
        client.println(String("X-API-Key: ") + webserver_xapikey);
        client.println("Connection: close");
        client.println(); // blank line after headers
        client.print(msg);
        // Perplexity says to do a client.print(obs) not println;
      }

      Output(F("OBS:HTTP SENT"));

      // Check every 500ms for data, up to 2 minutes. While waiting take Wind Readings every 1s
      exit_timer = 0;
      while(client.connected() && !client.available()) {     
        BackGroundWork(); // 1 Second
        // Prevent infinate waiting - not sure if this is needed but can't hurt
        if (++exit_timer >= 120) { // after 2 minutes lets call it quits
          break;
        }
      }
      
      Output(F("OBS:HTTP WAIT"));
        
      // Read first line of HTTP Response, then get out of the loop
      r=0;
      unsigned long startTime = millis();
      unsigned long timeout = 5000; // 5 seconds timeout 
      bool read_negative_one=false; 
      while ((client.connected() || client.available() ) && r<63 && !posted) {
        if (client.available()) {
          int val = client.read(); //gets byte from buffer 0 = no data, -1 means error
          if (val == -1) {
            // No more data, exit loop
            read_negative_one=true;
            Serial_writeln("");
            Output(F("READ -1"));
            break;           
          }
          else {
            response[r] = (char)val;
            response[++r] = 0;  // Make string null terminated
            if (strstr(response, "200 OK") != NULL) { // Does response includes a "200 OK" substring?
              NetworkHasBeenOperational = true;
              posted = true;
              break;
            }
            if ((response[r-1] == 0x0A) || (response[r-1] == 0x0D)) { // LF or CR
              // if we got here then we never saw the 200 OK
              Serial_writeln("");
              Output(F("OBS:HTTP RESP EOL"));
              break;
            }
          }
        }
        else {
          // No data available yet, check timeout
          if (millis() - startTime > timeout) {
            Serial_writeln("");
            Serial_writeln(F("OBS:HTTP RESP TIMEOUT"));
            break;
          }
          delay(10); // very short delay to prevent tight loop         
        }
      }

      sprintf (buf, "OBS:HTTP RESP[%s]", response); // SEE (E4 53) on failure

      // Print response as hex
      Output(buf);
      Serial_write(response, HEX);
      Serial_write("");
     
      // Early closure of a network connection on the MKR NB 1500 can contribute to communication 
      // instability or the need for modem resets in some edge cases due to how the firmware handles 
      // connection states
      
      // Read rest of the response after first line - I have seen this hang with unknow characters be read
      if (!read_negative_one) {
        int count=0;
        startTime = millis();   
        timeout = 5000; // 5 seconds timeout
        while (client.connected() || client.available()) { //connected or data available
          if (client.available()) {
            int val = client.read(); //gets byte from buffer 0 = no data, -1 means error
            if (val == -1) {
              // No more data, exit loop
              Serial_writeln("");
              Output(F("READ -1"));
              break;           
            }
            else {
              char c = (char)val;
              Serial_write (c);
              if (++count > 1000){
                Serial_writeln("");
                Output("OBS:HTTP RESP BREAK");
                break;
              }
              // Reset startTime each time new data is received
              startTime = millis();
            }
          }
          else {
            // No data available yet, check timeout
            if (millis() - startTime > timeout) {
              Serial_writeln("");
              Output(F("OBS:HTTP RESP TIMEOUT REST"));
              break;
            }
            delay(10); // very short delay to prevent tight loop
          }  
        }
        Serial_writeln("");
      }

      // Server disconnected from clinet. No data left to read. Disconnect client from the server
      client.stop();

      sprintf (buf, "OBS:%sPosted", (posted) ? "" : "Not ");
      Output(buf);
    }
  }
  return (posted);
}
