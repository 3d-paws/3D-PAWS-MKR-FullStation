/*
 * ======================================================================================================================
 *  OBS.h - Observations
 *  
 *  SEE https://arduinojson.org/v6/how-to/use-arduinojson-with-httpclient/
 * ======================================================================================================================
 */
#define OBSERVATION_INTERVAL      60   // Seconds

#define MAX_SENSORS         48

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

unsigned long lastOBS = 0;              // used to time next observation

void OBS_N2S_Publish();   // Prototype this function to aviod compile function unknown issue.


/*
 * ======================================================================================================================
 * OBS_URL_Send() - Do a GET request to log observation, process returned text for result code and set return status
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
bool OBS_URL_Send(char *obs)
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
    Output("OBS:SEND->HTTP");
    if (!client.connect(cf_webserver, cf_webserver_port)) {
      NoNetworkLoopCycleCount++;   // reset modem and reboot if count gets to our set max - done in main loop()
      Output("OBS:HTTP FAILED");
    }
    else {
      NoNetworkLoopCycleCount = 0;  // reset the counter to prevent rebooting
        
      Output("OBS:HTTP CONNECTED");
      
      // Make a HTTP request:
      client.print("GET ");
      client.print(obs); // path
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(cf_webserver);
      client.println("Connection: close");
      client.println();

      Output("OBS:HTTP SENT");

      // Check every 500ms for data, up to 3 minutes. While waiting take Wind Readings every 1s
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
          if (++exit_timer >= 180) { // after 3 minutes lets call it quits
            break;
          }
        }
      }
      
      Output("OBS:HTTP WAIT");
        
      // Read first line of HTTP Response, then get out of the loop
      r=0;
      while ((client.connected() || client.available() ) && r<63 && !posted) {
        response[r] = client.read();
        response[++r] = 0;  // Make string null terminated
        if (strstr(response, "200 OK") != NULL) { // Does response includes a "200 OK" substring?
          NetworkHasBeenOperational = true;
          posted = true;
          break;
        }
        if ((response[r-1] == 0x0A) || (response[r-1] == 0x0D)) { // LF or CR
          // if we got here then we never saw the 200 OK
          break;
        }
      }
  
      // Read rest of the response after first line
      // while (client.connected() || client.available()) { //connected or data available
      //   char c = client.read(); //gets byte from ethernet buffer
      //   Serial.print (c);
      // }

      sprintf (buf, "OBS:%s", response);
      Output(buf);
      
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
    struct tm  tms; // <<<<<<<<<<<<<<<<< Is this used???
   
    memset(obsbuf, 0, sizeof(obsbuf));

    sprintf (obsbuf, "/measurements/url_create?key=%s&instrument_id=%s", cf_apikey, cf_instrument_id);

    tm *dt = gmtime(&obs.ts); 
      
    sprintf (obsbuf+strlen(obsbuf), "&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
      
    sprintf (obsbuf+strlen(obsbuf), "&css=%d", obs.css);

    // Modify System Status and Set From Need to Send file bit
    obs.hth |= SSB_FROM_N2S; // Turn On Bit
    sprintf (obsbuf+strlen(obsbuf), "&hth=%d", obs.hth);
   
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            // sprintf (obsbuf+strlen(obsbuf), "&%s=%d%%2E%d", obs.sensor[s].id, 
            //  (int)obs.sensor[s].f_obs,  (int)(obs.sensor[s].f_obs*1000)%1000);
            sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    Serial_writeln (obsbuf);
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output("OBS-> N2S");
  }
  else {
    Output("OBS->N2S OBS:Empty");
  }
}

/*
 * ======================================================================================================================
 * OBS_LOG_Add() - Create observation in obsbuf and save to SD card.
 * 
 * {"at":"2022-02-13T17:26:07","css":18,"hth":0,"bcs":2,"bpc":63.2695,.....,"mt2":20.5625}
 * ======================================================================================================================
 */
