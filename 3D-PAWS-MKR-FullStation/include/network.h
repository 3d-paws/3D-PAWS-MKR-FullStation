/*
 * ======================================================================================================================
 * network.h - MKR modem network related Definations
 * ======================================================================================================================
 */
#include <Arduino_ConnectionHandler.h>  // Manage Cell Network Connection (Modified)

/*
   ======================================================================================================================
   FYI NB = NarrowBand
   FYI https://www.arduino.cc/reference/en/libraries/mkrnb/

   FYI Below Defined in Arduino_ConnectionHandler.h
   #ifdef ARDUINO_SAMD_MKRNB1500
   #include <MKRNB.h>
   #define BOARD_HAS_NB
   #define NETWORK_HARDWARE_ERROR
   #define NETWORK_IDLE_STATUS NB_NetworkStatus_t::IDLE
   #define NETWORK_CONNECTED NB_NetworkStatus_t::GPRS_READY
   #endif

   #ifdef ARDUINO_SAMD_MKRGSM1400
   #include <MKRGSM.h>
   #define BOARD_HAS_GSM
   #define NETWORK_HARDWARE_ERROR GPRS_PING_ERROR
   #define NETWORK_IDLE_STATUS GSM3_NetworkStatus_t::IDLE
   #define NETWORK_CONNECTED GSM3_NetworkStatus_t::GPRS_READY
   #endif

   NB_TIMEOUT = 30000;

   If you see "SIM not present or wrong PIN" check the antenna connection.
   ======================================================================================================================
*/

#define METHOD_GET  0
#define METHOD_POST 1

// Extern variables

#if defined(BOARD_HAS_NB)
// NBConnectionHandler from Arduino_NBConnectionHandler.cpp
extern NBConnectionHandler *conMan;
//extern NBConnectionHandler conMan;

// NBClient for NB-IoT network communication
extern NBClient client;

// NBScanner for network scanning (commented out in your code)
extern NBScanner scanner;

// NBModem for low-level modem control
extern NBModem modem;

// NB instance for NB-IoT access
extern NB nb_gsm;

// NBAccess for network access management
extern NB nbAccess;

#elif defined(BOARD_HAS_GSM)
// GSMConnectionHandler from Arduino_GSMConnectionHandler.cpp  
extern GSMConnectionHandler *conMan;
//extern GSMConnectionHandler conMan;

// GSMClient for GSM network communication
extern GSMClient client;

// GSMScanner for GSM network scanning
extern GSMScanner scanner;

// GSMModem for low-level GSM modem control
extern GSMModem modem;

// GSM instance for GSM access
extern GSM nb_gsm;
#endif

extern char ModemFirmwareVersion[32];
extern NetworkConnectionState ConnectionState;
extern unsigned int NoNetworkLoopCycleCount;
extern bool NetworkHasBeenOperational;

// Function prototypes
unsigned long GetCellEpochTime();
int GetCellSignalStrength();
int PrintCurrentCarrier();
void PrintCellSignalStrength();
void PrintModemIMEI();
void PrintModemFW();
bool WaitForNetworkConnection(int seconds);
void onNetworkConnect();
void onNetworkDisconnect();
void onNetworkError();
void CM_initialize();
bool Send_http(char *msg, char *webserver, int webserver_port, char *webserver_path, int webserver_method, char *webserver_xapikey);
