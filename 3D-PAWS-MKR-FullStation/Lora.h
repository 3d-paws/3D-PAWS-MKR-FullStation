/*
 * ======================================================================================================================
 *  Lora.h - LoRa Interrupt Routine use SPI, so need to be dedicated bus.
 *  
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
 * ======================================================================================================================
 */


#include <AES.h>
#include <RH_RF95.h>
#include <wiring_private.h>

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
#define LORA_RESET   A5  // A5 Used by lora_initialize()
#define LORA_RESET_NOACTIVITY 30 // 30 minutes

RH_RF95 rf95(LORA_SS, LORA_IRQ_PIN, hardware_spi); // SPI1

bool LORA_exists = false;
uint64_t lora_alarm_timer;   // Must get a LoRa mesage by the time set here, else we call lora_initialize()

/*
 * =======================================================================================================================
 *  AES Encryption - These need to be changed here and on the RaspberryPi (They need to match)
 *  
 *  
 * =======================================================================================================================
 */
// Private Key
byte *key = (unsigned char*) cf_aes_pkey;

// Initialization Vector must be and always will be 128 bits (16 bytes.)
// The real iv is actually my_iv repeated twice EX: 01234567 = 0123456701234567
// 56495141 -> 0x035E0C25 = 0x00 0x00 0x00 0x00 0x03 0x5E 0x0C 0x25
/*
 * In cryptography, an Initialization Vector (IV) is a fixed-size input to a cryptographic primitive that is typically 
 * required to be random or pseudorandom. It's used in certain modes of operation, like CBC (Cipher Block Chaining), 
 * to ensure that repeated plaintext blocks do not encrypt to the same ciphertext block. The IV should be unique for 
 * every encryption performed with the same key.
 */
unsigned long long int my_iv = stringToLongLong(cf_aes_myiv);


/*
 * =======================================================================================================================
 *  AES Instance
 * =======================================================================================================================
 */
AES aes;

/*
 * ======================================================================================================================
 *  LoRa Connected Rain Gauges and Soil Moisture
 *  - If LoRa device is rain gauge and we have not logged last observation sum together the reading and update ts
 *    Also sum the rgds time 
 *  - Each LoRa RS device will get an index in LORA_RS_OBS_STR. Will need to search is structure for matching unit_id
 *  - These observations will be save to N2S but not the local /OBS file. The LoRa device has it's own SD card.
 * ======================================================================================================================
 */
#define NUMBER_RS_LORA_DEVICES    10  // Set to the number of LoRa RS devices this station will be supporting
#define LORA_SG_INFO_MSG_LENGTH   64
#define SM_PROBES                 3
#define SM_BMX_SENSORS            2
typedef struct {              // LoRa Type 2 Messages
  DateTime      ts;
  unsigned int  message_counter;
  char          message[LORA_SG_INFO_MSG_LENGTH];
  // voltage and health bits get logged in main structure
} LORA_RS_INFO_STR;

typedef struct {
  DateTime      ts;
  int           message_type;
  int           unit_id;
  unsigned int  message_counter;
  float         rg;
  unsigned int  rgds;
  int           sm[SM_PROBES];
  float         st[SM_PROBES];
  float         p[SM_BMX_SENSORS];
  float         t[SM_BMX_SENSORS];
  float         h[SM_BMX_SENSORS];
  float         voltage;
  unsigned int  hth;
  bool          need2log;
  LORA_RS_INFO_STR info;
} LORA_RS_OBS_STR;
LORA_RS_OBS_STR lora_rs[NUMBER_RS_LORA_DEVICES];

/*
 * ======================================================================================================================
 *  LoRa Distance Message
 * ======================================================================================================================
 */
#define LORA_DISTANCE_MESSAGE_LEN 256
char LoRaDistanceMessage[LORA_DISTANCE_MESSAGE_LEN];
bool LoRaDistanceMessageSet=false;


/*
 * ======================================================================================================================
 *  LoRa Connected Stream Gauge - Only 1 gauge will ever be connected - (Last dying words!) 
 *  - If we have not logged last observation, then this gets overwritten
 * ======================================================================================================================
 */
#define SG_BMX_SENSORS 2
typedef struct {                // LoRa Type 1 and 3 messages
  DateTime      ts;
  int           message_type;
  int           unit_id;
  unsigned int  message_counter;
  int           stream_gauge;
  float         p[SG_BMX_SENSORS];
  float         t[SG_BMX_SENSORS];
  float         h[SG_BMX_SENSORS];
  float         voltage;
  unsigned int  hth;
  bool          need2log;
} LORA_SG_OBS_STR;
LORA_SG_OBS_STR lora_sg;

