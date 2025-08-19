/*
 * ======================================================================================================================
 *  PM.h PowerManagement LiPo Battery Monitor and PMIC 
 *  
 *  PMIC bq24195 is on board the MKR1500
 *  
 *  FROM: Arduino15/packages/arduino/hardware/samd/1.8.13/variants/mkrnb1500/
 *  FILE variant.h
 *  #define ADC_BATTERY  (32u)
 *  
 *  FILE: variant.cpp
 *  #if defined(USE_BQ24195L_PMIC)
 *  pinMode(ADC_BATTERY, OUTPUT);
 *  digitalWrite(ADC_BATTERY, LOW);
 *  delay(10);
 *  pinMode(ADC_BATTERY, INPUT);
 *  delay(100);
 *
 *  bool batteryPresent = analogRead(ADC_BATTERY) > 600;
 * if (batteryPresent) {
 *   enable_battery_charging();
 * }
 * disable_battery_fet(!batteryPresent);
 * set_voltage_current_thresholds();
 * #endif
 * ======================================================================================================================
 */

bool PMIC_exists = false;

#define BATTERY_STATE_UNKNOWN 0
#define BATTERY_STATE_NOT_CHARGING 1
#define BATTERY_STATE_CHARGING 2
#define BATTERY_STATE_CHARGED 3
#define BATTERY_STATE_DISCHARGING 4
#define BATTERY_STATE_FAULT 5
#define BATTERY_STATE_DISCONNECTED 6

/*
 * ======================================================================================================================
 *  Power Management IC - PMIC (bq24195)
 * ======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * get_batterystate()
 *  BATTERY_STATE_UNKNOWN = 0,
 *  BATTERY_STATE_NOT_CHARGING = 1,
 *  BATTERY_STATE_CHARGING = 2,
 *  BATTERY_STATE_CHARGED = 3,
 *  BATTERY_STATE_DISCHARGING = 4,
 *  BATTERY_STATE_FAULT = 5,
 *  BATTERY_STATE_DISCONNECTED = 6
 *=======================================================================================================================
 */
byte get_batterystate() {
  bool power_good;
  int  charge_status;
  byte chrg_stat;

  /* 
  8.5.1.9 System Status Register REG08 - Table 15. REG08 System Status Register Description
  Bit 7 VBUS_STAT[1] R 00 – Unknown (no input, or DPDM detection incomplete), 01 – USB host, 10 – Adapter
  Bit 6 VBUS_STAT[0] R port, 11 – OTG
  
  Bit 5 CHRG_STAT[1] R 00 – Not Charging, 01 – Pre-charge (<VBATLOWV), 10 – Fast Charging, 11 – Charge
  Bit 4 CHRG_STAT[0] R Termination Done
  
  Bit 3 DPM_STAT R 0 – Not DPM, 1 – VINDPM or IINDPM
  Bit 2 PG_STAT R 0 – Not Power Good, 1 – Power Good
  Bit 1 THERM_STAT R 0 – Normal, 1 – In Thermal Regulation
  Bit 0 VSYS_STAT R 0 – Not in VSYSMIN regulation (BAT > VSYSMIN), 1 – In VSYSMIN regulation (BAT <VSYSMIN)
   */

  charge_status = PMIC.chargeStatus();
  if (charge_status == -1) {
    return(BATTERY_STATE_UNKNOWN);
  }
  power_good = PMIC.isPowerGood();              // true if on USB
  chrg_stat = (charge_status & 0x30) >> 4;
  switch (chrg_stat) {
    case 0 : return(BATTERY_STATE_NOT_CHARGING); // Not Charging
    case 1 : return(BATTERY_STATE_CHARGING);     // Pre-charge (<VBATLOWV) - On USB Power
    case 2 : return(BATTERY_STATE_CHARGING);     // Fast Charging          - On USB Power
    case 3 :                                     // Charged
      if (power_good) {
        return(BATTERY_STATE_CHARGED); // On USB Power
      }
      else {
        return(BATTERY_STATE_DISCHARGING);  // Discharching so no usb power and we must be on battery
      }
   }
}

/*
 * Function Name  : disableCharge
 * Description    : Disable the battery charger
 * Input          : NONE
 * Return         : 0 on Error, 1 on Success 
 * bool PMICClass::disableCharge() {
*/

/* 
 *=======================================================================================================================
 * pmic_initialize() - BQ24195
 * SEE https://github.com/particle-iot/device-os  
 *            device-os-develop/wiring/src/spark_wiring_power.cpp
 *            
 * https://support.arduino.cc/hc/en-us/articles/360016119199-What-is-the-meaning-of-CHRG-LED-different-states-in-MKR-boards-          
 * LED STAT Pin State
 *   Charging in progress (including recharge) = LOW
 *   Charging complete                         = HIGH
 *   Sleep mode, charge disable                = HIGH
 *   Charge suspend (Input over-voltage, TS fault, timer fault, input or system over-voltage) =  blinking at 1Hz
 *=======================================================================================================================
 */
bool pmic_initialize() {
  if (!PMIC.begin()) {
    Output (F("PM:INIT FAILED"));    SystemStatusBits |= SSB_PMIC;  // Turn On Bit
  }
  else {
    PMIC_exists = true;
    Output (F("PM:FOUND"));

    // No Battery connected so tuen off the charger
    if (PMIC.disableCharge()) {
      Output (F("PM:CHRGR:DISABLED"));
    }
    else {
      Output (F("PM:CHRGR:NOTDISABLED"));
    }

    sprintf (msgbuf,"PM:MSV=%.2f", PMIC.getMinimumSystemVoltage());  // Factory Default is 3.5
    Output (msgbuf);
    // PMIC.setMinimumSystemVoltage(3.0);
    // sprintf (msgbuf,"PM:SMSV=%.2f", PMIC.getMinimumSystemVoltage());
    // Output (msgbuf);
    
    sprintf (msgbuf,"PM:BS=%d", get_batterystate());
    Output (msgbuf);
    sprintf (msgbuf,"PM:PS=%s", (PMIC.isPowerGood()) ? "GOOD" : "BAD");
    Output (msgbuf);
  }
  return (PMIC_exists);
}
