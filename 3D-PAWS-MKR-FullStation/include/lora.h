/*
 * ======================================================================================================================
 *  lora.h - LoRa Definations
 * ======================================================================================================================
 */
#include <RH_RF95.h>

/*
 * ======================================================================================================================
 *  RF9X-RK-SPI1            https://github.com/rickkas7/RF9X-RK - 0.2.0 - Modified RadioHead LoRa for SPI1                       
 *  AES-master              https://github.com/spaniakos/AES - 0.0.1 - Modified to make it compile
 *  CryptoLW-RK             https://github.com/rickkas7/CryptoLW-RK - 0.2.0
 *  
 *  MISO  to A3 MISO
 *  MOSI  to D2 MOSI
 *  SCK   to D3 SCK
 *  CS    to D5
 *  RESET to A5
 *  DIO0  to D7 IRQ
 *  
 * Arduino/libraries/AES-master/AES_config.h  Modified to fix below problem.
 *     Arduino/libraries/AES-master/AES_config.h:43:18: fatal error: pgmspace.h: No such file or directory
 *     #include <pgmspace.h>
 *                ^~~~~~~~~~~~
 * diff AES_config.h.org AES_config.h
 *     40c40
 *     <     #if (defined(__AVR__))
 *     ---
 *     >     #if (true || defined(__AVR__))
 *     41a42
 *     >  #define printf_P printf
 *  
 *  https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-spi
 *  https://forum.arduino.cc/t/using-multiple-spi-buses-on-the-zero/635526
 *  https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/using-the-rfm-9x-radio
 *  http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
 *  
 *  rsync -av Crypto/src/RadioHead/RHutil ~/Arduino/libraries/RF9X-RK-SPI1/src/
 *  rsync-av BlockCipher.* ../../libraries/RF9X-RK-SPI1/src/
 */

 // SPI1 [D2 = MOSI, D3 = CLK, A3 = MISO]
#define LORA_IRQ_PIN 7   // D7 Interrupt
#define LORA_SS      5   // D5 Slave Select Pin
#define LORA_RESET   A4  // A4 Used by lora_initialize()  Was A5

#define LORA_RESET_NOACTIVITY 30 // 30 minutes

#define LORA_RELAY_MSGCNT     32  // Set to the number of LoRa RS devices this station will be supporting
#define LORA_RELAY_MSG_LENGTH 256

typedef struct {
  bool          need2log;
  int           message_type;
  char          message[LORA_RELAY_MSG_LENGTH];
} LORA_MSG_RELAY_STR;

// Extern variables
extern uint8_t  AES_KEY[16];
extern unsigned long long int AES_MYIV;
extern bool LORA_exists;
extern RH_RF95 rf95;
extern const char *relay_msgtypes[];
extern LORA_MSG_RELAY_STR lora_msg_relay[LORA_RELAY_MSGCNT];

// Function prototypes
void lora_device_initialize();
void lora_initialize();
void lora_msg_check();
void lora_msg_poll();
void lora_relay_msg_free(LORA_MSG_RELAY_STR *m);
bool lora_relay_need2log();
int lora_relay_need2log_idx();
int lora_relay_build_JSON();