#define LORA_SG_INFO_MSG_LENGTH 64
typedef struct {              // LoRa Type 2 Messages
  DateTime      ts;
  int           unit_id;
  unsigned int  message_counter;
  char          message[LORA_SG_INFO_MSG_LENGTH];
  float         voltage;
  unsigned int  hth;
} LORA_SG_INFO_STR;
LORA_SG_INFO_STR lora_sg_info;

/* 
 *=======================================================================================================================
 * lora_sg_msg_clear()
 *=======================================================================================================================
 */
void lora_sg_msg_clear() {
  if (LORA_exists) {
    lora_sg.unit_id = 0;
    lora_sg.message_type = 0;
    lora_sg.message_counter = 0;
    lora_sg.stream_gauge = 0;
    for (int i=0; i<2; i++) {
      lora_sg.t[i] = 0.0;
      lora_sg.p[i] = 0.0;
      lora_sg.h[i] = 0.0;
    }
    lora_sg.voltage = 0.0;
    lora_sg.hth= 0;
    lora_sg.need2log = false;
  }
}

/* 
 *=======================================================================================================================
 * lora_sg_info_msg_clear()
 *=======================================================================================================================
 */
void lora_sg_info_msg_clear() {
  if (LORA_exists) {
    lora_sg_info.unit_id = 0;
    lora_sg_info.message_counter = 0;
    memset (lora_sg_info.message, 0, LORA_SG_INFO_MSG_LENGTH);
    lora_sg_info.voltage = 0.0;
    lora_sg_info.hth = 0; 
  }
}

/* 
 *=======================================================================================================================
 * lora_rs_msg_init()
 *=======================================================================================================================
 */
void lora_rs_msg_init(LORA_RS_OBS_STR *m) {
  if (LORA_exists) {
    m->unit_id = 0;  // Not in use
    m->message_type = 0;
    m->message_counter = 0;
    m->rg = 0.0;
    m->rgds = 0;
    for (int i=0; i<3; i++) {
      m->sm[i] = 0;
      m->st[i] = 85.0;
    }
    for (int i=0; i<2; i++) {
      m->t[i] = 0.0;
      m->p[i] = 0.0;
      m->h[i] = 0.0;
    }
    m->voltage = 0.0;
    m->hth= 0;
    m->need2log = false;
    m->info.message_counter = 0;
    memset (m->info.message, 0, LORA_SG_INFO_MSG_LENGTH);
  }
}

/* 
 *=======================================================================================================================
 * lora_rs_find() - use unit id to fnd structure index, if no match, assign first free. Return -1 full
 *                - sets unit_id in structure
 *=======================================================================================================================
 */
int lora_rs_find(int unit_id) {
  for (int i=0; i< NUMBER_RS_LORA_DEVICES; i++) {
    // If we find an empty location before we find a match then we know it's not registered
    if (lora_rs[i].unit_id == 0) {
      lora_rs[i].unit_id = unit_id;
      return (i);
    }
    else if (lora_rs[i].unit_id == unit_id) {
      return (i);
    }
  }
  return (-1); // More lora devices than structures. no empty spots
}

/* 
 *=======================================================================================================================
 * lora_rs_need2log() - Return true if we have a RS observation that needs to be logged
 *=======================================================================================================================
 */
bool lora_rs_need2log() {
  for (int i=0; i< NUMBER_RS_LORA_DEVICES; i++) {
    if (lora_rs[i].need2log) {
      return (true);
    }
  }
  return(false);
}

/* 
 *=======================================================================================================================
 * lora_device_initialize()
 *=======================================================================================================================
 */
void lora_device_initialize() {
  if (LORA_exists) {
    lora_sg_msg_clear();
    lora_sg_info_msg_clear();

    // Rain and Soil devices
    for (int i=0; i< NUMBER_RS_LORA_DEVICES; i++) {
      lora_rs_msg_init(&lora_rs[i]);
    }
  }
}

/* 
 *=======================================================================================================================
 * lora_sg_obs()
 *=======================================================================================================================
 *   Message Header    -  SGt,ID,TC,  
 *   RS                   2 ASCII characters
 *   Message type,        Integer    
 *   UNIT_ID,             Integer
 *   Transmit Counter,    Integer
 *   Message Type 1,2 or 3
 *   Battery Voltage      Float
 *   Status Bits          Unsigned
 * 
 *   Message Type 1    -  SG1,ID,TC,POWERON,F,U
 *     Message(String)     [POWERON]
 * 
 *   Message Type 2    -  SG2,ID,TC,I,F,U
 *     Stream Gauge(Integer)
 * 
 *   Message Type 3    -  SG3,ID,TC,I,F,F,F,F,F,F,F,I Stream Gauge and BMX
 *     Stream Gauge(Integer)
 *     BM1 Preasure(Float),Temperature(Float),Humidity(Float)
 *     BM2 Preasure(Float),Temperature(Float),Humidity(Float)
 *=======================================================================================================================
 */
