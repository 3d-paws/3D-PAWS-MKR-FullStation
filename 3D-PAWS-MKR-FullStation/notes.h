 /*
  * ======================================================================================================================
  * notes.h - Things to know about
  * ======================================================================================================================
  */

// ======================================================================================================================
// UBLOX Firmware Upgrading (Latest: 2022-06-17 Modem: L0.0.00.00.05.12, Application: A.02.19)
// ======================================================================================================================
//   https://support.arduino.cc/hc/en-us/articles/360016119159-How-to-check-the-firmware-version-the-SARA-radio-module-u-blox-on-the-MKR-NB-1500
//   https://docs.particle.io/reference/technical-advisory-notices/tan001-sara-r410m-124-day/
//   https://portal.u-blox.com/s/feed/0D52p0000CUPR6qCQH
//   https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwjalZee8OT4AhXtHzQIHfkqAFoQFnoECA8QAQ&url=https%3A%2F%2Fcontent.u-blox.com%2Fsites%2Fdefault%2Ffiles%2FSARA-R410M-02B-03_PCN_UBX-20058104.pdf&usg=AOvVaw0gVP1zkO6p-yG_JwyaiPiJ
//   https://www.u-blox.com/en/product/sara-r4-series
//   https://www.u-blox.com/en/product/sara-r4-series#tab-documentation-resources
//   https://www.u-blox.com/sites/default/files/SARA-R4-FW-Update_AppNote_UBX-17049154.pdf
//   https://www.u-blox.com/en/product/m-center
//   https://www.ebay.com/itm/185498266481?chn=ps&norover=1&mkevt=1&mkrid=711-117182-37290-0&mkcid=2&itemid=185498266481&targetid=1263433205254
//   https://github.com/hologram-io/hologram-tools/tree/master/novaupdate
//
//
//   1) Install Qualcom Drivers
//   2) Install EasyFlash
//   3) Connect Antenna to MKRNB1500 - It seems the modem will not respond to AT commands until it sees an antenna
//   4) Connect Computer to MKR board
//   5) Load Sketch Examples/MKRNB/Tools/SerialSARAPassthrough and run AT command: ATI9 to get version L0.0.00.00.05.06,A.02.00 
//                                                                                 AT+GMM to get model SARA-R410M-02B
//   6) Unplub USB from MKR board, Connect MKR board to programming board, Connect computer to USB of programming board.
//   7) Launch Windows Computer Management and make sure under Ports (COM & LPT) you see the Qualcomm HS-USB device. Make note of the COM port number.
//   8) Launch EasyFlash 13.03.1.2
//         Set path to Firmware .dor file, SARA-R4, ComPort, 115200 Speed
//   9) Press Start
//   10) When done, unplug USB and remove board from programming board. 
//   11) Connect Computer to MKR USB and run AT commands again to verify firmware version.
//       After Firmware update ATI9   = L0.0.00.00.05.12,A.02.21
//                             AT+GMR = L0.0.00.00.05.12 [Mar 09 2022 17:00:00]
//                             AT+GMM = SARA-R410M-02B
// 

// ======================================================================================================================
// General Information and Notes
// ======================================================================================================================
// How to generate a .bin file in Arduino IDE for direct upload via command line
//   SEE https://randomnerdtutorials.com/bin-binary-files-sketch-arduino-ide/
// How to save and upload Arduino BINHEX (.bin) files
//   SEE https://www.youtube.com/watch?v=Lwpxrq9v1Yw
//
// Example Upload Command on MacOS 
// /Users/rjbubon/Library/Arduino15/packages/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=cu.usbmodem14301 -U true -i -e -w -v /var/folders/xq/crjv6t3d6rj7p06zg0nmnsrw0005dl/T/arduino_build_556421/3D-PAWS_MKRNB1500-FG.ino.bin -R 

// Where is stuff stored in Arduino
//   https://support.arduino.cc/hc/en-us/articles/4415103213714-Find-sketches-libraries-board-cores-and-other-files-on-your-computer
//   SEE /Users/rjbubon/Library/Arduino15/packages/arduino/hardware/samd/1.8.13/variants/mkrnb1500/variant.cpp

