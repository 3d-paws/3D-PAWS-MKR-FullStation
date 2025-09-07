/*
 * ======================================================================================================================
 *  OBS.h - Observations
 *  
 *  SEE https://arduinojson.org/v6/how-to/use-arduinojson-with-httpclient/
 * ======================================================================================================================
 */

 // Prototype thess functions to aviod compile function unknown issue.
void BackGroundWork();   

#define OBSERVATION_INTERVAL      60   // Seconds

#define MAX_SENSORS         96

typedef enum {
  F_OBS, 
  I_OBS, 
  U_OBS
} OBS_TYPE;

typedef struct {
  char          id[6];       // Suport 4 character length observation names
  int           type;
  float         f_obs;
  int           i_obs;
  unsigned long u_obs;
  bool          inuse;
} SENSOR;

typedef struct {
  bool            inuse;                // Set to true when an observation is stored here         
  time_t          ts;                   // TimeStamp
  int             css;                  // Cell Signal Strength
  unsigned long   hth;                  // System Status Bits
  SENSOR          sensor[MAX_SENSORS];
} OBSERVATION_STR;

OBSERVATION_STR obs;

void OBS_N2S_Publish();   // Prototype this function to aviod compile function unknown issue.


/*
 * ======================================================================================================================
 * OBS_Send() - Do a GET/POST request to log observation, process returned text for result code and set return status
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
bool OBS_Send(char *obs)
{
  char response[64];
  char buf[96];
  int r, i=0, exit_timer=0;
  bool posted = false;
  
  unsigned long nt = GetCellEpochTime(); // Getting the cell network time does a bunch of checks on if the Network is up.
                                         // So a return of 0 says there are problems, and we should not try to transmit.
  if (!nt) {
    sprintf (Buffer32Bytes,"OBS:SEND->NWTIME BAD");
    Output(Buffer32Bytes);  
    NoNetworkLoopCycleCount++;   // reset modem and reboot if count gets to our set max - done in main loop()
  }
  else {
    Output(F("OBS:SEND->HTTP"));
    if (!client.connect(cf_webserver, cf_webserver_port)) {
      NoNetworkLoopCycleCount++;   // reset modem and reboot if count gets to our set max - done in main loop()
      Output(F("OBS:HTTP FAILED"));
    }
    else {
      NoNetworkLoopCycleCount = 0;  // reset the counter to prevent rebooting 
      Output(F("OBS:HTTP CONNECTED"));

      // 0=GET, 1=POST
      if (cf_webserver_method == 0) { 
        json_to_get_string_inplace(cf_webserver_path, obs);
        Serial_writeln (obs);

        // Make a HTTP GET request:
        client.print("GET ");
        client.print(obs); // path
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(cf_webserver);
        client.println("Connection: close");
        client.println();
      }
      else { 
        // Construct HTTP POST request    
        client.println(String("POST ") + cf_webserver_path + " HTTP/1.1");
        client.println(String("Host: ") + cf_webserver);
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(strlen(obs));
        client.println("Connection: close");
        client.println();
        client.println(obs);
      }

      Output(F("OBS:HTTP SENT"));

      // Check every 500ms for data, up to 2 minutes. While waiting take Wind Readings every 1s
      i=0;
      exit_timer = 0;
      Wind_TakeReading();
      
      while(client.connected() && !client.available()) {     
        delay(500);  // Was delay(100);
        i++;
        if (i>=2) { // After a second take a wind observation - Was (i>=10)
          Wind_TakeReading();
          i=0;

          // Prevent infinate waiting - not sure if this is needed but can't hurt
          if (++exit_timer >= 60) { // after 2 minutes lets call it quits
            break;
          }
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

 /*
 * ======================================================================================================================
 * OBS_Clear() - Set OBS to not in use
 * ======================================================================================================================
 */