void lora_sg_obs(char *obs) {
  int message_type = 0;
  int unit_id = 0;
  unsigned int message_counter = 0;
  char *message;
  char *p;
  
  // Serial_write ("LoRa SG OBS");
  
  p = &obs[2];
  message_type = atoi (strtok_r(p, ",", &p));
  unit_id = atoi (strtok_r(p, ",", &p));
  message_counter = atoi (strtok_r(p, ",", &p));

  sprintf (Buffer32Bytes, "LSG[%d] %d,%d", unit_id, message_type, message_counter);
  Output (Buffer32Bytes);

  if (message_type == 1) {
    // Stream Gauge Info Message
    lora_sg_info.ts = stc.getEpoch();;
    lora_sg_info.unit_id = unit_id;
    lora_sg_info.message_counter = message_counter;

    message = strtok_r(p, ",", &p);
    strncpy (lora_sg_info.message, message, LORA_SG_INFO_MSG_LENGTH-1); // minus 1 so last byte in array will always be null
 
    lora_sg_info.voltage = atof (strtok_r(p, ",", &p));
    lora_sg_info.hth = atoi (strtok_r(p, ",", &p));
  }
  else {
    // Stream Gauge Observation Message
    lora_sg.ts = stc.getEpoch();
    lora_sg.unit_id = unit_id;
    lora_sg.message_type = message_type;
    lora_sg.message_counter = message_counter;

    // In both message type 2 and 3 messages
    lora_sg.stream_gauge = atoi (strtok_r(p, ",", &p));

    if (message_type == 3) {
      // Message has BMX sensor information
      for (int i=0; i<SM_BMX_SENSORS; i++) {
        lora_sg.p[i] = atof (strtok_r(p, ",", &p));
        lora_sg.t[i] = atof (strtok_r(p, ",", &p));
        lora_sg.h[i] = atof (strtok_r(p, ",", &p));
      }
    }

    lora_sg.voltage = atof (strtok_r(p, ",", &p));
    lora_sg.hth = atoi (strtok_r(p, ",", &p));
    lora_sg.need2log = true;
  }
}

/* 
 *=======================================================================================================================
 * lora_rs_obs() - Rain and Soil Sensors
 *=======================================================================================================================
 *   Message Header    -  RSt,ID,TC,  
 *   RS                   2 ASCII characters
 *   Message type,        Integer    
 *   UNIT_ID,             Integer
 *   Transmit Counter,    Integer
 *   Message Type 1,2,3,4 or 5
 *   Battery Voltage      Float
 *   Status Bits          Unsigned
 *   
 *   Message Type 1    -  RS1,ID,TC,POWERON,F,U
 *     Message(String)     [POWERON]
 *    
 *   Message Type 2    -  RS2,ID,TC - Rain only
 *     Rain Gauge(Int)        Rain get summed until transmitted
 *     Rain Gauge Delta(Int)  Delta Time get summed until transmitted
 *    
 *   Message Type 3    -  RS3,ID,TC - Rain,Soil,No BMX - Soil Temp of 85C means no probe
 *     Include Type 2
 *     Soil 1 Temp(Float), Moisture(Int)
 *     Soil 2 Temp(Float), Moisture(Int)
 *     Soil 3 Temp(Float), Moisture(Int)
 *   
 *   Message Type 4    -  RS4,ID,TC - Rain,No Soil,BMX 
 *     Include Type 2
 *     BM1 Preasure(Float),Temperature(Float),Humidity(Float)
 *     BM2 Preasure(Float),Temperature(Float),Humidity(Float)
 *   
 *   Message Type 5    -  RS5,ID,TC - Rain,Soil,BMX
 *     Include Type 2
 *     Include Type 3
 *     Include Type 4
 *=======================================================================================================================
 */
