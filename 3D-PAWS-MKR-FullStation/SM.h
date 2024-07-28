/*
 * ======================================================================================================================
 * SM.h - StationMonitor - When Jumper set Display StationMonitor()
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * StationMonitor() - On OLED display station information
 * ======================================================================================================================
 */
void StationMonitor() {
  int r, c, len;
  float bmx_pressure = 0.0;
  float bmx_temp = 0.0;
  float bmx_humid;
  float htu_humid = 0.0;
  float htu_temp;
  float mcp_temp = 0.0;
  float si_vis = 0.0;
  float si_ir = 0.0;
  float si_uv = 0.0;
  char Buffer16Bytes[16];

  int CellSignalStrength = GetCellSignalStrength();

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
  if (AS5600_exists) {
    sprintf (Buffer16Bytes, "D:%3d", Wind_SampleDirection());
  }
  else {
    sprintf (Buffer16Bytes, "D:NF ");
  }

  sprintf (Buffer32Bytes, "R:%02d,S:%02d,%s %X",
    raingauge_interrupt_count,
    anemometer_interrupt_count,
    Buffer16Bytes,
    SystemStatusBits);
  len = (strlen (Buffer32Bytes) > 21) ? 21 : strlen (Buffer32Bytes);
  for (c=0; c<=len; c++) oled_lines [0][c] = *(Buffer32Bytes+c);
  Serial_writeln (Buffer32Bytes);

  // =================================================================
  // Line 1 of OLED
  // =================================================================
  if (BMX_1_exists) {
    switch (BMX_1_chip_id) {
       case BMP280_CHIP_ID :
         bmx_pressure = bmp1.readPressure()/100.0F;           // bmxp1
         bmx_temp = bmp1.readTemperature();                   // bmxt1
       break;
       case BME280_BMP390_CHIP_ID :
         if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
           bmx_pressure = bme1.readPressure()/100.0F;           // bmxp1
           bmx_temp = bme1.readTemperature();                   // bmxt1
           bmx_humid = bme1.readHumidity();                     // bmxh1 
         }
         else { // BMP390
           bmx_pressure = bm31.readPressure()/100.0F;
           bmx_temp = bm31.readTemperature();
         }
       break;
       case BMP388_CHIP_ID :
          bmx_pressure = bm31.readPressure()/100.0F;
          bmx_temp = bm31.readTemperature();
       break;
       default:
          Output ("BMX:WTF"); // This should not happen.
       break;
    }
    sprintf (Buffer32Bytes, "P:%d.%02d T:%d.%02d", 
      (int)bmx_pressure, (int)(bmx_pressure*100)%100,
      (int)bmx_temp, (int)(bmx_temp*100)%100);
  }
  else {
    sprintf (Buffer32Bytes, "BMX:NF");
  }
  len = (strlen (Buffer32Bytes) > 21) ? 21 : strlen (Buffer32Bytes);
  for (c=0; c<=len; c++) oled_lines [1][c] = *(Buffer32Bytes+c);
  Serial_writeln (Buffer32Bytes);

  // =================================================================
  // Line 2 of OLED
  // =================================================================
  if (MCP_1_exists) {
    // mcp.shutdown_wake(0);        // wake up, ready to read! - power consumption ~200 micro Ampere
    mcp_temp = mcp1.readTempC();   
    // mcp.shutdown_wake(1);        // shutdown MCP9808 - power consumption ~0.1 mikro Ampere

    sprintf (Buffer16Bytes, "T%d.%02d", (int)mcp_temp, (int)(mcp_temp*100)%100);
  }
  else {
    sprintf (Buffer16Bytes, "MCP:NF");
  }

  if (HTU21DF_exists) {
    htu_humid = htu.readHumidity();
    htu_temp = htu.readTemperature();

    sprintf (Buffer32Bytes, "H:%02d.%02d%s C:%d", 
      (int)htu_humid, (int)(htu_humid*100)%100,
      Buffer16Bytes, CellSignalStrength);
  }
  else {
    sprintf (Buffer32Bytes, "HTU:NF %s C:%d", 
      Buffer16Bytes, CellSignalStrength);  
  }


  len = (strlen (Buffer32Bytes) > 21) ? 21 : strlen (Buffer32Bytes);
  for (c=0; c<=len; c++) oled_lines [2][c] = *(Buffer32Bytes+c);
  Serial_writeln (Buffer32Bytes);

  // =================================================================
  // Line 3 of OLED
  // =================================================================
  if (VEML7700_exists) {
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux)) ? 0.0 : lux;
    sprintf (Buffer32Bytes, "LX:%02d.%1d", (int)lux, (int)(lux*10)%10);
  }
  else {
    sprintf (Buffer32Bytes, "LX:NF");
  }

  len = (strlen (Buffer32Bytes) > 21) ? 21 : strlen (Buffer32Bytes);
  for (c=0; c<=len; c++) oled_lines [3][c] = *(Buffer32Bytes+c);
  Serial_writeln (Buffer32Bytes);

  OLED_update();
}
