/*
 * ======================================================================================================================
 * SDC.h - SD Card Stuff
 * 
 * SEE https://store-usa.arduino.cc/products/mkr-sd-proto-shield
 *  
 * MKR SD Board Pins
 *    CS  4
 *    CLK 9  /SCK
 *    DI  8  MOSI
 *    DO  10 MISO
 * ======================================================================================================================
 */
#define SD_ChipSelect     4  // GPIO 10 is Pin 10 on Feather and D5 on Particle Boron Board
#define CF_NAME           "CONFIG.TXT"
#define KEY_MAX_LENGTH    30 // Config File Key Length
#define VALUE_MAX_LENGTH  30 // Config File Value Length
#define LINE_MAX_LENGTH   VALUE_MAX_LENGTH+KEY_MAX_LENGTH+3   // =, CR, LF 

SdFat SD;                                   // File system object.
File SD_fp;
char SD_obsdir[] = "/OBS";                  // Observations stored in this directory. Created at power on if not exist
bool SD_exists = false;                     // Set to true if SD card found at boot
char SD_n2s_file[] = "N2SOBS.TXT";          // Need To Send Observation file
uint32_t SD_n2s_max_filesz = 512 * 60 * 24; // Keep a little over 1 day. When it fills, it is deleted and we start over.
uint32_t SD_N2S_POSITION = 0;

char SD_sim_file[] = "SIM.TXT";         // File used to set Ineternal or External sim configuration
char SD_simold_file[] = "SIMOLD.TXT";   // SIM.TXT renamed to this after sim configuration set
char SD_crt_file[] = "CRT.TXT";         // if file exists clear rain totals and delete file

/* 
 *=======================================================================================================================
 * SD_initialize()
 *=======================================================================================================================
 */
void SD_initialize() {

  if (!SD.begin(SD_ChipSelect)) {
    Output ("SD:NF");
    SystemStatusBits |= SSB_SD;
    delay (5000);
  }
  else {
    if (!SD.exists(SD_obsdir)) {
      if (SD.mkdir(SD_obsdir)) {
        Output (F("SD:MKDIR OBS OK"));
        Output (F("SD:Online"));
        SD_exists = true;
      }
      else {
        Output (F("SD:MKDIR OBS ERR"));
        Output (F("SD:Offline"));
        SystemStatusBits |= SSB_SD;  // Turn On Bit     
      } 
    }
    else {
      Output (F("SD:Online"));
      Output (F("SD:OBS DIR Exists"));
      SD_exists = true;
    }
  }
}

/* 
 *=======================================================================================================================
 * SD_LogObservation()
 *=======================================================================================================================
 */
void SD_LogObservation(char *observations) {
  char SD_logfile[24];
  File fp;

  time_t ts = stc.getEpoch();
  tm *dt = gmtime(&ts); 
    
  if (!SD_exists) {
    Output (F("SD:NOT EXIST"));
    return;
  }

  if (!STC_valid) {
    Output (F("SD:STC NOT VALID"));
    return;
  }
  
  sprintf (SD_logfile, "%s/%4d%02d%02d.log", SD_obsdir, dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday);
  Output (SD_logfile);
   
  fp = SD.open(SD_logfile, FILE_WRITE); 
  if (fp) {
    fp.println(observations);
    fp.close();
    SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
    Output (F("OBS Logged to SD"));
  }
  else {
    SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
    Output (F("SD:Open(Log)ERR"));
    // At thins point we could set SD_exists to false and/or set a status bit to report it
    // sd_initialize();  // Reports SD NOT Found. Library bug with SD
  }
}

/* 
 *=======================================================================================================================
 * SD_N2S_Delete()
 *=======================================================================================================================
 */
bool SD_N2S_Delete() {
  bool result;

  if (SD_exists && SD.exists(SD_n2s_file)) {
    if (SD.remove (SD_n2s_file)) {
      SystemStatusBits &= ~SSB_N2S; // Turn Off Bit
      Output (F("N2S->DEL:OK"));
      result = true;
    }
    else {
      Output (F("N2S->DEL:ERR"));
      SystemStatusBits |= SSB_SD; // Turn On Bit
      result = false;
    }
  }
  else {
    SystemStatusBits &= ~SSB_N2S; // Turn Off Bit
    Output (F("N2S->DEL:NF"));
    result = true;
  }
  eeprom.n2sfp = 0;
  EEPROM_Update();
  return (result);
}

/* 
 *=======================================================================================================================
 * SD_NeedToSend_Add()
 *=======================================================================================================================
 */