void OBS_LOG_Add() {  
  if (obs.inuse) {     // Sanity check

    memset(obsbuf, 0, sizeof(obsbuf));

    sprintf (obsbuf, "{");

    tm *dt = gmtime(&obs.ts); 
    
    sprintf (obsbuf+strlen(obsbuf), "\"at\":\"%d-%02d-%02dT%02d:%02d:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);
      
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
            Output ("WhyAmIHere?");
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");
    
    Output("OBS->SD");
    Serial_writeln (obsbuf);
    SD_LogObservation(obsbuf); 
  }
  else {
    Output("OBS->SD OBS:Empty");
  }
}

/*
 * ======================================================================================================================
 * OBS_URL_Build() - Create observation in obsbuf for sending to Chords
 * 
 * Example /measurements/url_create?key=FOOBAR&instrument_id=53&at=2022-05-17T17%3A40%3A04&hth=8770
 * ======================================================================================================================
 */
bool OBS_URL_Build() {   
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));

    sprintf (obsbuf, "/measurements/url_create?key=%s&instrument_id=%s", cf_apikey, cf_instrument_id);

    tm *dt = gmtime(&obs.ts); 
    
    sprintf (obsbuf+strlen(obsbuf), "&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
      dt->tm_hour, dt->tm_min, dt->tm_sec);

    sprintf (obsbuf+strlen(obsbuf), "&css=%d", obs.css);
    sprintf (obsbuf+strlen(obsbuf), "&hth=%d", obs.hth);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), "&%s=%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
      }
    }

    Output("OBS->URL");
    Serial_writeln (obsbuf);
    return (true);
  }
  else {
    Output("OBS->URL OBS:Empty");
    return (false);
  }
}


/*
 * ======================================================================================================================
 * OBS_RS_Build() - Create observation in obsbuf for rain and soil sensors (RS)
 *                  Requires we have at least one observation needing to be logged before we call this function.
 *                  The Lora NodeID is also the Chords instrumnet id
 * ======================================================================================================================
 */
