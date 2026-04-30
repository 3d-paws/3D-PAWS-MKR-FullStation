/*
 * ======================================================================================================================
 * statmon.cpp - Station Monitor - When Jumper Set Main Loop runs StationMonitor()
 * ======================================================================================================================
 */
#include <RTClib.h>

#include "include/ssbits.h"
#include "include/mkrboard.h"
#include "include/sensors_i2c_44_47.h"
#include "include/sensors.h"
#include "include/wrda.h"
#include "include/cf.h"
#include "include/output.h"
#include "include/support.h"
#include "include/time.h"
#include "include/main.h"
#include "include/statmon.h"

/*
 * ======================================================================================================================
 * StationMonitor() - Display station information
 * ======================================================================================================================
 */
void StationMonitor() {
  static int cycle = 0;
  static int count = 0;
  int r, c, len;

  // Clear display with spaces
  for (r=0; r<4; r++) {
    for (c=0; c<22; c++) {
      oled_lines [r][c] = ' ';
    }
    oled_lines [r][c] = (char) NULL;
  }
  
  // =================================================================
  // Line 0 of OLED
  // =================================================================
  rtc_timestamp();
  len = (strlen (timestamp) > 21) ? 21 : strlen (timestamp);
  for (c=0; c<=len; c++) oled_lines [0][c] = *(timestamp+c);
  Serial_writeln (timestamp);
  
  // =================================================================
  // Line 1 of OLED Wind Direction and Speed RG1 RG2 Interrupt counts
  // =================================================================
  memset(msgbuf, 0, sizeof(msgbuf));

  if (AS5600_exists) {
    sprintf (msgbuf+strlen(msgbuf), "D:%3d S:%02d", 
      Wind_SampleDirection(), anemometer_interrupt_count);
  }
  else {
    sprintf (msgbuf+strlen(msgbuf), "D:NF  S:NF");
  }
  
  if (cf_rg1_enable) {
    sprintf (msgbuf+strlen(msgbuf), " R1:%02d", raingauge1_interrupt_count); 
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0;
  }
  else {
    sprintf (msgbuf+strlen(msgbuf), " R1:ND");
  }
  
  if (cf_op1 == OP1_STATE_RAIN) {
    sprintf (msgbuf+strlen(msgbuf), " 2:%02d", raingauge2_interrupt_count);
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0;
  }
  else {
    sprintf (msgbuf+strlen(msgbuf), " 2:ND");
  }

  len = (strlen (msgbuf) > 21) ? 21 : strlen (msgbuf);
  for (c=0; c<=len; c++) oled_lines [1][c] = *(msgbuf+c);
  Serial_writeln (msgbuf);
  
  // =================================================================
  // Line 2 of OLED
  // =================================================================
  memset(msgbuf, 0, sizeof(msgbuf));

  if ((cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) {
    float ds = DS_Median();
    sprintf (msgbuf+strlen(msgbuf), "D:%4d %d", DS_Median(), anemometer_interrupt_count);
  }
  else {
    sprintf (msgbuf+strlen(msgbuf), "D! W:%d", anemometer_interrupt_count);
  }

  int bcs = get_batterystate();
  sprintf (msgbuf+strlen(msgbuf), " B:%s H:%04X", 
      batterystate[bcs], SystemStatusBits);
      
  len = (strlen (msgbuf) > 21) ? 21 : strlen (msgbuf);
  for (c=0; c<=len; c++) oled_lines [2][c] = *(msgbuf+c);
  Serial_writeln (msgbuf);

  // =================================================================
  // Line 3 of OLED Cycle between multiple sensors
  // =================================================================
  if (cycle == 0) {
    if (BMX_1_exists) {
      float p,t,h;
      bmx1_read(p, t, h);
      sprintf (msgbuf, "B1 %.2f %.2f %.2f", p,t,h);
    }
    else {
      sprintf (msgbuf, "B1 NF");
    }
  }
  
  if (cycle == 1) {
    if (BMX_2_exists) {
      float p,t,h;
      bmx2_read(p, t, h);
      sprintf (msgbuf, "B2 %.2f %.2f %.2f", p,t,h);
    }
    else {
      sprintf (msgbuf, "B2 NF");
    }
  }

  if (cycle == 2) {
    memset(msgbuf, 0, sizeof(msgbuf));
      
    if (MCP_1_exists) {
      float mcp_temp = mcp1.readTempC();   
      sprintf (msgbuf, "MCP1 T%.2f", mcp_temp);
    }
    else {
      sprintf (msgbuf, "MCP1 NF");
    }
  }

  if (cycle == 3) {
    if (MCP_2_exists) {
      float mcp_temp = mcp2.readTempC();   
      sprintf (msgbuf, "MCP2 T%.2f", mcp_temp);
    }
    else {
      sprintf (msgbuf, "MCP2 NF");
    }
  }

  if (cycle == 4) {
   sensor_i2c_44_47_statmon(0, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }
  if (cycle == 5) {
   sensor_i2c_44_47_statmon(1, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }
  if (cycle == 6) {
   sensor_i2c_44_47_statmon(2, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }
  if (cycle == 7) {
   sensor_i2c_44_47_statmon(3, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }

  if (cycle == 8) {   
    if (HTU21DF_exists) {
      float htu_humid = htu.readHumidity();
      float htu_temp = htu.readTemperature();

      sprintf (msgbuf, "HTU H:%0.2f T:%0.2f", htu_humid, htu_temp);
    }
    else {
      sprintf (msgbuf, "HTU NF"); 
    } 
  }

  if (cycle == 9) {   
    if (VEML7700_exists) {
      float lux = veml.readLux(VEML_LUX_AUTO);
      lux = (isnan(lux)) ? 0.0 : lux;
        sprintf (msgbuf, "LX L%.2f", lux);
    }
    else {
      sprintf (msgbuf, "LX NF");
    }
  }

  if (cycle == 10) {   
    if (HIH8_exists) {
      float t = 0.0;
      float h = 0.0;
      bool status = hih8_getTempHumid(&t, &h);
      if (!status) {
        t = -999.99;
        h = 0.0;
      }
      sprintf (msgbuf, "HIH8 T%.2f H%.2f", t,h);
    }
    else {
      sprintf (msgbuf, "HIH8 NF");
    }
  }
  
  if (cycle == 11) {
    if (PM25AQI_exists) {
      sprintf (msgbuf, "PM 10:%d 25:%d 100:%d", 
        pm25aqi_obs.e10,
        pm25aqi_obs.e25,
        pm25aqi_obs.e100);
    }
    else {
      sprintf (msgbuf, "PM NF");
    }
  }

  len = (strlen (msgbuf) > 21) ? 21 : strlen (msgbuf);
  for (c=0; c<=len; c++) oled_lines [3][c] = *(msgbuf+c);
  Serial_writeln (msgbuf);

  // Give the use some time to read line 3 before changing
  if (count++ >= 5) {
    cycle = ++cycle % 12; // << +1
    count = 0;
  }
  
  OLED_update();
}