void SD_NeedToSend_Add(char *observation) {
  File fp;

  if (!SD_exists) {
    return;
  }
  
  fp = SD.open(SD_n2s_file, FILE_WRITE); // Open the file for reading and writing, starting at the end of the file.
                                         // It will be created if it doesn't already exist.
  if (fp) {  
    if (fp.size() > SD_n2s_max_filesz) {
      fp.close();
      Output (F("N2S:Full"));
      if (SD_N2S_Delete()) {
        // Only call ourself again if we truely deleted the file. Otherwise infinate loop.
        SD_NeedToSend_Add(observation); // Now go and log the data
      }
    }
    else {
      fp.println(observation); //Print data, followed by a carriage return and newline, to the File
      fp.close();
      SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
      SystemStatusBits |= SSB_N2S; // Turn on Bit that says there are entries in the N2S File
      Output (F("N2S:OBS Added"));
    }
  }
  else {
    SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
    Output (F("N2S:Open Error"));
    // At thins point we could set SD_exists to false and/or set a status bit to report it
    // sd_initialize();  // Reports SD NOT Found. Library bug with SD
  }
}

/* 
 *=======================================================================================================================
 * SD_ClearRainTotals() -- If CRT.TXT exists on SD card clear rain totals - Checked at Boot
 *=======================================================================================================================
 */
void SD_ClearRainTotals() {
  if (STC_valid && SD_exists) {
    if (SD.exists(SD_crt_file)) {
      if (SD.remove (SD_crt_file)) {
        Output (F("CRT:OK-CLR"));
        uint32_t current_time = stc.getEpoch();
        EEPROM_ClearRainTotals(current_time);
      }
      else {
        Output (F("CRT:ERR-RM"));
      }
    }
    else {
      Output (F("CRT:OK-NF"));
    }
  }
  else {
    Output (F("CRT:ERR-CLK"));
  }
}


/* 
 *=======================================================================================================================
 * Support functions for Config file
 * 
 *  https://arduinogetstarted.com/tutorials/arduino-read-config-from-sd-card
 *  
 *  myInt_1    = SD_findInt(F("myInt_1"));
 *  myFloat_1  = SD_findFloat(F("myFloat_1"));
 *  myString_1 = SD_findString(F("myString_1"));
 *  
 *  CONFIG.TXT content example
 *  myString_1=Hello
 *  myInt_1=2
 *  myFloat_1=0.74
 *=======================================================================================================================
 */

int SD_findKey(const __FlashStringHelper * key, char * value) {
  File configFile = SD.open(CF_NAME);

  if (!configFile) {
    Serial.print(F("SD Card: error on opening file "));
    Serial.println(CF_NAME);
    return(0);
  }

  char key_string[KEY_MAX_LENGTH];
  char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1]; // 1 is = character
  int key_length = 0;
  int value_length = 0;

  // Flash string to string
  PGM_P keyPoiter;
  keyPoiter = reinterpret_cast<PGM_P>(key);
  byte ch;
  do {
    ch = pgm_read_byte(keyPoiter++);
    if (ch != 0)
      key_string[key_length++] = ch;
  } while (ch != 0);

  // check line by line
  while (configFile.available()) {
    int buffer_length = configFile.readBytesUntil('\n', SD_buffer, LINE_MAX_LENGTH);
    if (SD_buffer[buffer_length - 1] == '\r')
      buffer_length--; // trim the \r

    if (buffer_length > (key_length + 1)) { // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) { // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
  }

  configFile.close();  // close the file
  return value_length;
}

int HELPER_ascii2Int(char *ascii, int length) {
  int sign = 1;
  int number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

long HELPER_ascii2Long(char *ascii, int length) {
  int sign = 1;
  long number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

float HELPER_ascii2Float(char *ascii, int length) {
  int sign = 1;
  int decimalPlace = 0;
  float number  = 0;
  float decimal = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c == '.')
        decimalPlace = 1;
      else if (c >= '0' && c <= '9') {
        if (!decimalPlace)
          number = number * 10 + (c - '0');
        else {
          decimal += ((float)(c - '0') / pow(10.0, decimalPlace));
          decimalPlace++;
        }
      }
    }
  }

  return (number + decimal) * sign;
}

String HELPER_ascii2String(char *ascii, int length) {
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str += String(c);
  }
  return str;
}

char* HELPER_ascii2CharStr(char *ascii, int length) {
  char *str;
  int i = 0;
  str = (char *) malloc (length+1);
  str[0] = 0;
  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str[i] = c;
    str[i+1] = 0;
  }
  return str;
}

bool SD_available(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return value_length > 0;
}

int SD_findInt(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Int(value_string, value_length);
}

float SD_findFloat(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Float(value_string, value_length);
}

String SD_findString(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2String(value_string, value_length);
}

char* SD_findCharStr(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2CharStr(value_string, value_length);
}

