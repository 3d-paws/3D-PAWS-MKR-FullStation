/*
 * ======================================================================================================================
 *  LoRa.h - LoRa Functions
 * ======================================================================================================================
 */

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

#include <AES.h>
#include <RH_RF95.h>
#include <wiring_private.h>

// Prototyping functions to aviod compile function unknown issue.
void SD_NeedToSend_Add(char *observation);
 
 /*
 * ======================================================================================================================
 *  Define who we are on LoRa network and who we are seding as 
 * ======================================================================================================================
 */
// #define LORA_RECEIVING_STATION_ID  1   // This unit's LoRa ID for Receiving Messages

/*
 * ======================================================================================================================
 *  Singleton instance of the radio driver
 *
 *  The RH_RF95 driver uses interrupts to react to events in the RFM module,
 *  such as the reception of a new packet, or the completion of transmission
 *  of a packet.  The RH_RF95 driver interrupt service routine reads status from
 *  and writes data to the the RFM module via the SPI interface. It is very
 *  important therefore, that if you are using the RH_RF95 driver with another
 *  SPI based deviced, that you disable interrupts while you transfer data to
 *  and from that other device.  Use cli() to disable interrupts and sei() to
 *  reenable them.
 * 
 *  Note: We are dedicating SPI1 to the RH_RF95 driver. No need to mask interrupts.
 * ======================================================================================================================
 */

// SPI1 [D2 = MOSI, D3 = CLK, A3 = MISO]
#define LORA_IRQ_PIN 7   // D7 Interrupt
#define LORA_SS      5   // D5 Slave Select Pin
#define LORA_RESET   A4  // A4 Used by lora_initialize()  Was A5
#define LORA_RESET_NOACTIVITY 30 // 30 minutes

RH_RF95 rf95(LORA_SS, LORA_IRQ_PIN, hardware_spi); // SPI1

bool LORA_exists = false;
uint64_t lora_alarm_timer;   // Must get a LoRa mesage by the time set here, else we call lora_initialize()

/*
 * =======================================================================================================================
 *  AES Encryption - These need to be changed here and on the RaspberryPi (They need to match)
 * =======================================================================================================================
 */
// Private Key
uint8_t  AES_KEY[16]; // Now filled in from CONFIG.TXT


// Initialization Vector must be and always will be 128 bits (16 bytes.)
// The real iv is actually my_iv repeated twice EX: 01234567 = 0123456701234567
// 01234567 - > 0x12D687 = 0x00 0x00 0x00 0x00 0x00 0x12 0xD6 0x87
// unsigned long long int my_iv = 01234567;

unsigned long long int AES_MYIV;

/*
 * =======================================================================================================================
 *  AES Instance
 * =======================================================================================================================
 */
AES aes;

/*
 * ======================================================================================================================
 *  LoRa Relay Remote Device Message Storage - Remember we only have 32K VRAM can not store too many messages
 * ======================================================================================================================
 */
#define LORA_RELAY_MSGCNT     32  // Set to the number of LoRa RS devices this station will be supporting
#define LORA_RELAY_MSG_LENGTH 256 
const char *relay_msgtypes[] = {"UNKN", "INFO", "LR"}; // Particle Message Types being received for relay
typedef struct {
  bool          need2log;
  int           message_type;
  char          message[LORA_RELAY_MSG_LENGTH];
} LORA_MSG_RELAY_STR;
LORA_MSG_RELAY_STR lora_msg_relay[LORA_RELAY_MSGCNT];


/* 
 *=======================================================================================================================
 * lora_relay_msg_free()
 *=======================================================================================================================
 */
void lora_relay_msg_free(LORA_MSG_RELAY_STR *m) {
  m->need2log = false;
  m->message_type = 0;
  memset (m->message, 0, LORA_RELAY_MSG_LENGTH);
}


/* 
 *=======================================================================================================================
 * lora_relay_need2log() - Return true if we have a relay that needs to be logged
 *=======================================================================================================================
 */
bool lora_relay_need2log() {
  for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
    if (lora_msg_relay[i].need2log) {
      return (true);
    }
  }
  return(false);
}

/* 
 *=======================================================================================================================
 * lora_relay_notinuse() - return first open spot or -1 if full
 *=======================================================================================================================
 */
int lora_relay_notinuse() {
  for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
    if (!lora_msg_relay[i].need2log) {
      return (i);
    }
  }
  return(-1);
}

/* 
 *=======================================================================================================================
 * lora_relay_need2log_idx() - return first need2log spot or -1 if none
 *=======================================================================================================================
 */
int lora_relay_need2log_idx() {
  for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
    if (lora_msg_relay[i].need2log) {
      return (i);
    }
  }
  return(-1);
}

/* 
 *=======================================================================================================================
 * lora_msgs_to_n2s()
 *=======================================================================================================================
 */