void lora_rs_obs(char *obs) {
  LORA_RS_OBS_STR *m;

  int message_type = 0;
  int unit_id = 0;
  unsigned int message_counter = 0;
  char *message;
  char *p;
 
  // Serial_write ("LoRa RS OBS");
  
  p = &obs[2]; // Start after "RS"
  message_type = atoi (strtok_r(p, ",", &p));
  unit_id = atoi (strtok_r(p, ",", &p));
  message_counter = atoi (strtok_r(p, ",", &p));

  sprintf (Buffer32Bytes, "LRS[%d] %d,%d", unit_id, message_type, message_counter);
  Output (Buffer32Bytes);

  // Locate storage index
  int idx = lora_rs_find(unit_id);

  if (idx == -1) {
    // No Space - More Lora devices than we have defined space for
    // Observation lost
    Output ("LORA RS NoSpace");
    return;
  }

  m = &lora_rs[idx]; // Lets work with a pointer and not the index

  if (message_type == 1) {
    m->info.ts = stc.getEpoch();;
    m->info.message_counter = message_counter;
    message = strtok_r(p, ",", &p);
    strncpy (m->info.message, message, LORA_SG_INFO_MSG_LENGTH-1); // minus 1 so last byte in array will always be null
  }
  else {
    m->ts = stc.getEpoch();;
    m->message_type = message_type;
    // m->unit_id = unit_id;  // Already set with lora_rs_find()
    m->message_counter = message_counter;

    // We sum the rain gauge information
    m->rg += atof (strtok_r(p, ",", &p));
    m->rgds += atoi (strtok_r(p, ",", &p));

    if (message_type == 3 || message_type == 5){
      for (int i=0; i<SM_PROBES; i++) {
        m->sm[i] = atoi (strtok_r(p, ",", &p));
        m->st[i] = atof (strtok_r(p, ",", &p));
      }
    }
    if (message_type == 4 || message_type == 5){
      for (int i=0; i<SM_BMX_SENSORS; i++) {
        m->p[i] = atof (strtok_r(p, ",", &p));
        m->t[i] = atof (strtok_r(p, ",", &p));
        m->h[i] = atof (strtok_r(p, ",", &p));
      }
    }
  }
  m->voltage = atof (strtok_r(p, ",", &p));
  m->hth = atof (strtok_r(p, ",", &p));
  m->need2log = true;
}


/* 
 *=======================================================================================================================
 * lora_ld_obs()
 *=======================================================================================================================
 *   Message Header    -  LDt,FQDN,Messaage,TC  
 *   LD                   2 ASCII characters
 *   Message type,        1 Byte integer    
 *   Message,             String = distance information
 *   BatVoltage           Float
 *   
 *   LD1,C=1,T=16:22:52.000,F=2,LA=4002.5292N,LO=10503.8640W,A=1561.10,4.18
 *=======================================================================================================================
 */
void lora_ld_obs(char *lm) {
  int message_type = 0;
  int unit_id = 0;
  unsigned int message_counter = 0;
  char *p;
  
  p = &lm[2];
  message_type = atoi (strtok_r(p, ",", &p));

  if (message_type == 1) {
    Output ("LD1 MSG");
    sprintf (LoRaDistanceMessage, "%s,%s", cf_lora_distanceapi,p);
    if (cf_lora_distancelog == 1) {
      LoRaDistanceMessageSet = true;  // Main Loop() will make calls to relay the message to logging site
    }
    Serial_writeln(LoRaDistanceMessage);
  }
}

/* 
 *=======================================================================================================================
 * lora_initialize()
 *=======================================================================================================================
 */
void lora_initialize() { 
  // Output ("LORA Init");
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

    Output ("LORA OK");
  }
  else {
    SystemStatusBits |= SSB_LORA;  // Turn On Bit
    
    Output ("LORA NF");
  }

  // Even if LoRa not found set the timer for time we call initialize after setup()
  lora_alarm_timer = millis() + (LORA_RESET_NOACTIVITY * 60000);  // Minutes * 60 seconds
}

/* 
 *=======================================================================================================================
 * lora_msg_check()
 *=======================================================================================================================
 */
