/*
 * ======================================================================================================================
 * support.cpp - Support Functions
 * ======================================================================================================================
 */
#include <arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>

#include "include/mkrboard.h"
#include "include/output.h"
#include "include/main.h"
#include "include/support.h"

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * I2C_Device_Exist - does i2c device exist at address
 * 
 *  The i2c_scanner uses the return value of the Write.endTransmisstion to see 
 *  if a device did acknowledge to the address.
 *=======================================================================================================================
 */
bool I2C_Device_Exist(byte address) {
  byte error;

  Wire.begin();                     // Connect to I2C as Master (no addess is passed to signal being a slave)

  Wire.beginTransmission(address);  // Begin a transmission to the I2C slave device with the given address. 
                                    // Subsequently, queue bytes for transmission with the write() function 
                                    // and transmit them by calling endTransmission(). 

  error = Wire.endTransmission();   // Ends a transmission to a slave device that was begun by beginTransmission() 
                                    // and transmits the bytes that were queued by write()
                                    // By default, endTransmission() sends a stop message after transmission, 
                                    // releasing the I2C bus.

  // endTransmission() returns a byte, which indicates the status of the transmission
  //  0:success
  //  1:data too long to fit in transmit buffer
  //  2:received NACK on transmit of address
  //  3:received NACK on transmit of data
  //  4:other error 

  // Partice Library Return values
  // SEE https://docs.particle.io/cards/firmware/wire-i2c/endtransmission/
  // 0: success
  // 1: busy timeout upon entering endTransmission()
  // 2: START bit generation timeout
  // 3: end of address transmission timeout
  // 4: data byte transfer timeout
  // 5: data byte transfer succeeded, busy timeout immediately after
  // 6: timeout waiting for peripheral to clear stop bit

  if (error == 0) {
    return (true);
  }
  else {
    // sprintf (msgbuf, "I2CERR: %d", error);
    // Output (msgbuf);
    return (false);
  }
}

/*
 * ======================================================================================================================
 * Blink() - Count, delay between, delay at end
 * ======================================================================================================================
 */
void Blink(int count, int between)
{
  int c;

  for (c=0; c<count; c++) {
    digitalWrite(LED_PIN, HIGH);
    delay(between);
    digitalWrite(LED_PIN, LOW);
    delay(between);
  }
}

/*
 * ======================================================================================================================
 * FadeOn() - https://www.dfrobot.com/blog-596.html
 * ======================================================================================================================
 */
void FadeOn(unsigned int time,int increament){
  for (byte value = 0 ; value < 255; value+=increament){
  analogWrite(LED_PIN, value);
  delay(time/(255/5));
  }
}

/*
 * ======================================================================================================================
 * FadeOff() - 
 * ======================================================================================================================
 */
void FadeOff(unsigned int time,int decreament){
  for (byte value = 255; value >0; value-=decreament){
  analogWrite(LED_PIN, value);
  delay(time/(255/5));
  }
}

/*
 * ======================================================================================================================
 * myswap()
 * ======================================================================================================================
 */
void myswap(unsigned int *p, unsigned int *q) {
  int t;
   
  t=*p; 
  *p=*q; 
  *q=t;
}

/*
 * ======================================================================================================================
 * mysort()
 * ======================================================================================================================
 */
void mysort(unsigned int a[], unsigned int n) { 
  unsigned int i, j;

  for(i = 0;i < n-1;i++) {
    for(j = 0;j < n-i-1;j++) {
      if(a[j] > a[j+1])
        myswap(&a[j],&a[j+1]);
    }
  }
}

/*
 * =======================================================================================================================
 * isnumeric() - check if string contains all digits
 * =======================================================================================================================
 */
bool isnumeric(char *s) {
  for (int i=0; i< strlen(s); i++) {
    if (!isdigit(*(s+i)) ) {
      return(false);
    }
  }
  return(true);
}

/*
 * =======================================================================================================================
 * isValidNumberString - Helper function to validate numeric string (optional leading + or - allowed)
 * =======================================================================================================================
 */
