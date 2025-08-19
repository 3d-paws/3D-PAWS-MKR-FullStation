/*
 * ======================================================================================================================
 *  SF.h - Support Functions - Misc Support Functions Functions
 * ======================================================================================================================
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
 * json_to_get_string() - Assumes obs contains a JSON object string, and cf_webserver_path is a C string
 * ======================================================================================================================
 */
bool json_to_get_string_inplace(const char *cf_webserver_path, char *obs) {
  const int JSON_BUFFER_SIZE = MAX_HTTP_SIZE;
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;

  if (deserializeJson(doc, obs) != DeserializationError::Ok) {
    return false; // JSON parse error
  }

  char temp[JSON_BUFFER_SIZE];
  temp[0] = '\0';

  // Start with the path
  safe_strcat(temp, sizeof(temp), cf_webserver_path);
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
        snprintf(tmp, sizeof(tmp), "%f", kv.value().as<float>());
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

/*
 * ======================================================================================================================
 * JPO_ClearBits() - Clear System Status Bits related to initialization
 * ======================================================================================================================
 */
void JPO_ClearBits() {
  if (JustPoweredOn) {
    JustPoweredOn = false;
    SystemStatusBits &= ~SSB_PWRON;   // Turn Off Power On Bit
    // SystemStatusBits &= ~SB_SD;       // Turn Off SD Missing Bit - Required keep On
    // SystemStatusBits &= ~SSB_RTC;     // Turn Off RTC Missing Bit - Required keep On
    SystemStatusBits &= ~SSB_OLED;    // Turn Off OLED Missing Bit
    SystemStatusBits &= ~SSB_LORA;    // Turn Off LoRa Missing Bit
    SystemStatusBits &= ~SSB_BMX_1;   // Turn Off BMX_1 Not Found Bit
    SystemStatusBits &= ~SSB_BMX_2;   // Turn Off BMX_2 Not Found Bit
    SystemStatusBits &= ~SSB_HTU21DF; // Turn Off HTU Not Found Bit
    SystemStatusBits &= ~SSB_MCP_1;   // Turn Off MCP_1 Not Found Bit
    SystemStatusBits &= ~SSB_MCP_2;   // Turn Off MCP_2 Not Found Bit
    SystemStatusBits &= ~SSB_MCP_3;   // Turn Off MCP_2 Not Found Bit
    SystemStatusBits &= ~SSB_SHT_1;   // Turn Off SHT_1 Not Found Bit
    SystemStatusBits &= ~SSB_SHT_2;   // Turn Off SHT_1 Not Found Bit
    SystemStatusBits &= ~SSB_HIH8;    // Turn Off HIH Not Found Bit
    SystemStatusBits &= ~SSB_VLX;     // Turn Off VEML7700 Not Found Bit
    SystemStatusBits &= ~SSB_PMIC;    // Turn Off Power Management IC Not Found Bit
    SystemStatusBits &= ~SSB_PM25AQI; // Turn Off PM25AQI Not Found Bit
    SystemStatusBits &= ~SSB_HDC_1;   // Turn Off HDC302x Not Found Bit
    SystemStatusBits &= ~SSB_HDC_2;   // Turn Off HDC302x Not Found Bit
    SystemStatusBits &= ~SSB_BLX;     // Turn Off BLUX30 Not Found Bit
    SystemStatusBits &= ~SSB_LPS_1;   // Turn Off LPS35HW Not Found Bit
    SystemStatusBits &= ~SSB_LPS_2;   // Turn Off LPS35HW Not Found Bit
    SystemStatusBits &= ~SSB_TLW;     // Turn Off Tinovi Leaf Wetness Not Found Bit
    SystemStatusBits &= ~SSB_TSM;     // Turn Off Tinovi Soil Moisture Not Found Bit
    SystemStatusBits &= ~SSB_TMSM;    // Turn Off Tinovi MultiLevel Soil Moisture Not Found Bit
    // SystemStatusBits &= ~SSB_EEPROM;  // Turn Off 24LC32 EEPROM Not Found Bit - Required keep On
  }
}
