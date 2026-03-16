/*
 * ======================================================================================================================
 *  cf.h - Configuration File Definations
 * ======================================================================================================================
 */

/* 
 * ======================================================================================================================
#
# CONFIG.TXT
#
# Line Length is limited to 60 characters
#123456789012345678901234567890123456789012345678901234567890
webserver=some.domain.com
webserver_port=80
urlpath=/measurements/url_create
# Will be added to the HTML Header as X-API-Key also passed in data as instrument_id
apikey=1234
instrument_id=0

# Information web server, Not Chords
info_server=some.domain.com
info_server_port=80
info_urlpath=/info/log.php
# Will be added to the HTML Header as X-API-Key
info_apikey=1234

# Carrier SIM APNs
# Twilio Super SIM (LTE-M/NB): wireless.twilio.com
# Twilio Narrowband (T-Mobile US): iot.nb
# T-Mobile direct NB-IoT: iot.tm or fast.t-mobile.com
# Verizon: vzwinternet or nb.internet
# AT&T: iot.launch or nb.internet
sim_pin=
sim_apn=
sim_username=
sim_password=

# Private Key - 128 bits (16 bytes of ASCII characters)
aes_pkey=10FE2D3C4B5A6978

# Initialization Vector must always be 128 bits (16 bytes.)
# The real iv is actually myiv repeated twice
# 01234567 -> 0x12D687 = 0x00 0x12 0xD6 0x87 0x00 0x12 0xD6 0x87
aes_myiv=01234567

# This unit's LoRa ID for Receiving and Sending Messages
lora_unitid=1:

# You can set transmitter power from 5 to 23 dBm, default is 13
lora_txpw=5

# Valid entries are 433, 866, 915
lora_freq=915

#################################################
# General Configurations Settings
#################################################
# No Wind
# 0 = wind data
# 1 = no wind data
nowind=0

# Rain Gauge (rg1) - pin D1
# Options 0,1
rg1_enable=0

# OptionPin 1 - pin A1
# 0 = No sensor
# 1 = raw (op1r - average 5 samples spaced 10ms)
# 2 = 2nd rain gauge (rg2)
# 5 = 5m distance sensor (ds, dsr)
# 10 = 10m distance sensor (ds, dsr)
op1=0

# OptionPin 2 - pin A2
# 0 = No sensor (Pin in use if pm25aqi air quality dectected)
# 1 = raw (op2r - average 5 samples spaced 10ms)
# 2 = read Voltaic battery voltage (vbv)
op2=0

# Distance sensor baseline. If positive, distance = baseline - ds_median
ds_baseline=0

# elevation used for MSLP
elevation=0

# Rain Total Rollover Offset from 0 UTC- Valuse of (0 to 23) valid
rtro=0

#################################################
# System Timing
#################################################
# Observation Period (1,5,6,10,15,20,30)
# 1 minute observation period is the default
obs_period=1

# Number of hours between daily reboots
# A value of 0 disables this feature
daily_reboot=22

# Reset after N attempts calling Send_http() and failing on GetCellEpochTime() or client.connect() functions.
no_network_reset_count=60
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Define Global Configuration File Variables
 * ======================================================================================================================
 */
#define CF_NAME           "CONFIG.TXT"
#define KEY_MAX_LENGTH    30                // Config File Key Length
#define VALUE_MAX_LENGTH  30                // Config File Value Length
#define LINE_MAX_LENGTH   VALUE_MAX_LENGTH+KEY_MAX_LENGTH+3   // =, CR, LF

// Extern variables

// Web Server
extern char *cf_webserver;
extern int  cf_webserver_port;
extern char *cf_urlpath;

extern char *cf_apikey;
extern int cf_instrument_id;

// Info Server
extern char *cf_info_server;
extern int  cf_info_server_port;
extern char *cf_info_urlpath;
extern char *cf_info_apikey; 

// SIM
extern char *cf_sim_pin;
extern char *cf_sim_apn;
extern char *cf_sim_username;
extern char *cf_sim_password;

// LoRa AES
extern char *cf_aes_pkey;
extern long cf_aes_myiv;

// LoRa
extern int cf_lora_unitid;
extern int cf_lora_txpwr;
extern int cf_lora_freq;

// Instruments
extern int cf_nowind;
extern int cf_rg1_enable;
extern int cf_op1;
extern int cf_op2;
extern int cf_ds_baseline;
extern int cf_daily_reboot;
extern int cf_elevation;

// System Timing
extern int cf_obs_period;
extern int cf_daily_reboot;
extern int cf_no_network_reset_count;
extern int cf_rtro;

// Function prototypes
void SD_ReadConfigFile();