// SIM Card Information
//    Fast.t-mobile.com (for LTE devices)
//    epc.tmobile.com (for non-LTE devices)
//
//    Twillio - NDSU is using this
//       https://www.twilio.com/iot/super-sim-card

// Over the Air Update Notes / Reference
// Over-the-air updates are useful to upload new code to your ESP32 board when it is not easily accessible. 
// The OTA Web Updater code creates a web server that you can access to upload new code to your ESP32 board 
// using a web browser on your local network.
//   https://randomnerdtutorials.com/esp32-over-the-air-ota-programming/

 /*
  * ======================================================================================================================
  * Modification to Arduino_ConnectionHandler
  *   The ConnectionHandler method is called passing unset parameters. After reding config file we have the paramteres
  *   The below adds a function to reset the private variables to those set by the config file.
  * ======================================================================================================================
  */
/*
  Arduino/libraries/Arduino_ConnectionHandler/src
  
  Arduino_GSMConnectionHandler.h
    public:
    void GSMResetVariables(char *pin, char *apn, char *login, char *pass); // Added by ICDP

  Arduino_GSMConnectionHandler.cpp
    PUBLIC MEMBER FUNCTIONS
    // Added by ICDP
    void GSMConnectionHandler::GSMResetVariables(char *pin, char *apn, char *login, char *pass)
    {
      Debug.print(DBG_INFO, F("GSMResetVariables(START)"));
      _pin   = pin;
      _apn   = apn;
      _login = login;
      _pass  = pass;
      Debug.print(DBG_INFO, _pin);
      Debug.print(DBG_INFO, _apn);
      Debug.print(DBG_INFO, _login);
      Debug.print(DBG_INFO, _pass);
      Debug.print(DBG_INFO, F("GSMResetVariables(END)"));
    }
    // Added by ICDP

  Arduino_NBConnectionHandler.h
    public:
    void NBResetVariables(char *pin, char *apn, char *login, char *pass); // Added by ICDP

  Arduino_NBConnectionHandler.cpp
    PUBLIC MEMBER FUNCTIONS
    // Added by ICDP
    void NBConnectionHandler::NBResetVariables(char *pin, char *apn, char *login, char *pass)
    {
      Debug.print(DBG_INFO, F("NBResetVariables(START)"));
      _pin   = pin;
      _apn   = apn;
      _login = login;
      _pass  = pass;
      Debug.print(DBG_INFO, _pin);
      Debug.print(DBG_INFO, _apn);
      Debug.print(DBG_INFO, _login);
      Debug.print(DBG_INFO, _pass);
      Debug.print(DBG_INFO, F("NBResetVariables(END)"));
    }
    // Added by ICDP
 */
 
 /*
  * ======================================================================================================================
  * Adding SPI1 and Wire1 to MKR NB 1500
  * 
  * https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports?view=all
  * 
  * SEE /Users/rjbubon/Library/Arduino15/packages/arduino/hardware/samd/1.8.13/variants/mkrnb1500/variant.cpp
  * 
  * What is in use by default 
  *   SERCOM0   =  Unused
  *   SERCOM1   =  SPI        =  MOSI,PA16,1/00  SCK,PA17,1/01  MISO,PA19,1/03
  *   SERCOM2   =  Wire i2c   =  SDA,PA08,2/00   SDC,PA09,2/01
  *   SERCOM3   =  Unused 
  *   SERCOM4   =  SerialSARA =  GSM_TX,PA12,4/00  GSM_RX,PA13,4/01 GSM_RTS,PA14,4/02  GSM_CTS,PA15,4/03
  *   SERCOM5   =  Serial1    =  RX,PB23.5/03    TX,PB22,5/02
  *
  *   SAMD21 PIN Mappings in Datasheet Page 33 
  *   Table 7-1. PORT Function Multiplexing for SAM D21 A/B/C/D Variant Devices and SAM DA1 A/B Variant Devices
  *   SERCOM 0 & 3 are open
  *   
  *      SDA,PA08   0/00   2/00  <<<<<<<<<  SERCOM0/00 IN USE
  *      SCL,PA09   0/01   2/01  <<<<<<<<<  SERCOM0/01 IN USE
  *      D2,PA10    0/02   2/02  <<<<<<<<<  SERCOM0/02
  *      D3,PA11    0/03   2/03  <<<<<<<<<  SERCOM0/03
  *      A3,PA04           0/00  <<<<<<<<<  SERCOM0/00 ALT      
  *      A4,PA05           0/01  <<<<<<<<<  SERCOM0/01 ALT
  *      A5,PA06           0/02  <<<<<<<<<  SERCOM0/02 ALT
  *      A6,PA07           0/03  <<<<<<<<<  SERCOM0/03 ALT 
  *      
  *   SPIClass SPI (&PERIPH_SPI,  PIN_SPI_MISO,  PIN_SPI_SCK,  PIN_SPI_MOSI,  PAD_SPI_TX,  PAD_SPI_RX);
  *   
  *   SPI PAD Names
  *        SEE: /Users/rjbubon/Library/Arduino15/packages/arduino/hardware/samd/1.8.13/cores/arduino/SERCOM.h
  *   
  *   SPI_PAD_0_SCK_1 means MOSI is on SERCOMn.0 and SCK is on SERCOMn.1
  *   SPI_PAD_2_SCK_3 means MOSI is on SERCOMn.2 and SCK is on SERCOMn.3
  *   SPI_PAD_3_SCK_1 means MOSI is on SERCOMn.3 and SCK is on SERCOMn.1
  *   SPI_PAD_0_SCK_3 means MOSI is on SERCOMn.0 and SCK is on SERCOMn.3
  *   
  *   SERCOM_RX_PAD_0 means MISO on SERCOMn.0
  *   SERCOM_RX_PAD_1 means MISO on SERCOMn.1
  *   SERCOM_RX_PAD_2 means MISO on SERCOMn.2
  *   SERCOM_RX_PAD_3 means MISO on SERCOMn.3
  *   
  *   Set up Muxing names to PADs
  *     MISO A3 is on PAD 0   so we select SERCOM_RX_PAD_0
  *   
  *     MOSI D2 is on PAD 2   so we select SPI_PAD_2_SCK_3 which means MOSI is on SERCOMn.2 and SCK is on SERCOMn.3
  *     CLK  D3 is on PAD 3
  *   
  *   SPIClass SPI (&PERIPH_SPI,  PIN_SPI_MISO,  PIN_SPI_SCK,  PIN_SPI_MOSI,  PAD_SPI_TX,      PAD_SPI_RX)                            
  *                               A3             D3            D2             SPI_PAD_2_SCK_3  SERCOM_RX_PAD_0                                                                                                                                          
  * ======================================================================================================================
  */
  
