/*
 * ======================================================================================================================
 *  mkrboard.h - MKR Related Board Functions and Definations
 * ======================================================================================================================
 */
#include <Arduino.h>

#define LED_PIN                   LED_BUILTIN
#define ERROR_LED_PIN             LED_BUILTIN
#define ERROR_LED_LIGHTUP_STATE   HIGH // the state that makes the led light up on your board, either low or high

#define BATTERY_STATE_UNKNOWN 0
#define BATTERY_STATE_NOT_CHARGING 1
#define BATTERY_STATE_CHARGING 2
#define BATTERY_STATE_CHARGED 3
#define BATTERY_STATE_DISCHARGING 4
#define BATTERY_STATE_FAULT 5
#define BATTERY_STATE_DISCONNECTED 6

// Extern variables
extern const char *batterystate[];
extern char DeviceID[25];
extern char CryptoID[19];
extern const char* pinNames[];
extern bool ECCX08_Exists;
extern bool PMIC_exists;

// Function prototypes
void GetDeviceID();
void ECCX08_initialize();
byte get_batterystate();
bool pmic_initialize();