long SD_findLong(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Long(value_string, value_length);
}

/* 
 *=======================================================================================================================
 * SD_ReadConfigFile()
 *=======================================================================================================================
 */
void SD_ReadConfigFile() {
  cf_webserver      = SD_findCharStr(F("webserver"));
  sprintf(msgbuf, "CF:%s=[%s]", F("webserver"), cf_webserver); Output (msgbuf);
  
  cf_webserver_port = SD_findInt(F("webserver_port"));
  if (cf_webserver_port == 0) {
    cf_webserver_port = 80;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("webserver_port"), cf_webserver_port); Output (msgbuf);

  cf_webserver_path = SD_findCharStr(F("webserver_path"));
  sprintf(msgbuf, "CF:%s=[%s]", F("webserver_path"), cf_webserver_path); Output (msgbuf);
  
  cf_apikey         = SD_findCharStr(F("apikey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("apikey"), cf_apikey); Output (msgbuf);
  
  cf_instrument_id  = SD_findInt(F("instrument_id"));
  sprintf(msgbuf, "CF:%s=[%d]", F("instrument_id"), cf_instrument_id); Output (msgbuf);

  cf_webserver_method = SD_findInt(F("webserver_method"));
  if ((cf_webserver_method < 0) || (cf_webserver_method > 1)) {
    cf_webserver_method = 0;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("webserver_method"), cf_webserver_method); Output (msgbuf);
  
  cf_sim_apn        = SD_findCharStr(F("sim_apn"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_apn"), cf_sim_apn); Output (msgbuf);
  
  cf_sim_pin   = SD_findCharStr(F("sim_pin"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_pin"), cf_sim_pin); Output (msgbuf);
  
  cf_sim_username   = SD_findCharStr(F("sim_username"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_username"), cf_sim_username); Output (msgbuf);
  
  cf_sim_password   = SD_findCharStr(F("sim_password"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_password"), cf_sim_password); Output (msgbuf);
  
  cf_reboot_countdown_timer = SD_findInt(F("reboot_countdown_timer"));
  if (cf_reboot_countdown_timer == 0) {
    cf_reboot_countdown_timer = 79200;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("reboot_countdown_timer"), cf_reboot_countdown_timer); Output (msgbuf);
  
  cf_no_network_reset_count = SD_findInt(F("no_network_reset_count"));
  if (cf_no_network_reset_count == 0) {
    cf_no_network_reset_count = 60;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("no_network_reset_count"), cf_no_network_reset_count); Output (msgbuf);
  
  cf_lora_unitid     = SD_findInt(F("lora_unitid"));
  if (cf_lora_unitid == 0) {
    cf_lora_unitid = 1;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("lora_unitid"), cf_lora_unitid); Output (msgbuf);
  
  cf_lora_txpwr     = SD_findInt(F("lora_txpwr"));
  if (cf_lora_txpwr == 0) {
    cf_lora_txpwr = 5;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("lora_txpwr"), cf_lora_txpwr); Output (msgbuf);
  
  cf_lora_freq      = SD_findInt(F("lora_freq"));
  if (cf_lora_freq == 0) {
    cf_lora_freq = 915;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("lora_freq"), cf_lora_freq); Output (msgbuf);
  
  cf_aes_pkey   = SD_findCharStr(F("aes_pkey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("aes_pkey"), cf_aes_pkey); Output (msgbuf);
  
  cf_aes_myiv    = SD_findLong(F("aes_myiv"));
  sprintf(msgbuf, "CF:%s=[%lu]", F("aes_myiv"), cf_aes_myiv); Output (msgbuf);

  // Rain
  cf_rg1_enable   = SD_findInt(F("rg1_enable"));
  if (cf_rg1_enable == 0) {
    cf_rg1_enable = 1;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("rg1_enable"), cf_rg1_enable); Output (msgbuf);

  cf_rg2_enable   = SD_findInt(F("rg2_enable"));
  sprintf(msgbuf, "CF:%s=[%d]", F("rg2_enable"), cf_rg2_enable); Output (msgbuf);

  // Distance
  cf_ds_enable    = SD_findInt(F("ds_enable"));
  sprintf(msgbuf, "CF:%s=[%d]", F("ds_enable"), cf_ds_enable); Output (msgbuf);

  cf_ds_baseline = SD_findInt(F("ds_baseline"));
  sprintf(msgbuf, "CF:%s=[%d]", F("ds_baseline"), cf_ds_baseline); Output (msgbuf);

  cf_obs_period   = SD_findInt(F("obs_period"));
  if (cf_obs_period == 0) {
    cf_obs_period = 1;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("obs_period"), cf_obs_period); Output (msgbuf);
}