/*
 * ======================================================================================================================
 * Modify the variant filese to add SPI1.  If we reload the board package, we loose these changes
 * We modifing the variant files because there is a SPIClassSAMD we must call if we are doing it from arduino sketch.
 * In stead of just plain SPIClass. We also be modifing RF9X-RK-SPI1's files to add this call. I still don't think 
 * we are getting defined and connected to SPI1 this way.
 * ======================================================================================================================
 */
/* 
 SEE Directory /Users/rjbubon/Library/Arduino15/packages/arduino/hardware/samd/1.8.13/variants/mkrnb1500

#diff variant.h.org variant.h
101c101
< #define SPI_INTERFACES_COUNT 1
---
> #define SPI_INTERFACES_COUNT 2
116a117,127
> // SPI1
> #define PIN_SPI1_MOSI    (2u)  // D2
> #define PIN_SPI1_SCK     (3u)  // D3
> #define PIN_SPI1_MISO    (18u) // A3
> #define PERIPH_SPI1   sercom0
> #define PAD_SPI1_TX   SPI_PAD_2_SCK_3
> #define PAD_SPI1_RX   SERCOM_RX_PAD_0
> static const uint8_t MOSI1 = PIN_SPI1_MOSI;
> static const uint8_t MISO1 = PIN_SPI1_MISO;
> static const uint8_t SCK1  = PIN_SPI1_SCK;
>
119c130
< #define WIRE_INTERFACES_COUNT 1
---
> #define WIRE_INTERFACES_COUNT 2
128a140,147
> // Wire1
> #define PIN_WIRE1_SDA        (0u)
> #define PIN_WIRE1_SCL        (1u)
> #define PERIPH_WIRE1         sercom3
> #define WIRE_IT_HANDLER1     SERCOM3_Handler
> static const uint8_t SDA1 = PIN_WIRE1_SDA;
> static const uint8_t SCL1 = PIN_WIRE1_SCL;


#diff variant.cpp.org variant.cpp
29a30
>  |            |       Wire1      |        |                 |        |     |     |     |     |         |         |        |        |          |          |
31a33,34
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
>  |            |       SPI1       |        |                 |        |     |     |     |     |         |         |        |        |          |          |
33a37
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
40,43c44,49
<   { PORTA, 22, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM4_CH0,   TC4_CH0,      EXTERNAL_INT_6    },
<   { PORTA, 23, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM4_CH1,   TC4_CH1,      EXTERNAL_INT_7    },
<   { PORTA, 10, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel18,  PWM1_CH0,   TCC1_CH0,     EXTERNAL_INT_NONE },
<   { PORTA, 11, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel19,  PWM1_CH1,   TCC1_CH1,     EXTERNAL_INT_NONE },
---
>   { PORTA, 22, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM4_CH0,   TC4_CH0,      EXTERNAL_INT_6    }, // SDA1: SERCOM3/PAD[0]
>   { PORTA, 23, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM4_CH1,   TC4_CH1,      EXTERNAL_INT_7    }, // CLK1: SERCOM3/PAD[1]
>
>   { PORTA, 10, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel18,  PWM1_CH0,   TCC1_CH0,     EXTERNAL_INT_NONE }, // MOSI1: SERCOM0/PAD[2]
>   { PORTA, 11, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel19,  PWM1_CH1,   TCC1_CH1,     EXTERNAL_INT_NONE }, // CLK1:  SERCOM0/PAD[3]
>
91a98,99
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
>  |            |       SPI1       |        |                 |        |     |     |     |     |         |         |        |        |          |          |
92a101
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
101c110,112
<   { PORTA,  4, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel4,   PWM0_CH0,   TCC0_CH0,     EXTERNAL_INT_NONE },
---
>
>   { PORTA,  4, PIO_SERCOM_ALT,(PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER  ), ADC_Channel4,   PWM0_CH0,   TCC0_CH0,     EXTERNAL_INT_NONE }, // MISO1: SERCOM0/PAD[0]
>
*/