void lora_msgs_to_n2s() {
  if (LORA_exists) {
    LORA_MSG_RELAY_STR *m;

    for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
      m = &lora_msg_relay[i];
      if (m->need2log) {
        // sprintf (msgbuf, "%s,%s", m->message, relay_msgtypes[m->message_type]); // A Particle thing
        sprintf (msgbuf, "%s", m->message);
        SD_NeedToSend_Add(msgbuf); // Save to N2F File 
        lora_relay_msg_free(m);
        sprintf (Buffer32Bytes, "LoRaMsg[%d]->N2S", i);
        Output (Buffer32Bytes);
      }
    }
  }
}

/*
 * ======================================================================================================================
 * OBS_Relay_Build_JSON() - Copy JSON observation to obsbuf, remove from relay structure
 *                          Return the message relay type we are preparing
 * ======================================================================================================================
 */
int OBS_Relay_Build_JSON() {
  LORA_MSG_RELAY_STR *m;
  int relay_type = 0;

  memset(msgbuf, 0, sizeof(msgbuf));

  // Locate message we need to log
  int i = lora_relay_need2log_idx();
  if (i >= 0) {
    m = &lora_msg_relay[i];
    strncpy (obsbuf, m->message, LORA_RELAY_MSG_LENGTH-1); // minus 1 so last byte in array will always be null
    relay_type = m->message_type;
    lora_relay_msg_free(m);
  }
  return (relay_type);
}


/* 
 *=======================================================================================================================
 * lora_device_initialize()
 *=======================================================================================================================
 */
void lora_device_initialize() {
  if (LORA_exists) {
    // Init LoRa Relay Message structure
    for (int i=0; i< LORA_RELAY_MSGCNT; i++) {
      lora_relay_msg_free(&lora_msg_relay[i]);
    }
  }
}
/* 
 *=======================================================================================================================
 * lora_cf_validate() - Validate LoRa variables from CONFIG.TXT
 *=======================================================================================================================
 */
bool lora_cf_validate() {
  if (cf_aes_pkey == NULL) {
    Output (F("AES PKEY !SET"));
    return (false);
  }
  else if (strlen (cf_aes_pkey) != 16) {
    Output (F("AES PKEY !16 Bytes"));
    return (false);    
  }
  else if (cf_aes_myiv == 0) {
    Output (F("AES MYIV !SET"));
    return (false);
  }
  else if ((cf_lora_txpwr<5) || (cf_lora_txpwr>23)) {
    Output (F("LORA PWR ERR"));
    return (false);
  }
  else if ((cf_lora_freq!=915) && (cf_lora_freq!=866) && (cf_lora_freq!=433)) {
    Output (F("LORA FREQ ERR"));
    return (false);        
  }
  else if ((cf_lora_unitid<0) || (cf_lora_unitid>254)) {
    Output (F("LORA ADDR ERR"));
    return (false);
  }
  else { 
    memcpy ((char *)AES_KEY, cf_aes_pkey, 16);
    sprintf(msgbuf, "AES_KEY[%s]", cf_aes_pkey); Output (msgbuf);

    AES_MYIV=cf_aes_myiv;
    sprintf(msgbuf, "AES_MYIV[%llu]", AES_MYIV); Output (msgbuf);

    Output (F("LORA CFV OK"));
    return (true);
  }
}

/* 
 *=======================================================================================================================
 * lora_initialize()
 *=======================================================================================================================
 */
void lora_initialize() {

  // Validate LoRa variables from CONFIG.TXT
  if (lora_cf_validate()) {
    // Output (F("LORA Init"));
    pinMode(LORA_RESET, OUTPUT);
    // manual reset
    digitalWrite(LORA_RESET, LOW);
    delay(100);
    digitalWrite(LORA_RESET, HIGH);
    delay(100);

    if (rf95.init()) {
      // The default transmitter power is 13dBm, using PA_BOOST.
      // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
      // you can set transmitter powers from 5 to 23 dBm:
      rf95.setTxPower(cf_lora_txpwr, false);

      // If you are using Modtronix inAir4 or inAir9,or any other module which uses the
      // transmitter RFO pins and not the PA_BOOST pins
      // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true. 
      // Failure to do that will result in extremely low transmit powers.
      //  driver.setTxPower(14, true);

      rf95.setFrequency(cf_lora_freq);

      // If we need to send something
      rf95.setThisAddress(cf_lora_unitid);
      rf95.setHeaderFrom(cf_lora_unitid);

      // Be sure to grab all node packet 
      rf95.setPromiscuous(true);

      // We're ready to listen for incoming message
      rf95.setModeRx();

      LORA_exists=true;

      Output (F("LORA OK"));
    }
    else {
      SystemStatusBits |= SSB_LORA;  // Turn On Bit
    
      Output (F("LORA NF"));
    }
  }
  else {
    Output (F("LORA ERR CF NF"));
  }
  // Even if LoRa not found set the timer for time we call initialize after setup()
  lora_alarm_timer = millis() + (LORA_RESET_NOACTIVITY * 60000);  // Minutes * 60 seconds
}

/* 
 *=======================================================================================================================
 * lora_relay_msg() -LoRa Rain and Soil Remote Sensors we relay their messages to Particle
 *    
 * Relay Message Format
 *   NCS    Length (N) and Checksum (CS)
 *   XX,    Lora Message Type IF(INFO), LR (LoRa Relay)
 *   INT,   Station ID
 *   INT,   Message Counter
 *   OBS    JSON Observation
 *=======================================================================================================================
 */
