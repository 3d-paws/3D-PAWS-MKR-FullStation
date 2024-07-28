/* 
 *=======================================================================================================================
 * CF.h - Configuration File Settings
 *=======================================================================================================================
 */

/*
#
# CONFIG.TXT
#
# Line Length is limited to 63 characters
#12345678901234567890123456789012345678901234567890123456789012

webserver=
webserver_port=80
apikey=
instrument_id=
sim_pin=
sim_apn=
sim_username=
sim_password=
reboot_countdown_timer=79200
no_network_reset_count=60

# Private Key - 128 bits (16 bytes of ASCII characters)
aes_pkey=10FE2D3C4B5A6978

# Initialization Vector must be and always will be 128 bits (16 bytes.)
# The real iv is actually myiv repeated twice
# 1234567 -> 0x12D687 = 0x00 0x12 0xD6 0x87 0x00 0x12 0xD6 0x87
aes_myiv=1234567

# This unit's LoRa ID for Receiving and Sending Messages
lora_unitid=1

# You can set transmitter power from 5 to 23 dBm, default is 13
lora_txpwr=23

# Valid entries are 433, 866, 915
lora_freq=915
*/

char *cf_webserver="";
int  cf_webserver_port=80;
char *cf_apikey="";
char *cf_instrument_id="";
char *cf_sim_pin="";
char *cf_sim_apn="";
char *cf_sim_username="";
char *cf_sim_password="";
int cf_reboot_countdown_timer=79200;
int cf_no_network_reset_count=60;
int cf_lora_unitid=1;
int cf_lora_txpwr=23;
int cf_lora_freq=915;
char *cf_aes_pkey="";
char *cf_aes_myiv="";

// LoRa Distance Message Relay to Logging Website
int  cf_lora_distancelog = 0;                      // Set to 1 to log distance to webserver listed below
char cf_lora_distancewebserver[] = "foobar.com"; // Set to "" if not relaying the message
int  cf_lora_distancewebserver_port = 80;
char cf_lora_distanceapi[] = "/ldl.php?APISecretKey";  // Set to "" when not relaying the message
