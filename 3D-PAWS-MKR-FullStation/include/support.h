/*
 * ======================================================================================================================
 *  support.h - Support Functions Definations
 * ======================================================================================================================
 */
#include <Arduino.h>

// Extern variables

// Function prototypes
bool I2C_Device_Exist(byte address);
void Blink(int count, int between);
void FadeOn(unsigned int time,int increament);
void FadeOff(unsigned int time,int decreament);
void mysort(unsigned int a[], unsigned int n);
bool isValidNumberString(const char *str);
bool isValidHexString(const char *hexString, size_t expectedLength);
bool hexStringToUint32(const char *hexString, uint32_t *result);
void hexStringToByteArray(const char *hexString, uint8_t *byteArray, int len);

long long int stringToLongLong(const char* str);
void safe_strcat(char *dest, size_t dest_size, const char *src);
void url_encode(const char *src, char *dest, int dest_len);
bool json_to_get_string_inplace(const char *cf_urlpath, char *obs);