void lora_relay_msg(char *obs) {
  LORA_MSG_RELAY_STR *m;

  int message_type = 0;
  int unit_id = 0;
  unsigned int message_counter = 0;
  char *message;
  char *p;

  // There are used with Particle code to set the message ID type
  if ((obs[0] == 'I') && (obs[1] == 'F')) {
    message_type = 1;
  }
  else if ((obs[0] == 'L') && (obs[1] == 'R')) {
    message_type = 2; 
  }
  else {
    sprintf (Buffer32Bytes, "LORA Relay %c%c Unkn", obs[0], obs[1]);
    Output (Buffer32Bytes);
    return;
  }

  p = &obs[2]; // Start after message type 
  unit_id = atoi (strtok_r(p, ",", &p));
  message_counter = atoi (strtok_r(p, ",", &p));
  message = p;

  sprintf (Buffer32Bytes, "LORA TYPE:%s ID:%d CNT:%d", relay_msgtypes[message_type], unit_id, message_counter);
  Output (Buffer32Bytes);
  // Output (message);

  // Locate storage index
  int idx = lora_relay_notinuse();

  if (idx == -1) {
    Output (F("LORA Relay NoSpace"));

    // Dump all LoRA messages to N2S
    lora_msgs_to_n2s();  // If No Space - Dump all LoRA messages to N2S, then save new message

    idx = lora_relay_notinuse(); // This better not be -1 after freeing
    if (idx == -1) {
      Output (F("LORA Relay MsgLost"));
      return;
    }
  }

  m = &lora_msg_relay[idx]; // Lets work with a pointer and not the index
  m->need2log = true;
  m->message_type = message_type;
  strncpy (m->message, message, LORA_RELAY_MSG_LENGTH-1); // minus 1 so last byte in array will always be null
  sprintf (Buffer32Bytes, "LORA Relay %s -> Queued:%d", relay_msgtypes[message_type], idx);
  Output (Buffer32Bytes);
}

/* 
 *=======================================================================================================================
 * lora_msg_check()
 *=======================================================================================================================
 */
void lora_msg_check() {

  if (LORA_exists) {
    // Output (F("LoRa Check()"));
    if (rf95.available()) {
      byte iv [N_BLOCK];
    
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN]; // 251 Bytes
      uint8_t len  = sizeof(buf);
      [[maybe_unused]] uint8_t from = rf95.headerFrom();
      [[maybe_unused]] uint8_t to   = rf95.headerTo();
      [[maybe_unused]] uint8_t id   = rf95.headerId();
      [[maybe_unused]] uint8_t flags= rf95.headerFlags();
      [[maybe_unused]] int8_t  rssi = rf95.lastRssi(); 
      uint16_t checksum = 0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t i;
      uint8_t msglen = 0;
      char msg[256];             // Used to hold decrypted lora messages

      memset(buf, 0, RH_RF95_MAX_MESSAGE_LEN);
      memset(msg, 0, RH_RF95_MAX_MESSAGE_LEN+1);
    
      if (rf95.recv(buf, &len)) {
        // memcpy (msg, &buf[3], buf[0]);
        // Output (msg);
        // Serial_write ("LoRa Msg");
     
        aes.iv_inc();
        aes.set_IV(AES_MYIV);
        aes.get_IV(iv);
        aes.do_aes_decrypt(buf, len, (byte *) msg, AES_KEY, 128, iv);
      
        if ( ( msg[3] == 'I' && msg[4] == 'F') ||
             ( msg[3] == 'L' && msg[4] == 'R')) {

          // Get length of what follows
          msglen = msg[0];

          // Compute Checksum
          checksum=0;
          for (i=3; i<msglen; i++) {
            checksum += msg[i];
          }
          byte1 = checksum>>8;
          byte2 = checksum%256;

          // Validate Checksum against sent checksum
          if ((byte1 == msg[1]) && (byte2 == msg[2])) {
            // Make what follows a string
            msg[msglen]=0;

            char *payload = (char*)(msg+3); // After length and 2 checksum bytes

            // Display LoRa Message on Serial Console           
            Serial_write (payload);

            lora_relay_msg (payload);
          }
          else {
            Output (F("LORA CS-ERR"));
          }
        }
      }
      // Received LoRa Signal, Reset alarm
      lora_alarm_timer = millis() + (LORA_RESET_NOACTIVITY * 60000);
    }
    else {
      // Have not received LoRa message in N seconds
      if (millis() >= lora_alarm_timer) {
        Output (F("LORA Init"));
        // Reset and reinitialize LoRa
      
        // lora_reinitialize_count++   // Do a back on how often to reset
        // Need to set some system status bit here too
        lora_initialize();
      }
    }
  }
}

/* 
 *=======================================================================================================================
 * lora_msg_poll() -- Spend 750ms looking for LoRa Messages
 *=======================================================================================================================
 */
void lora_msg_poll() {
  for (int i=0; i<3; i++) {
    lora_msg_check();
    delay (250);
  }
}