bool isValidNumberString(const char *str) {
    if (!str || !*str) return false; // empty string invalid
    int i = 0;
    // Check optional sign for first char
    if (str[0] == '+' || str[0] == '-') i = 1;
    for (; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

/*
 * =======================================================================================================================
 * isValidHexString() - validates whether a given string is a valid hexadecimal string of a specified length
 * =======================================================================================================================
 */
bool isValidHexString(const char *hexString, size_t expectedLength) {
    size_t length = strlen(hexString);

    // Check if the length is as expected
    if (length != expectedLength) {
        sprintf(msgbuf, "Hexlen not %d", expectedLength);
        Output (msgbuf);
        return (false);
    }

    // Check if each character is a valid hexadecimal digit
    for (size_t i = 0; i < length; i++) {
        if (!isxdigit(hexString[i])) {
            sprintf(msgbuf, "!Hex char '%c' at pos %d", hexString[i], i);
            Output (msgbuf);
            return (false);
        }
    }

    return (true); // Valid hex string
}

/*
 * =======================================================================================================================
 * hexStringToUint32() - 
 * =======================================================================================================================
 */
bool hexStringToUint32(const char *hexString, uint32_t *result) {
    // Check if the hex string is of length 8
    if (strlen(hexString) != 8) {
        Output ("Error: Hex string must be of length 8");
        return false;
    }

    *result = 0;
    for (int i = 0; i < 8; i++) {
        char c = hexString[i];
        uint32_t digit;
    
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            digit = 10 + (c - 'A');
        } else {
            return false;
        }

        *result = (*result << 4) | digit;
    }
    return true; // Successful conversion
}

/*
 * =======================================================================================================================
 * hexStringToByteArray() -
 * =======================================================================================================================
 */
void hexStringToByteArray(const char *hexString, uint8_t *byteArray, int len) {
    for (int i = 0; i < len; i += 2) {
        char hexPair[3];
        hexPair[0] = hexString[i];
        hexPair[1] = hexString[i + 1];
        hexPair[2] = '\0';

        // Convert hex pair to uint8_t
        uint8_t byteValue = 0;
        for (int j = 0; j < 2; ++j) {
            byteValue <<= 4;

            if (hexPair[j] >= '0' && hexPair[j] <= '9') {
                byteValue |= hexPair[j] - '0';
            } else if (hexPair[j] >= 'A' && hexPair[j] <= 'F') {
                byteValue |= hexPair[j] - 'A' + 10;
            } else if (hexPair[j] >= 'a' && hexPair[j] <= 'f') {
                byteValue |= hexPair[j] - 'a' + 10;
            }
        }

        byteArray[i / 2] = byteValue;
    }
}

/*
 * ======================================================================================================================
 * stringToLongLong()
 * ======================================================================================================================
 */
long long int stringToLongLong(const char* str) {
  char buffer[20]; // Adjust the size according to your maximum expected input length
  strcpy(buffer, str);
  return strtoll(buffer, NULL, 10);
}


/*
 * ======================================================================================================================
 * safe_strcat() - imple safe strcat that limits copy to avoid buffer overflow
 * ======================================================================================================================
 */
void safe_strcat(char *dest, size_t dest_size, const char *src) {
  strncat(dest, src, dest_size - strlen(dest) - 1);
}

/*
 * ======================================================================================================================
 * url_encode()
 * ======================================================================================================================
 */
void url_encode(const char *src, char *dest, int dest_len) {
  static const char hex[] = "0123456789ABCDEF";
  int di = 0;
  for (int si = 0; src[si] != '\0' && di < dest_len - 1; si++) {
    char c = src[si];
    if (('a' <= c && c <= 'z') ||
        ('A' <= c && c <= 'Z') ||
        ('0' <= c && c <= '9') ||
        (c == '-' || c == '_' || c == '.' || c == '~')) {
      dest[di++] = c;
    } 
    else {
      if (di + 3 >= dest_len - 1) break; // avoid buffer overflow
      dest[di++] = '%';
      dest[di++] = hex[(c >> 4) & 0xF];
      dest[di++] = hex[c & 0xF];
    }
  }
  dest[di] = '\0';
}

/*
 * ======================================================================================================================
 * json_to_get_string() - Assumes obs contains a JSON object string, and cf_urlpath is a C string
 * ======================================================================================================================
 */
bool json_to_get_string_inplace(const char *cf_urlpath, char *obs) {
  const int JSON_BUFFER_SIZE = MAX_HTTP_SIZE;
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;

  if (deserializeJson(doc, obs) != DeserializationError::Ok) {
    return false; // JSON parse error
  }

  char temp[JSON_BUFFER_SIZE];
  temp[0] = '\0';

  // Start with the path
  safe_strcat(temp, sizeof(temp), cf_urlpath);
  safe_strcat(temp, sizeof(temp), "?");

  bool first = true;
  for (JsonPair kv : doc.as<JsonObject>()) {
    if (!first) {
      safe_strcat(temp, sizeof(temp), "&");
    }
    first = false;

    // Add key safely
    safe_strcat(temp, sizeof(temp), kv.key().c_str());
    safe_strcat(temp, sizeof(temp), "=");

    char valbuf[128];
    const char *val = kv.value().as<const char*>();
    if (val) {
      url_encode(val, valbuf, sizeof(valbuf));
      safe_strcat(temp, sizeof(temp), valbuf);
    } else {
      char tmp[32];
      if (kv.value().is<int>()) {
        snprintf(tmp, sizeof(tmp), "%d", kv.value().as<int>());
        safe_strcat(temp, sizeof(temp), tmp);
      } else if (kv.value().is<float>()) {
        snprintf(tmp, sizeof(tmp), "%.4f", kv.value().as<float>());
        safe_strcat(temp, sizeof(temp), tmp);
      }
    }

    if (strlen(temp) >= sizeof(temp) - 1) {
      break;
    }
  }

  // Copy back safely
  strncpy(obs, temp, JSON_BUFFER_SIZE - 1);
  obs[JSON_BUFFER_SIZE - 1] = '\0';

  return true;
}