void lora_msg_check() {

  if (LORA_exists) {
    // Output ("LoRa Check()");
    if (rf95.available()) {
      byte iv [N_BLOCK];
    
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN]; // 251 Bytes
      uint8_t len  = sizeof(buf);
      uint8_t from = rf95.headerFrom();
      uint8_t to   = rf95.headerTo();
      uint8_t id   = rf95.headerId();
      uint8_t flags= rf95.headerFlags();
      int8_t  rssi = rf95.lastRssi(); 
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
        aes.set_IV(my_iv);
        aes.get_IV(iv);
        aes.do_aes_decrypt(buf, len, (byte *) msg, key, 128, iv);
        if ( ( msg[3] == 'R' && msg[4] == 'S' && (msg[5] == '1' || msg[5] == '2' || msg[5] == '3' || msg[5] == '4' || msg[5] == '5') ) || 
             ( msg[3] == 'S' && msg[4] == 'G' && (msg[5] == '1' || msg[5] == '2' || msg[5] == '3') ) ||     
             ( msg[3] == 'L' && msg[4] == 'D' && (msg[5] == '1') )
          ) {
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

            char *payload = (char*)(msg+3); 

            // Display LoRa Message on Serial Console           
            Serial_writeln (payload);

            switch (payload[0]) {
              case 'R' :  // Rain - Soil Gauge
                lora_rs_obs (payload);
                break;
              case 'S' :  // Stream Gauge
                lora_sg_obs (payload);
                break;
              case 'L' :  // LoRa Distance Message
                lora_ld_obs (payload);
                break;
              default  :  // Unknown 
                Output ("LORA Dev Unkn");
                break;
            }
          }
          else {
            Output ("LORA CS-ERR");
          }
        }
      }
      // Received LoRa Signal, Reset alarm
      lora_alarm_timer = millis() + (LORA_RESET_NOACTIVITY * 60000);
    }
    else {
      // Have not received LoRa message in N seconds
      if (millis() >= lora_alarm_timer) {
        Output ("LORA Init");
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
 * lora_msg_poll() -- Spend 750ms second looking for LoRa Messages
 *=======================================================================================================================
 */
void lora_msg_poll() {
  for (int i=0; i<3; i++) {
    lora_msg_check();
    delay (250);
  }
}

/*
 * ======================================================================================================================
 * lora_message_relay() - Relay the LoRa Message to a Logging site so we can see it on Our Cell Phone Browser
 * ======================================================================================================================
 */
bool lora_message_relay(char *msg)
{
  char response[64];
  char buf[96];
  int r, i=0, exit_timer=0;
  bool posted = false;
  
  int NetworkAccessStatus = nb_gsm.isAccessAlive();  // Check network access status
  unsigned long nt = GetCellEpochTime();

  if (!NetworkAccessStatus) {
    sprintf (Buffer32Bytes,"LMR:SEND->NO NW:%u",nt);
    Output(Buffer32Bytes);
  }
  else {
    sprintf (Buffer32Bytes,"LMR:SEND->HTTP:%u",nt);
    Output(Buffer32Bytes);

    if (nt < 1659354588) { // Chose a time that is in the past. 
      // If we get bad time then lets not trust modem with a connect call
      Output("OBS:NWTIME BAD");
    }
    else {
      // If we are going to hang, it will be in the connect call
      uint32_t ts = stc.getEpoch();
 
      // Alarm in 4 minutes from now, if modem hang. Then reset modem it and reboot
      stc.setAlarmEpoch(ts+240);
      stc.enableAlarm(stc.MATCH_HHMMSS);
      //Output("OBS:ALARM SET"); 

      if (!client.connect(cf_lora_distancewebserver, cf_lora_distancewebserver_port)) {
        stc.disableAlarm();
        Output("LMR:HTTP FAILED");
      }
      else {
        stc.disableAlarm();
        Output("LMR:HTTP CONNECTED");
      
        // Make a HTTP request:
        client.print("GET ");
        client.print(msg); // path
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(cf_lora_distancewebserver);
        client.println("Connection: close");
        client.println();

        Output("LMR:HTTP SENT");

        // Wait for data and take Wind Readings while doing so
        i=0;
        exit_timer = 0;
        Wind_TakeReading();
        while(client.connected() && !client.available()) {
          delay(100);  
          i++;
          if (i>=10) { // After a second take a wind observation
            Wind_TakeReading();
            i=0;

            // Prevent infinate waiting - not sure if this is needed but can't hurt
            if (++exit_timer >= 180) { // after 3 minutes lets call it quits
              break;
            }
          }
        }
      
        Output("LMR:HTTP WAIT");
        
        // Read first line of HTTP Response, then get out of the loop
        r=0;
        while ((client.connected() || client.available() ) && r<63 && !posted) {
          response[r] = client.read();
          response[++r] = 0;  // Make string null terminated
          if (strstr(response, "200 OK") != NULL) { // Does response includes a "200 OK" substring?
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

        sprintf (buf, "LMR:%s", response);
        Output(buf);
      
        // Server disconnected from clinet. No data left to read. Disconnect client from the server
        client.stop();

        sprintf (buf, "LMR:%sPosted", (posted) ? "" : "Not ");
        Output(buf);
      }
    }
  }
  return (posted);
}