/*
 API Logging Script
 <?php
if (preg_match ("/^MYAPIKEY\,/", $_SERVER['QUERY_STRING'])) {
  list($APIKEY, $rest) = preg_split ("/,/", $_SERVER['QUERY_STRING'], 2);
  $out = sprintf("%s,%s\n", $_SERVER['REQUEST_TIME'], $rest);
  file_put_contents("ldl.txt", $out, FILE_APPEND);
}
*/

/*
void AlarmHandlerReboot_V2() { 
  // Reset modem via modem reset pin
  unsigned long c1=0x02FFFFFF, c2=0;
  digitalWrite(SARA_RESETN, HIGH); // Datasheet says to hold the reset high for 10s
  // Below creates around 12 to 13 seconds of delay
  c1=0x04FFFFFF, c2=0;
  while (c1-- > 0) {
    c2++; // Give the loop something to do to slow the loop down
  }
  digitalWrite(SARA_RESETN, LOW);

  
  // Toggle pin to reboot relay
  digitalWrite(REBOOT_PIN, HIGH);
  
  //
  // If Relay fails then fall through here and try a restart
  //
  
  // Below creates delay
  c1=0x02FFFFFF; c2=0;  
  while (c1-- > 0) {
    c2++; // Give the loop something to do to slow the loop down
  }
  digitalWrite(REBOOT_PIN, LOW);
  
  wdt_init ( WDT_CONFIG_PER_4K ); // Set watchdog timer and restart when expires.
  while(true) ;
  
  // We will never reach this!
}
 */

 /*
  * Alarm testing
  *   time_t tts = stc.getEpoch();
  tm *dt = gmtime(&tts);
  sprintf (Buffer32Bytes, "%d:%d.%d",dt->tm_hour, dt->tm_min, dt->tm_sec);
  Output (Buffer32Bytes);
  stc.setAlarmEpoch(tts+60);
  stc.attachInterrupt(AlarmHandlerReboot);
  stc.enableAlarm(stc.MATCH_HHMMSS);  
  
  while (true) delay(1000);
  stc.disableAlarm(); // Disable Alarm
*/
  */

 
