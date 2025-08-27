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
webserver_path=
apikey=
instrument_id=

# 0=GET 1=POST
webserver_method=0

sim_pin=
sim_apn=
sim_username=
sim_password=

reboot_countdown_timer=79200
no_network_reset_count=60

# Private Key - 128 bits (16 bytes of ASCII characters)
aes_pkey=10FE2D3C4B5A6978

# Initialization Vector must always be 128 bits (16 bytes.)
# The real iv is actually myiv repeated twice
# 01234567 -> 0x12D687 = 0x00 0x12 0xD6 0x87 0x00 0x12 0xD6 0x87
aes_myiv=01234567

# This unit's LoRa ID for Receiving and Sending Messages
lora_unitid=1:

# You can set transmitter power from 5 to 23 dBm, default is 13
lora_txpwr=5

# Valid entries are 433, 866, 915
lora_freq=915

# Rain Gauges
rg1_enable=0
rg2_enable=0

# Distance Sensor
ds_enable=0
ds_baseline=0

# Observation Periods 1,2,5,6,10,15,20,30
obs_period=2
*/

// Look in SDC.h for actual defaults
char *cf_webserver="";
int  cf_webserver_port=80;
char *cf_webserver_path="";
char *cf_apikey="";
int cf_instrument_id=0;
int cf_webserver_method=0;
char *cf_sim_pin="";
char *cf_sim_apn="";
char *cf_sim_username="";
char *cf_sim_password="";
int cf_reboot_countdown_timer=79200;
int cf_no_network_reset_count=60;

char *cf_aes_pkey="";
long cf_aes_myiv=0;
int cf_lora_unitid=1;
int cf_lora_txpwr=5;
int cf_lora_freq=915;

int cf_rg1_enable=0;
int cf_rg2_enable=0;
int cf_ds_enable=0;
int cf_ds_baseline=0;
int cf_obs_period=0;
