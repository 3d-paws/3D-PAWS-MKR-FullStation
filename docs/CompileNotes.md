# Compile Notes
[←Top](../README.md)<BR>

The code is provided at [Github - https://github.com/3d-paws/3D-PAWS-MKR-FullStation](https://github.com/3d-paws/3D-PAWS-MKR-FullStation)

Libraries are provided and need to be copied to your Arduino's library directory.


## Modification to Library MKRNB - Adding support for Modem Hard Reset to MKR NB 1500.

The Arduino MKR NB 1500 modem reset pin is SARA_RESETN, a GPIO connected to the modem RESET_N pin.

Edit libraries/MKRNB/src/NBModem.h and add the below.
```C
/* ICDP Added */
void hardReset();
```
Edit libraries/MKRNB/src/NBModem.cpp and add the below.
```C
/* ICDP Added */
void NBModem::hardReset()
{
  MODEM.hardReset();
}

Edit libraries/MKRNB/src/Modem.cpp and change hardReset to the below. This fixes the known bug of MKR NB 1500 RESET_N pin has INVERTED LOGIC due to N-channel MOSFET level shifters
```C
void ModemClass::hardReset()
{
  // EMERGENCY hardware reset ONLY - SARA-R410M RESET_N timing
  pinMode(_resetPin, OUTPUT);
  
  // Disable V_INT monitoring first
  setVIntPin(SARA_VINT_OFF);
  
  // CORRECT: Assert reset LOW immediately (>=10s per datasheet)
  digitalWrite(_resetPin, LOW);
  delay(11000);  // 11s > 10s minimum
  
  // Release reset (goes HIGH/inactive)
  digitalWrite(_resetPin, HIGH);
  
  // Wait full boot + AT response capability (~30s total)
  delay(20000);
  
  // Re-detect modem type/baud
  autosense(30000);
}
```

```

## Modification to Library MKRGSM - Adding support for Modem Hard Reset to MKR GSM 1400.

Edit libraries/MKGSM/src/GSMModem.h and add the below.
```C
/* ICDP Added */
void hardReset();
```
Edit libraries/MKGSM/src/GSMModem.cpp and add the below.
```C
/* ICDP Added */
void GSMModem::hardReset()
{
  MODEM.hardReset();
}
```

Edit libraries/MKGSM/src/Modem.h and add the below.
```C
/* ICDP Added */
void hardReset();
```
Edit libraries/MKGSM/src/Modem.cpp and add the below after the existing reset() function.
```C
/* ICDP Added */
int ModemClass::hardReset()
{
  // Hardware pin reset, only use in emergency
  pinMode(_resetPin, OUTPUT);

  // Assert reset (active low on SARA modem)
  digitalWrite(_resetPin, LOW);
  delay(1000); // Hold low long enough for a full reset (datasheet-level value recommended)

  // Deassert reset and let modem boot
  digitalWrite(_resetPin, HIGH);

  // Give the modem time to come back up and start responding to AT
  delay(10000);

  // Re-autosense / check if modem responds again
  if (autosense(10000) == 1) {
    return 1;
  }

  return 0;
}
```

Note: The library "Arduino_ConnectionHandler" includes the "MKRNB" library. If compiling for "MKR NB 1500". If compiling for "MKR GSM 1400" it will include "MKRGSM" library.


### Modifing Board Variants files to support SPI1 for LoRa 
#### mkrnb1500 board

Modified files are located in 3D-PAWS-MKR-FullStation/variants/mkrnb1500<br>
Copy to the Arduino package install for the board Library/Arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrnb1500

#### mkrgsm1400 board
Modified files are located in 3D-PAWS-MKR-FullStation/variants/mkrgsm1400<br>
Copy to the Arduino package install for the board Library/Arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrgsm1400


### Details on adding SPI1 and Wire1 to MKR NB 1500 and will use the modified library called RF9X-RK-SPI1

```C
  https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports?view=all
   
  SEE /Users/rjbubon/Library/Arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrnb1500/variant.cpp
  
  What is in use by default 
    SERCOM0   =  Unused
    SERCOM1   =  SPI        =  MOSI,PA16,1/00  SCK,PA17,1/01  MISO,PA19,1/03
    SERCOM2   =  Wire i2c   =  SDA,PA08,2/00   SDC,PA09,2/01
    SERCOM3   =  Unused 
    SERCOM4   =  SerialSARA =  GSM_TX,PA12,4/00  GSM_RX,PA13,4/01 GSM_RTS,PA14,4/02  GSM_CTS,PA15,4/03
    SERCOM5   =  Serial1    =  RX,PB23.5/03    TX,PB22,5/02
  
  SAMD21 PIN Mappings in Datasheet Page 33 
  Table 7-1. PORT Function Multiplexing for SAM D21 A/B/C/D Variant Devices and SAM DA1 A/B Variant Devices

    SERCOM 0 & 3 are open
     
    SDA,PA08   0/00   2/00  <<<<<<<<<  SERCOM0/00 IN USE
    SCL,PA09   0/01   2/01  <<<<<<<<<  SERCOM0/01 IN USE   Will use the below
    D2,PA10    0/02   2/02  <<<<<<<<<  SERCOM0/02          <<<< MOSI1
    D3,PA11    0/03   2/03  <<<<<<<<<  SERCOM0/03          <<<< CLK1
    A3,PA04           0/00  <<<<<<<<<  SERCOM0/00 ALT      <<<< MISO1   Alternate pin needed since 0/00 and 0/01 are in use
    A4,PA05           0/01  <<<<<<<<<  SERCOM0/01 ALT
    A5,PA06           0/02  <<<<<<<<<  SERCOM0/02 ALT
    A6,PA07           0/03  <<<<<<<<<  SERCOM0/03 ALT 
        
    SPIClass SPI (&PERIPH_SPI,  PIN_SPI_MISO,  PIN_SPI_SCK,  PIN_SPI_MOSI,  PAD_SPI_TX,  PAD_SPI_RX);
     
  SPI PAD Names
    SEE: /Users/rjbubon/Library/Arduino15/packages/arduino/hardware/samd/1.8.14/cores/arduino/SERCOM.h
     
    SPI_PAD_0_SCK_1 means MOSI is on SERCOMn.0 and SCK is on SERCOMn.1
    SPI_PAD_2_SCK_3 means MOSI is on SERCOMn.2 and SCK is on SERCOMn.3
    SPI_PAD_3_SCK_1 means MOSI is on SERCOMn.3 and SCK is on SERCOMn.1
    SPI_PAD_0_SCK_3 means MOSI is on SERCOMn.0 and SCK is on SERCOMn.3
     
    SERCOM_RX_PAD_0 means MISO on SERCOMn.0
    SERCOM_RX_PAD_1 means MISO on SERCOMn.1
    SERCOM_RX_PAD_2 means MISO on SERCOMn.2
    SERCOM_RX_PAD_3 means MISO on SERCOMn.3
     
  Set up Muxing names to PADs
    MISO A3 is on PAD 0   so we select SERCOM_RX_PAD_0
     
    MOSI D2 is on PAD 2   so we select SPI_PAD_2_SCK_3 which means MOSI is on SERCOMn.2 and SCK is on SERCOMn.3
    CLK  D3 is on PAD 3
     
    SPIClass SPI (&PERIPH_SPI,  PIN_SPI_MISO,  PIN_SPI_SCK,  PIN_SPI_MOSI,  PAD_SPI_TX,      PAD_SPI_RX)                            
                                A3             D3            D2             SPI_PAD_2_SCK_3  SERCOM_RX_PAD_0                                                                                                                                          
 ```

### Detail on modifing the variant filese to add SPI1
Note If we reload or update the board package (via Board Manager), we loose these changes.

We modifing the variant files because there is a SPIClassSAMD we must call if we are doing it from arduino sketch.
Instead of just plain SPIClass. We also will be modifing RF9X-RK-SPI1's files to add this call. 

Change directory to the location of the Arduino board installation.

For a MacOS user this is directory /Users/username/Library/Arduino15/packages/arduino/hardware/samd/1.8.14/variants/mkrnb1500

```
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
> #define PAD_SPI1_TX   SPI_PAD_2_SCK_3                  means MOSI is on SERCOMn.2 (D2) and             SCK is on SERCOMn.3 (D3)
> #define PAD_SPI1_RX   SERCOM_RX_PAD_0              means MISO on SERCOMn.0
> static const uint8_t MOSI1 = PIN_SPI1_MOSI;
> static const uint8_t MISO1 = PIN_SPI1_MISO;
> static const uint8_t SCK1  = PIN_SPI1_SCK;

#diff variant.cpp.org variant.cpp
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
>  |            |       SPI1       |        |                 |        |     |     |     |     |         |         |        |        |          |          |
33a36
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
42,43c45,48
<   { PORTA, 10, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel18,  PWM1_CH0,   TCC1_CH0,     EXTERNAL_INT_NONE },
<   { PORTA, 11, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel19,  PWM1_CH1,   TCC1_CH1,     EXTERNAL_INT_NONE },
---
>
>   { PORTA, 10, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel18,  PWM1_CH0,   TCC1_CH0,     EXTERNAL_INT_NONE }, // MOSI1: SERCOM0/PAD[2]
>   { PORTA, 11, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel19,  PWM1_CH1,   TCC1_CH1,     EXTERNAL_INT_NONE }, // CLK1:  SERCOM0/PAD[3]
>
61,62c66,67
<  | 11         | SDA              |  PA08  |                 |   NMI  | *16 |     | X00 |     |   0/00  |  *2/00  | TCC0/0 | TCC1/2 | I2S/SD1  |          |
<  | 12         | SCL              |  PA09  |                 |   09   | *17 |     | X01 |     |   0/01  |  *2/01  | TCC0/1 | TCC1/3 | I2S/MCK0 |          |
---
>  | 11         | SDA              |  PA08  |                 |   NMI  | *16 |     | X00 |     |  *0/00  |   2/00  | TCC0/0 | TCC1/2 | I2S/SD1  |          |
>  | 12         | SCL              |  PA09  |                 |   09   | *17 |     | X01 |     |  *0/01  |   2/01  | TCC0/1 | TCC1/3 | I2S/MCK0 |          |
91a97,98
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
>  |            |       SPI1       |        |                 |        |     |     |     |     |         |         |        |        |          |          |
92a100
>  +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
101c109,111
<   { PORTA,  4, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), ADC_Channel4,   PWM0_CH0,   TCC0_CH0,     EXTERNAL_INT_NONE },
---
>
>   { PORTA,  4, PIO_SERCOM_ALT,(PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER  ), ADC_Channel4,   PWM0_CH0,   TCC0_CH0,     EXTERNAL_INT_NONE }, // MISO1: SERCOM0/PAD[0]
>
133,134c143,144
<  | 26         |                  |  PA12  | GSM_TX          |   12   |     |     |     |     |   2/00  |  *4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  |
<  | 27         |                  |  PA13  | GSM_RX          |   13   |     |     |     |     |   2/01  |  *4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  |
---
>  | 26         |                  |  PA12  | GSM_TX          |   12   |     |     |     |     |  *2/00  |   4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  |
>  | 27         |                  |  PA13  | GSM_RX          |   13   |     |     |     |     |  *2/01  |   4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  |
136c146
<  | 29         |                  |  PA15  | GSM_CTS         |   15   |     |     |     |     |   2/03  |  *4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 |
---
>  | 29         |                  |  PA15  | GSM_CTS         |   15   |     |     |     |     |  *2/03  |   4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 |
*/

```

### Modified RF9X-RK library supporting SPI1
The github download whill have a libraries directory. This will have library RF9X-RK-SPI1. When you copy this to where the Arduino looks for libraries when compiling. Look for library RF9X-RK in that directory. If it exisits there, then move it out. You can not have both RF9X-RK-SPI1 and RF9X-RK at the same time. The compile will complain and fail.

The library RF9X-RK-SPI1 has been modifed to support SPI1. Changes were made to RF9X-RK-SPI1/src/RHHardwareSPI.cpp 

### Example Uploading via command line
- How to generate a .bin file in Arduino IDE for direct upload via command line. https://randomnerdtutorials.com/bin-binary-files-sketch-arduino-ide/
- How to save and upload Arduino BINHEX (.bin) files. https://www.youtube.com/watch?v=Lwpxrq9v1Yw

Example MacOS: /Users/username/Library/Arduino15/packages/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=cu.usbmodem14301 -U true -i -e -w -v 'path to binary file'/3D-PAWS_MKRNB1500-FG.ino.bin -R 

### Misc Notes:
- The software will determine if you are using a Arduino MKR NB 1500 or a Arduino MKR GSM 1400 board.
- To put the MKR in firmware down load mode, press the button on the board once quickly.
- Once board is running 3D-PAWS code, there after you can place the firmware file in a file called UPDATE.BIN on the SD card. Then reboot. After update, the UPDATE.BIN will be removed from the SD card.