void OBS_Clear() {
  obs.inuse =false;
  for (int s=0; s<MAX_SENSORS; s++) {
    obs.sensor[s].inuse = false;
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Add() - Save OBS to N2S file
 * ======================================================================================================================
 */
void OBS_N2S_Add() {
  if (obs.inuse) {     // Sanity check
    char ts[32];
   
    memset(obsbuf, 0, sizeof(obsbuf));

    sprintf (obsbuf, "{");
    sprintf (obsbuf+strlen(obsbuf), "\"key\":\"%s\"", cf_apikey); 
    sprintf (obsbuf+strlen(obsbuf), ",\"instrument_id\":%d", cf_instrument_id);

    tm *dt = gmtime(&obs.ts);

    sprintf (obsbuf+strlen(obsbuf), ",\"at\":\"%d-%02d-%02dT%02d%:%02d%:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
      
    sprintf (obsbuf+strlen(obsbuf), ",\"css\":%d", obs.css);
    obs.hth |= SSB_FROM_N2S; // Turn On Bit - Modify System Status and Set From Need to Send file bit
    sprintf (obsbuf+strlen(obsbuf), ",\"hth\":%d", obs.hth);

    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output (F("WhyAmIHere?"));
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");

    Serial_writeln (obsbuf);
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output(F("OBS-> N2S"));
  }
  else {
    Output(F("OBS->N2S OBS:Empty"));
  }
}

/*
 * ======================================================================================================================
 * OBS_Build_JSON() - Create observation in obsbuf for sending to Chords 
 * 
 * Example {"key":1234,"instrument_id":53,"at":"2022-05-17T17:40:04","hth":8770}
 * ======================================================================================================================
 */
bool OBS_Build_JSON() {   
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));

    // Save the Observation in JSON format
    
    sprintf (obsbuf, "{");
    sprintf (obsbuf+strlen(obsbuf), "\"key\":\"%s\"", cf_apikey); 
    if (DevID_Exists) {
      sprintf (obsbuf+strlen(obsbuf),",\"devid\":\"%02X%02X%02X%02X%02X%02X%02X%02X%02X\"", 
        DevID[0],DevID[1],DevID[2],DevID[3],DevID[4],DevID[5],DevID[6],DevID[7],DevID[8]);
    }
    else {
      sprintf (obsbuf+strlen(obsbuf),",\"devid\":0");
    }
    sprintf (obsbuf+strlen(obsbuf), ",\"instrument_id\":%d", cf_instrument_id);

    tm *dt = gmtime(&obs.ts); 
    
    sprintf (obsbuf+strlen(obsbuf), ",\"at\":\"%d-%02d-%02dT%02d%:%02d%:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);

    sprintf (obsbuf+strlen(obsbuf), ",\"css\":%d", obs.css);
    sprintf (obsbuf+strlen(obsbuf), ",\"hth\":%d", obs.hth);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output (F("WhyAmIHere?"));
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");

    Output(F("OBS->URL"));
    Serial_writeln (obsbuf);
    return (true);
  }
  else {
    Output(F("OBS->JSON OBS:Empty"));
    return (false);
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Save() - Save Observations to Need2Send File
 * ======================================================================================================================
 */
void OBS_N2S_Save() {

  // Save Station Observations to N2S file
  OBS_N2S_Add();
  OBS_Clear();

  // Save Rain and Soil LoRa Observations to N2S file
  while (lora_relay_need2log()) {
    OBS_Relay_Build_JSON(); // Copy JSON observation to obsbuf, remove from relay structure
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output(F("LR->N2S"));
    Serial_write (obsbuf); 
  }
}

/*
 * ======================================================================================================================
 * OBS_Take() - Take Observations - Should be called once a minute - fill data structure
 * ======================================================================================================================
 */
void OBS_Take() {
  int sidx = 0;;
  float rg1 = 0.0;
  float rg2 = 0.0;
  unsigned long rg1ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  unsigned long rg2ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  float ws = 0.0;
  int wd = 0;
  unsigned long rgds;    // rain gauge delta seconds, seconds since last rain gauge observation logged
  float mcp3_temp = 0.0;  // globe temperature
  float wetbulb_temp = 0.0;
  float sht1_humid = 0.0;
  float sht1_temp = 0.0;
  float heat_index = 0.0;

  // Safty Check for Vaild Time
  if (!STC_valid) {
    Output (F("OBS_Take: Time NV"));
    return;
  }
  
  OBS_Clear(); // Just do it again as a safty check

  Wind_GustUpdate(); // Update Gust and Gust Direction readings

  obs.inuse = true;
  obs.ts = Time_of_obs;     // Set in main loop no need to call stc.getEpoch();
  obs.css = GetCellSignalStrength();
  obs.hth = SystemStatusBits;

  // Rain Gauge 1 - Each tip is 0.2mm of rain
  if (cf_rg1_enable) {
    rg1ds = (millis()-raingauge1_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg1 = raingauge1_interrupt_count * 0.2;
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg1 = (isnan(rg1) || (rg1 < QC_MIN_RG) || (rg1 > ((rg1ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg1;
  }

  // Rain Gauge 2 - Each tip is 0.2mm of rain
  if (cf_rg2_enable) {
    rg2ds = (millis()-raingauge2_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg2 = raingauge2_interrupt_count * 0.2;
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg2 = (isnan(rg2) || (rg2 < QC_MIN_RG) || (rg2 > ((rg2ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg2;
  }

  if (cf_rg1_enable || cf_rg2_enable) {
    EEPROM_UpdateRainTotals(rg1, rg2);
  }
  
  // Rain Gauge 1
  if (cf_rg1_enable) {
    strcpy (obs.sensor[sidx].id, "rg1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = rg1;
    obs.sensor[sidx++].inuse = true;
  
    strcpy (obs.sensor[sidx].id, "rgt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgt1;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "rgp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgp1;
    obs.sensor[sidx++].inuse = true;
  }

  // Rain Gauge 2
  if (cf_rg2_enable) {
    strcpy (obs.sensor[sidx].id, "rg2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = rg2;
    obs.sensor[sidx++].inuse = true;
  
    strcpy (obs.sensor[sidx].id, "rgt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgt2;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "rgp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgp2;
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_ds_enable) {
    float ds_median, ds_median_raw;
    
    ds_median = ds_median_raw = DS_Median();
    if (cf_ds_baseline > 0) {
      ds_median = cf_ds_baseline - ds_median_raw;
    }

    strcpy (obs.sensor[sidx].id, "ds");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ds_median;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "dsr");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ds_median_raw;
    obs.sensor[sidx++].inuse = true;
  }
  
  // 03 Wind Speed (Global)
  ws = Wind_SpeedAverage();
  ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
  strcpy (obs.sensor[sidx].id, "ws");
  obs.sensor[sidx].type = F_OBS;
  obs.sensor[sidx].f_obs = ws;
  obs.sensor[sidx++].inuse = true;

  // 04 Wind Direction
  wd = Wind_DirectionVector();
  wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
  strcpy (obs.sensor[sidx].id, "wd");
  obs.sensor[sidx].type = I_OBS;
  obs.sensor[sidx].i_obs = wd;
  obs.sensor[sidx++].inuse = true;

  // 05 Wind Gust (Global)
  ws = Wind_Gust();
  ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
  strcpy (obs.sensor[sidx].id, "wg");
  obs.sensor[sidx].type = F_OBS;
  obs.sensor[sidx].f_obs = ws;
  obs.sensor[sidx++].inuse = true;

  // 06 Wind Gust Direction (Global)
  wd = Wind_GustDirection();
  wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
  strcpy (obs.sensor[sidx].id, "wgd");
  obs.sensor[sidx].type = I_OBS;
  obs.sensor[sidx].i_obs = wd;
  obs.sensor[sidx++].inuse = true;

  Wind_ClearSampleCount(); // Clear Counter

  //
  // Add I2C Sensors
  //
  if (BMX_1_exists) {
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_1_chip_id == BMP280_CHIP_ID) {
      p = bmp1.readPressure()/100.0F;       // bp1 hPa
      t = bmp1.readTemperature();           // bt1
    }
    else if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_1_type == BMX_TYPE_BME280) {
        p = bme1.readPressure()/100.0F;     // bp1 hPa
        t = bme1.readTemperature();         // bt1
        h = bme1.readHumidity();            // bh1
      }
      if (BMX_1_type == BMX_TYPE_BMP390) {
        p = bm31.readPressure()/100.0F;     // bp1 hPa
        t = bm31.readTemperature();         // bt1       
      }
    }
    else { // BMP388
      p = bm31.readPressure()/100.0F;       // bp1 hPa
      t = bm31.readTemperature();           // bt1
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    
    // 7 BMX1 Preasure
    strcpy (obs.sensor[sidx].id, "bp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // 8 BMX1 Temperature
    strcpy (obs.sensor[sidx].id, "bt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // 9 BMX1 Humidity
    if (BMX_1_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }
  }

  if (BMX_2_exists) {
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_2_chip_id == BMP280_CHIP_ID) {
      p = bmp2.readPressure()/100.0F;       // bp2 hPa
      t = bmp2.readTemperature();           // bt2
    }
    else if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_2_type == BMX_TYPE_BME280) {
        p = bme2.readPressure()/100.0F;     // bp2 hPa
        t = bme2.readTemperature();         // bt2
        h = bme2.readHumidity();            // bh2 
      }
      if (BMX_2_type == BMX_TYPE_BMP390) {
        p = bm32.readPressure()/100.0F;       // bp2 hPa
        t = bm32.readTemperature();           // bt2
      }      
    }
    else { // BMP388
      p = bm32.readPressure()/100.0F;       // bp2 hPa
      t = bm32.readTemperature();           // bt2
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

    // 10 BMX2 Preasure
    strcpy (obs.sensor[sidx].id, "bp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // 11 BMX2 Temperature
    strcpy (obs.sensor[sidx].id, "bt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // 12 BMX2 Humidity
    if (BMX_2_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }
  }

  if (HTU21DF_exists) {
    float t = 0.0;
    float h = 0.0;
    
    // 13 HTU Humidity
    strcpy (obs.sensor[sidx].id, "hh1");
    obs.sensor[sidx].type = F_OBS;
    h = htu.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    // 14 HTU Temperature
    strcpy (obs.sensor[sidx].id, "ht1");
    obs.sensor[sidx].type = F_OBS;
    t = htu.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (SHT_1_exists) {
    float t = 0.0;
    float h = 0.0;

    // 15 SHT1 Temperature
    strcpy (obs.sensor[sidx].id, "st1");
    obs.sensor[sidx].type = F_OBS;
    t = sht1.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // 16 SHT1 Humidity   
    strcpy (obs.sensor[sidx].id, "sh1");
    obs.sensor[sidx].type = F_OBS;
    h = sht1.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    sht1_humid = h; // save for derived observations
  }

  if (SHT_2_exists) {
    float t = 0.0;
    float h = 0.0;

    // 17 SHT2 Temperature
    strcpy (obs.sensor[sidx].id, "st2");
    obs.sensor[sidx].type = F_OBS;
    t = sht2.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // 18 SHT2 Humidity   
    strcpy (obs.sensor[sidx].id, "sh2");
    obs.sensor[sidx].type = F_OBS;
    h = sht2.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }

  if (HDC_1_exists) {
    double t = -999.9;
    double h = -999.9;

    if (hdc1.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
      SystemStatusBits &= ~ SSB_HDC_1;  // Turn Off Bit
    }
    else {
      Output (F("ERR:HDC1 Read"));
      SystemStatusBits |= SSB_HDC_1;  // Turn On Bit
    }

    // 18 HDC1 Temperature
    strcpy (obs.sensor[sidx].id, "hdt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // 19 HDC1 Humidity
    strcpy (obs.sensor[sidx].id, "hdh1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
    obs.sensor[sidx++].inuse = true;
  }

  if (HDC_2_exists) {
    double t = -999.9;
    double h = -999.9;

    if (hdc2.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
      SystemStatusBits &= ~ SSB_HDC_2;  // Turn Off Bit
    }
    else {
      Output (F("ERR:HDC1 Read"));
      SystemStatusBits |= SSB_HDC_2;  // Turn On Bit
    }

    // 20 HDC2 Temperature
    strcpy (obs.sensor[sidx].id, "hdt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // 21 HDC2 Humidity
    strcpy (obs.sensor[sidx].id, "hdh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
    obs.sensor[sidx++].inuse = true;
  }

  if (LPS_1_exists) {
    float t = lps1.readTemperature();
    float p = lps1.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // 22 LPS1 Temperature
    strcpy (obs.sensor[sidx].id, "lpt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // 23 LPS1 Pressure
    strcpy (obs.sensor[sidx].id, "lpp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;
  }

  if (LPS_2_exists) {
    float t = lps2.readTemperature();
    float p = lps2.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // 24 LPS1 Temperature
    strcpy (obs.sensor[sidx].id, "lpt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // 25 LPS1 Pressure
    strcpy (obs.sensor[sidx].id, "lpp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (HIH8_exists) {
    float t = 0.0;
    float h = 0.0;
    bool status = hih8_getTempHumid(&t, &h);
    if (!status) {
      t = -999.99;
      h = 0.0;
    }
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

    // 26 HIH8 Temperature
    strcpy (obs.sensor[sidx].id, "ht2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // 27 HIH8 Humidity
    strcpy (obs.sensor[sidx].id, "hh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_1_exists) {
    float t = 0.0;
   
    // 28 MCP1 Temperature
    strcpy (obs.sensor[sidx].id, "mt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_2_exists) {
    float t = 0.0;
    
    // 29 MCP2 Temperature
    strcpy (obs.sensor[sidx].id, "mt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp2.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_3_exists) {
    float t = 0.0;

    // 30 MCP3 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp3.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    mcp3_temp = t; // globe temperature
  }

  if (MCP_4_exists) {
    float t = 0.0;

    // 31 MCP4 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp4.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (VEML7700_exists) {
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux) || (lux < QC_MIN_LX)  || (lux > QC_MAX_LX))  ? QC_ERR_LX  : lux;

    // 32 VEML7700 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "lx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }

  if (BLX_exists) {
    float lux=blx_takereading();
    lux = (isnan(lux) || (lux < QC_MIN_BLX)  || (lux > QC_MAX_BLX))  ? QC_ERR_BLX  : lux;

    // 33 DFR BLUX30 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "blx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }

  // PUT DISTANCE HERE 34
  // PUT 2ND RG HERE 35 36 37

  if (PM25AQI_exists) {
    // 38 Standard Particle PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s10;
    obs.sensor[sidx++].inuse = true;

    // 39 Standard Particle PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s25;
    obs.sensor[sidx++].inuse = true;

    // 40 Standard Particle PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s100;
    obs.sensor[sidx++].inuse = true;

    // 41 Atmospheric Environmental PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e10;
    obs.sensor[sidx++].inuse = true;

    // 42 Atmospheric Environmental PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e25;
    obs.sensor[sidx++].inuse = true;

    // 43  Atmospheric Environmental PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e100;
    obs.sensor[sidx++].inuse = true;

    // Clear readings
    pm25aqi_clear();
  }
  
  // 44 Heat Index Temperature
  if (HI_exists) {
    heat_index = hi_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "hi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) heat_index;
    obs.sensor[sidx++].inuse = true;    
  } 
  
  // 45 Wet Bulb Temperature
  if (WBT_exists) {
    wetbulb_temp = wbt_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "wbt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wetbulb_temp;
    obs.sensor[sidx++].inuse = true;  
  }

  // 46 Wet Bulb Globe Temperature
  if (WBGT_exists) {
    float wbgt = 0.0;
    if (MCP_3_exists) {
      wbgt = wbgt_using_wbt(sht1_temp, mcp3_temp, wetbulb_temp); // TempAir, TempGlobe, TempWetBulb
    }
    else {
      wbgt = wbgt_using_hi(heat_index);
    }
    strcpy (obs.sensor[sidx].id, "wbgt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbgt;
    obs.sensor[sidx++].inuse = true;    
  }

  // 47,48 Tinovi Leaf Wetness
  if (TLW_exists) {
    tlw.newReading();
    delay(100);
    float w = tlw.getWet();
    float t = tlw.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs.sensor[sidx].id, "tlww");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) w;
    obs.sensor[sidx++].inuse = true; 

    strcpy (obs.sensor[sidx].id, "tlwt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }

  // 49-52 Tinovi Soil Moisture
  if (TSM_exists) {
    tsm.newReading();
    delay(100);
    float e25 = tsm.getE25();
    float ec = tsm.getEC();
    float vwc = tsm.getVWC();
    float t = tsm.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs.sensor[sidx].id, "tsme25");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) e25;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tsmec");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) ec;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tsmvwc");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) vwc;
    obs.sensor[sidx++].inuse = true; 

    strcpy (obs.sensor[sidx].id, "tsmt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }

  // 53-58 Tinovi Multi Level Soil Moisture
  if (TMSM_exists) {
    soil_ret_t multi;
    float t;

    tmsm.newReading();
    delay(100);
    tmsm.getData(&multi);

    strcpy (obs.sensor[sidx].id, "tmsms1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[0];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[1];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms3");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[2];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms4");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[3];
    obs.sensor[sidx++].inuse = true;

    t = multi.temp[0];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs.sensor[sidx].id, "tmsmt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    t = multi.temp[1];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs.sensor[sidx].id, "tmsmt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }
}



/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing
 * ======================================================================================================================
 */
void OBS_Do() {
  bool OK2Send = false;
  
  Output(F("OBS_DO()"));
 
  I2C_Check_Sensors(); // Make sure Sensors are online
  
  Output(F("OBS_TAKE()"));
  OBS_Take();          // Take a observation
  
  Output(F("OBS_BUILD()"));
  
  if (OBS_Build_JSON()) { // This will also print the JSON out
    // At this point, the obs data structure has been filled in with observation data

    Output(F("OBS->SD"));
    // Serial_writeln (obsbuf);
    SD_LogObservation(obsbuf); // Saves Main observations to Log file. LoRa observations are not saved. LoRa devices have their own SD card

    Output(F("OBS_SEND()"));
  
    if (!OBS_Send(obsbuf)) {  
      Output(F("FS->PUB FAILED"));
      OBS_N2S_Save(); // Saves Main observations and Lora observations
      
      OBS_PubFailCnt++; // This is catching if network is not connected and if we get bad http responses when connected
    }
    else {
      OK2Send = true; 
      Output(F("FS->PUB OK"));
      OBS_PubFailCnt=0;
    }
  }
  else {
    OK2Send = false;
  }

  
  // Publish LoRa Relay Observations   
  if (LORA_exists) {
      
    // We want to transmit all the LoRa msgs or save them to N2S file if we can not transmit them.
    while (lora_relay_need2log()) {      
      OBS_Relay_Build_JSON(); // Copy JSON observation to obsbuf, remove from relay structure
      if (OK2Send) {
         OK2Send = OBS_Send(obsbuf);  // Note a new LoRa RS msgs could be received as we are sending    
      }
      if (!OK2Send) {
        SD_NeedToSend_Add(obsbuf); // Save to N2F File
        Output(F("LR->N2S"));
        Serial_write (msgbuf); 
      }
    }
  }

  // Check if we have any N2S only if we have not added to the file while trying to send OBS
  if (OK2Send) {
    OBS_N2S_Publish(); 
  }
}

/* 
 *=======================================================================================================================
 * OBS_N2S_Publish()
 *=======================================================================================================================
 */
void OBS_N2S_Publish() {
  File fp;
  char ch;
  int i;
  int sent=0;

  memset(obsbuf, 0, sizeof(obsbuf));

  Output (F("OBS:N2S Publish"));

  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  
  if (SD_exists && SD.exists(SD_n2s_file)) {
    Output (F("OBS:N2S:Exists"));

    fp = SD.open(SD_n2s_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

    if (fp) {
      // Delete Empty File or too small of file to be valid
      if (fp.size()<=20) {
        fp.close();
        Output (F("OBS:N2S:Empty"));
        SD_N2S_Delete();
      }
      else {
        if (eeprom.n2sfp) {
          if (fp.size()<=eeprom.n2sfp) {
            // Something wrong. Can not have a file position that is larger than the file
            eeprom.n2sfp = 0; 
          }
          else {
            fp.seek(eeprom.n2sfp);  // Seek to where we left off last time. 
          }
        } 

        // Loop through each line / obs and transmit
        
        // set timer on when we need to stop sending n2s obs
        unsigned long  TimeFromNow;
        if (cf_obs_period == 1) {
          TimeFromNow = time_to_next_obs() - (15 * 1000); // stop sending 15s before next observation period if 1m obs
        }
        else {
          TimeFromNow = time_to_next_obs() - (60 * 1000); // stop sending 1m before next observation period if not 1m obs
        }

        i = 0;
        while (fp.available() && (i < MAX_HTTP_SIZE )) {
          ch = fp.read();

          if (ch == 0x0A) {
            
            Serial_writeln (obsbuf);
            
            if (OBS_Send(obsbuf)) {
              sprintf (Buffer32Bytes, "OBS:N2S[%d]->PUB:OK", sent++);
              Output (Buffer32Bytes);
              //Serial_writeln (obsbuf);

              // setup for next line in file
              i = 0;

              // file position is at the start of the next observation or at eof
              eeprom.n2sfp = fp.position();

              BackGroundWork();
              sprintf (Buffer32Bytes, "OBS:N2S[%d] Contunue", sent);
              Output (Buffer32Bytes); 

              if(millis() > TimeFromNow) {
                // need to break out so new obs can be made
                Output (F("OBS:N2S->TIME2EXIT"));
                break;                
              }
            }
            else {
                sprintf (Buffer32Bytes, "OBS:N2S[%d]->PUB:ERR", sent);
                Output (Buffer32Bytes);
                // On transmit failure, stop processing file.
                break;
            }
            
            // At this point file pointer's position is at the first character of the next line or at eof
            
          } // Newline
          else if (ch == 0x0D) { // CR, LF follows and will trigger the line to be processed       
            obsbuf[i] = 0; // null terminate then wait for newline to be read to process OBS
          }
          else {
            obsbuf[i++] = ch;
          }

          // Check for buffer overrun
          if (i >= MAX_HTTP_SIZE) {
            sprintf (Buffer32Bytes, "OBS:N2S[%d]->BOR:ERR", sent);
            Output (Buffer32Bytes);
            fp.close();
            SD_N2S_Delete(); // Bad data in the file so delete the file           
            return;
          }
        } // end while 

        if (fp.available() <= 20) {
          // If at EOF or some invalid amount left then delete the file
          fp.close();
          SD_N2S_Delete();
        }
        else {
          // At this point we sent 0 or more observations but there was a problem.
          // eeprom.n2sfp was maintained in the above read loop. So we will close the
          // file and next time this function is called we will seek to eeprom.n2sfp
          // and start processing from there forward. 
          fp.close();
          EEPROM_Update(); // Update file postion in the eeprom.
        }
      }
    }
    else {
        Output (F("OBS:N2S->OPEN:ERR"));
    }
  }
}