void OBS_RS_Build() {
  LORA_RS_OBS_STR *m;
  char tag[8];

  // Locate message we need to log
  for (int i=0; i< NUMBER_RS_LORA_DEVICES; i++) {
    if (lora_rs[i].need2log) {
      m = &lora_rs[i];
      break;
    }
  }
  
  memset(obsbuf, 0, sizeof(obsbuf));

  sprintf (obsbuf, "/measurements/url_create?key=%s&instrument_id=%s", cf_apikey, m->unit_id);

  tm *dt = gmtime(&obs.ts);
    
  sprintf (obsbuf+strlen(obsbuf), "&at=%d-%02d-%02dT%02d%%3A%02d%%3A%02d",
    dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday,
    dt->tm_hour, dt->tm_min, dt->tm_sec);

  sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", "bv",  m->voltage);
  sprintf (obsbuf+strlen(obsbuf), "&%s=%d",   "hth", m->hth);
  sprintf (obsbuf+strlen(obsbuf), "&%s=%d",   "rg",  m->rg);

  if (m->message_type == 3 || m->message_type == 5){
    for (int i=0; i<SM_PROBES; i++) {
      // Soil Temp
      sprintf(tag, "st%d", i+1);
      sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", tag, m->st);

      // Soil Moisture
      sprintf(tag, "sm%d", i+1);
      sprintf (obsbuf+strlen(obsbuf), "&%s=%d", tag, m->sm);
    }
  }

  if (m->message_type == 4 || m->message_type == 5){
    for (int i=0; i<SM_BMX_SENSORS; i++) {
      sprintf(tag, "p%d", i+1);
      sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", tag, m->p);

      sprintf(tag, "t%d", i+1);
      sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", tag, m->t);

      sprintf(tag, "h%d", i+1);
      sprintf (obsbuf+strlen(obsbuf), "&%s=%.1f", tag, m->h);
    }
  }

  m->need2log = false;
  m->rg = 0;
  m->rgds = 0;
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
  while (lora_rs_need2log()) {
    OBS_RS_Build();
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output("RS->N2S");
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
  float rain = 0.0;
  float ws = 0.0;
  int wd = 0;
  unsigned long rgds;    // rain gauge delta seconds, seconds since last rain gauge observation logged
  float mcp1_temp = 0.0;
  float sht1_humid = 0.0;
  float heat_index = 0.0;

  // Safty Check for Vaild Time
  if (!STC_valid) {
    Output ("OBS_Take: Time NV");
    return;
  }
  
  OBS_Clear(); // Just do it again as a safty check

  Wind_GustUpdate(); // Update Gust and Gust Direction readings

  obs.inuse = true;
  // obs.ts = Time.now();
  obs.ts = lastOBS;     // Set in main loop no need to call stc.getEpoch();
  obs.css = GetCellSignalStrength();
  obs.hth = SystemStatusBits;

  // 00 Rain Gauge - Each tip is 0.2mm of rain
  rgds = (millis()-raingauge_interrupt_stime)/1000;
  rain = raingauge_interrupt_count * 0.2;
  rain = (isnan(rain) || (rain < QC_MIN_RG) || (rain > ((rgds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rain;
  raingauge_interrupt_count = 0;
  raingauge_interrupt_stime = stc.getEpoch();
  raingauge_interrupt_ltime = 0; // used to debounce the tip

  // 01 Rain Gauge
  strcpy (obs.sensor[sidx].id, "rg");
  obs.sensor[sidx].type = F_OBS;
  obs.sensor[sidx].f_obs = rain;
  obs.sensor[sidx++].inuse = true;

  // 02 Rain Gauge Delta Seconds
  strcpy (obs.sensor[sidx].id, "rgs");
  obs.sensor[sidx].type = U_OBS;
  obs.sensor[sidx].u_obs = rgds;
  obs.sensor[sidx++].inuse = true;

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

  if (MCP_1_exists) {
    float t = 0.0;
   
    // 15 MCP1 Temperature
    strcpy (obs.sensor[sidx].id, "mt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    mcp1_temp = t; // save for derived observations
  }

  if (MCP_2_exists) {
    float t = 0.0;
    
    // 16 MCP2 Temperature
    strcpy (obs.sensor[sidx].id, "mt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp2.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (SHT_1_exists) {
    float t = 0.0;
    float h = 0.0;

    // 17 SHT1 Temperature
    strcpy (obs.sensor[sidx].id, "st1");
    obs.sensor[sidx].type = F_OBS;
    t = sht1.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // 18 SHT1 Humidity   
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

    // 19 SHT2 Temperature
    strcpy (obs.sensor[sidx].id, "st2");
    obs.sensor[sidx].type = F_OBS;
    t = sht2.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // 20 SHT2 Humidity   
    strcpy (obs.sensor[sidx].id, "sh2");
    obs.sensor[sidx].type = F_OBS;
    h = sht2.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
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

    // 21 HIH8 Temperature
    strcpy (obs.sensor[sidx].id, "ht2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // 22 HIH8 Humidity
    strcpy (obs.sensor[sidx].id, "hh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }

  if (VEML7700_exists) {
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux) || (lux < QC_MIN_LX)  || (lux > QC_MAX_LX))  ? QC_ERR_LX  : lux;

    // 23 VEML7700 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "lx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }

  if (PM25AQI_exists) {
    // 24 Standard Particle PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s10;
    obs.sensor[sidx++].inuse = true;

    // 25 Standard Particle PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s25;
    obs.sensor[sidx++].inuse = true;

    // 26 Standard Particle PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s100;
    obs.sensor[sidx++].inuse = true;

    // 27 Atmospheric Environmental PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e10;
    obs.sensor[sidx++].inuse = true;

    // 28 Atmospheric Environmental PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e25;
    obs.sensor[sidx++].inuse = true;

    // 29  Atmospheric Environmental PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e100;
    obs.sensor[sidx++].inuse = true;

    // Clear readings
    pm25aqi_clear();

  // 30 Heat Index Temperature
  if (HI_exists) {
    heat_index = hi_calculate(mcp1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "hi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) heat_index;
    obs.sensor[sidx++].inuse = true;    
  }
  
  // 31 Wet Bulb Temperature
  if (WBT_exists) {
    strcpy (obs.sensor[sidx].id, "wbt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbt_calculate(mcp1_temp, sht1_humid);
    obs.sensor[sidx++].inuse = true;    
  }

  // 32 Wet Bulb Globe Temperature
  if (WBGT_exists) {
    strcpy (obs.sensor[sidx].id, "wbgt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbgt_calculate(heat_index);
    obs.sensor[sidx++].inuse = true;    
  }
  }
   
  //
  // Add LoRa SG Observations 
  //
  if (LORA_exists) {
    lora_msg_check(); // do not use obsbuf in LoRa check
    // Check for Observation from LoRa Stream Gauge
    if (lora_sg.need2log) {
      // 33 Stream Gauge Reading
      sprintf (obs.sensor[sidx].id, "sg%d", lora_sg.unit_id);
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = lora_sg.stream_gauge;
      obs.sensor[sidx++].inuse = true;

      // 34 Stream Gauge Voltage
      sprintf (obs.sensor[sidx].id, "sg%dv", lora_sg.unit_id);
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = lora_sg.voltage;
      obs.sensor[sidx++].inuse = true;

      // Add the BME280 Sensor information
      if (lora_sg.message_type == 3) {  
        if (lora_sg.p[0] != 0.0) {
          // 35 Stream Gauge Preasure 1
          sprintf (obs.sensor[sidx].id, "sg%dp1", lora_sg.unit_id);
          obs.sensor[sidx].type = F_OBS;
          obs.sensor[sidx].f_obs = lora_sg.p[0];
          obs.sensor[sidx++].inuse = true;
          // 36 Stream Gauge Temperature 1
          sprintf (obs.sensor[sidx].id, "sg%dt1", lora_sg.unit_id);
          obs.sensor[sidx].type = F_OBS;
          obs.sensor[sidx].f_obs = lora_sg.t[0];
          obs.sensor[sidx++].inuse = true;
          // 37 Stream Gauge Humidity 1
          sprintf (obs.sensor[sidx].id, "sg%dh1", lora_sg.unit_id);
          obs.sensor[sidx].type = F_OBS;
          obs.sensor[sidx].f_obs = lora_sg.h[0];
          obs.sensor[sidx++].inuse = true;
        }
        if (lora_sg.p[1] != 0.0) {
          // 38 Stream Gauge Preasure 2
          sprintf (obs.sensor[sidx].id, "sg%dp2", lora_sg.unit_id);
          obs.sensor[sidx].type = F_OBS;
          obs.sensor[sidx].f_obs = lora_sg.p[1];
          obs.sensor[sidx++].inuse = true;
          // 39 Stream Gauge Temperature 2
          sprintf (obs.sensor[sidx].id, "sg%dt2", lora_sg.unit_id);
          obs.sensor[sidx].type = F_OBS;
          obs.sensor[sidx].f_obs = lora_sg.t[1];
          obs.sensor[sidx++].inuse = true;
          // 40 Stream Gauge Humidity 2
          sprintf (obs.sensor[sidx].id, "sg%dh2", lora_sg.unit_id);
          obs.sensor[sidx].type = F_OBS;
          obs.sensor[sidx].f_obs = lora_sg.h[1];
          obs.sensor[sidx++].inuse = true;
        }
        lora_sg_msg_clear();  // initialize data structure for next message 
      }
    }
  }

  // Main loop set lastOBS no need for this 
  // lastOBS =  obs.ts;
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing
 * ======================================================================================================================
 */
void OBS_Do() {

  Output("OBS_DO()");
  
  I2C_Check_Sensors(); // Make sure Sensors are online
  
  Output("OBS_TAKE()");

  OBS_Take();          // Take a observation
  
  Output("OBS_ADD()");
  
  // At this point, the obs data structure has been filled in with observation data
  OBS_LOG_Add(); // Saves Main observations to Log file. LoRa observations are not saved. LoRa devices have their own SD card.

 
  // Send Observation
  Output("OBS_BUILD()");
  
  OBS_URL_Build();

  Output("OBS_SEND()");
  
  if (!OBS_URL_Send(obsbuf)) {  
    Output("FS->PUB FAILED");
    OBS_N2S_Save(); // Saves Main observations and Lora observations
  }
  else {
    bool OK2Send = true;
        
    Output("FS->PUB OK");
   
    // Publish LoRa Rain and Soil Observations
    // Note we do not save LoRa Obsercations to SD log file, Each LoRa device has their own SD card for this
    if (LORA_exists) {
      // We want to transmit all RS msgs or save them to N2S file if we can not transmit them.
      while (lora_rs_need2log()) {
        OBS_RS_Build();
        if (OK2Send) {
          OK2Send = OBS_URL_Send(obsbuf);  // Note a new LoRa RS msgs could be received as we are sending
          if (OK2Send) {
            Output("RS->PUB OK");    
          }
          else {
            Output("RS->PUB ERR");
          }
        }
        if (!OK2Send) {
          sprintf (obsbuf+strlen(obsbuf), ",RS");  // Add Particle Event Type after JSON structure
          SD_NeedToSend_Add(obsbuf); // Save to N2F File
          Output("RS->N2S");
          Serial_write (obsbuf); 
        }
      }
    }

    // Check if we have any N2S only if we have not added to the file while trying to send OBS
    if (OK2Send) {
      OBS_N2S_Publish(); 
    }
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

  Output ("OBS:N2S Publish");

  if (SD_exists && SD.exists(SD_n2s_file)) {
    Output ("OBS:N2S:Exists");

    fp = SD.open(SD_n2s_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

    if (fp) {
      // Delete Empty File or too small of file to be valid
      if (fp.size()<=20) {
        fp.close();
        Output ("OBS:N2S:Empty");
        SD_N2S_Delete();
      }
      else {
        if (SD_N2S_POSITION) {
          fp.seek(SD_N2S_POSITION);  // Seek to where we left off last time. 
        }

        // Loop through each line / obs and transmit
        i = 0;
        while (fp.available() && (i < MAX_MSGBUF_SIZE )) {
          ch = fp.read();

          if (ch == 0x0A) {  // newline
            if (OBS_URL_Send(obsbuf)) { 
              sprintf (Buffer32Bytes, "OBS:N2S[%d]->PUB:OK", sent++);
              Output (Buffer32Bytes);
              Serial_writeln (obsbuf);

              // setup for next line in file
              i = 0;

              // file position is at the start of the next observation or at eof
              SD_N2S_POSITION = fp.position();
              
              BackGroundWork();
              sprintf (Buffer32Bytes, "OBS:N2S[%d] Contunue", sent);
              Output (Buffer32Bytes);         
            }
            else {
                sprintf (Buffer32Bytes, "OBS:N2S[%d]->PUB:ERR", sent);
                Output (Buffer32Bytes);
                // On transmit failure, stop processing file.
                break;
            }
            
            // At this point file pointer's position is at the first character of the next line or at eof

            // We could be in this loop for a while. We don't want to miss our 1 minute observation.
            // So make the observation and stay in the loop if we have space in the OBS array.
            // We need to avoid a full array that would cause all observations to be saved to N2S file 
            // we currently have open, a bad thing.
            if ( ((stc.getEpoch() - lastOBS) > OBSERVATION_INTERVAL)) {   
              Output ("OBS:N2S:Exit4DoOBS");
              break; 
            }
          } // Newline
          else if (ch == 0x0D) { // CR, LF follows and will trigger the line to be processed       
            obsbuf[i] = 0; // null terminate then wait for newline to be read to process OBS
          }
          else {
            obsbuf[i++] = ch;
          }

          // Check for buffer overrun
          if (i >= MAX_MSGBUF_SIZE) {
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
          // SD_N2S_POSITION was maintained in the above read loop. So we will close the
          // file and next time this function is called we will seek to SD_N2S_POSITION
          // and start processing from there forward. 
          fp.close();
        }
      }
    }
    else {
        Output ("OBS:N2S->OPEN:ERR");
    }
  }
}
