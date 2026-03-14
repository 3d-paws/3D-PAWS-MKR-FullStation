# Serial Monitor
[←Top](../README.md)<BR>
Adding a jumper wire between Particle pin D8 (Boron & Argon), A2 (Muon) and ground will enable serial text output on the USB port at boot time.

A serial monitor from Arduino's IDE can be used. TIP: With the Arduino serial monitor. Select "Both NL & CR" on the pull down menu at the bottom.  On a Mac with Visual Studio installed with Particle's Development Environment; the shell command "particle serial monitor" can be used.

Upon Particle board boot with the jumper wire connected, software will wait 60 seconds for you to connect the serial monitor. Flashing the board led.  After 60 seconds the software will continue the boot process. Below is an example of what you might see as the software initializes and discovers connected devices.

Particle
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 200px; border: 1px solid black; padding: 10px;">
<pre>
$ particle serial monitor
Opening serial monitor for com port: "/dev/tty.usbmodem14101"
Serial monitor opened successfully

OLED:Enabled
SC:Enabled
SER:OK
Copyright [2024] [University Corporation for Atmospheric Research]
FSAC-241206v37
SD:Online
SD:OBS DIR Exists
N2S:None
CF:aes_pkey=[....]
CF:aes_myiv=[....]
CF:lora_unitid=[1]
CF:lora_txpower=[23]
CF:lora_freq=[915]
EEPROM DUMP
 LEN:4096
 RT1:0.20
 RP1:0.00
 RT2:0.00
 RP2:0.00
 RGTS:1733949583
 N2SFP:0
 CS:1733949583
 CSC:1733949583
2000-01-01T00:00:16+
2024-12-11T20:41:15*
RTC:VALID
STC: Valid
2024-12-11T20:41:15=
SIM:Internal
SIM:NO UPDATE FILE
TXI:INIT
TXI5M Found
TXI=5M
A4:INIT
A4=DIST
DIST=5M
A5:INIT
A5=NF
BMX:INIT
get_Bosch_ChipID()
  I2C:77 Reg:00
  CHIPID:60 BME/390
BMP390_1 OK
get_Bosch_ChipID()
  I2C:76 Reg:00
  ERR_ET:7
  I2C:76 Reg:D0
  ERR_ET:7
BMX_2 NF
HTU21D:INIT
HTU NF
MCP9808:INIT
MCP1 OK
MCP2 NF
MCP3 NF
MCP4 NF
SHT:INIT
SHT1 OK
SHT2 NF
HIH8:INIT
HIH8 OK
SI1145:INIT
SI:NF
VLX:INIT
VLX OK
BLX:INIT
BLX:OK
AS5600:INIT
WD:OK
PM25AQI:INIT
PM:NF
HDC:INIT
HDC1 OK
HDC2 OK
LPS:INIT
LPS1 OK
LPS2 NF
TLW:INIT
TLW NF
TSM:INIT
TSM NF
TMSM:INIT
TMSM NF
WBT:INIT
WBT:OK
HI:INIT
HI:OK
WBGT:INIT
WBGT:OK wo/Globe
AES_KEY[....]
AES_MYIV[....]
LORA CFV OK
LORA NF
DoAction:OK
WindDist Init()
............................................................
IMSI:234103519249568
T>6, RT>6 - OK
EEPROM RT UPDATED
OBS[0]->SD
{"at":"2024-12-11T20:42:23","css":42.4994,"hth":17397249,"bcs":3,"bpc":96.9,"cfr":0,"rg":0.0,"rgt":0.2,"rgp":0.0,"ws":0.0,"wd":181,"wg":0.0,"wgd":-999,"bp1":843.6,"bt1":22.5,"st1":22.0,"sh1":30.5,"hdt1":22.2,"hdh1":29.3,"hdt2":22.2,"hdh2":29.3,"lpt1":17.0,"lpp1":843.1,"ht2":23.3,"hh2":29.3,"mt1":22.2,"vlx":7.0,"blx":293.5,"sg":431.0,"hi":21.8,"wbt":12.6,"wbgt":17.1}
DB:OBS_Exit
RTC: 1ST SYNC
2024-12-11T20:42:26*
INFO_DO()
{"devid":"e00fce68bde8f63590a3b118","devos":"6.1.1","freemem":56248,"uptime":88,"board":"boron","at":"2024-12-11T20:42:26","ver":"FSAC-241206v37","hth":17397249,"obsi":"60s","obsti":"5m","t2nt":"212s","drct":79200,"n2s":"NF","ps":"VIN","bcs":"CHARGED","bpc":96.9,"css":42.4994,"csq":37.4990,"imsi":"234103519249568","actsim":"INTERNAL","a4":"DIST 5M","sensors":"BMX1(BMP390),MCP1,SHT1,HDC1,HDC2,LPS1,HIH8,VEML,BLX,AS5600,HI,WBT,WBGT WO/GLOBE","oled":"32","scepin":"ENABLED","sce":"TRUE"}
INFO->PUB OK[488]
Connected
{"at":"2024-12-11T20:42:23","css":54.9996,"hth":17397249,"bcs":3,"bpc":96.9,"cfr":0,"rg":0.0,"rgt":0.2,"rgp":0.0,"ws":0.0,"wd":181,"wg":0.0,"wgd":-999,"bp1":843.6,"bt1":22.5,"st1":22.0,"sh1":30.5,"hdt1":22.2,"hdh1":29.3,"hdt2":22.2,"hdh2":29.3,"lpt1":17.0,"lpp1":843.1,"ht2":23.3,"hh2":29.3,"mt1":22.2,"vlx":7.0,"blx":293.5,"sg":431.0,"hi":21.8,"wbt":12.6,"wbgt":17.1}
FS[0]->PUB OK[369]
2024-12-11T20:42:30
CS:52.49 B:3,96.91
</pre>
</div>
<BR>
MKR NB 1500
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 200px; border: 1px solid black; padding: 10px;">
<pre>
MKR NB 1500
13:18:00.710 -> 
13:18:00.710 -> OLED:Enabled
13:18:00.710 -> SC:Enabled
13:18:00.710 -> NW:DEBUG ENABLED
13:18:00.748 -> SER:OK
13:18:02.732 -> Copyright [2025] [University Corporation for Atmospheric Research]
13:18:02.769 -> MKRFS-250907-1
13:18:02.769 -> REBOOTPN SET
13:18:02.804 -> HEARTBEAT SET
13:18:03.064 -> ECCX08:INIT
13:18:03.138 -> DevID:012351576D29B51BEE
13:18:03.138 -> SD:INIT
13:18:03.176 -> SD:Online
13:18:03.176 -> SD:OBS DIR Exists
13:18:03.213 -> CF:webserver=[3d.chordsrt.com]
13:18:03.251 -> CF:webserver_port=[80]
13:18:03.251 -> CF:webserver_path=[/measurements/url_create]
13:18:03.287 -> CF:apikey=[21DE6A8A]
13:18:03.325 -> CF:instrument_id=[53]
13:18:03.361 -> CF:webserver_method=[0]
13:18:03.395 -> CF:sim_apn=[super]
13:18:03.395 -> CF:sim_pin=[]
13:18:03.432 -> CF:sim_username=[]
13:18:03.470 -> CF:sim_password=[]
13:18:03.507 -> CF:reboot_countdown_timer=[79200]
13:18:03.545 -> CF:no_network_reset_count=[60]
13:18:03.582 -> CF:lora_unitid=[1]
13:18:03.582 -> CF:lora_txpwr=[5]
13:18:03.619 -> CF:lora_freq=[915]
13:18:03.655 -> CF:aes_pkey=[FEEDCODEBEEF4242]
13:18:03.691 -> CF:aes_myiv=[56495141]
13:18:03.728 -> CF:rg1_enable=[1]
13:18:03.765 -> CF:rg2_enable=[0]
13:18:03.803 -> CF:ds_enable=[0]
13:18:03.838 -> CF:ds_baseline=[0]
13:18:03.874 -> CF:obs_period=[2]
13:18:03.874 -> N2S:FOUND
13:18:03.912 -> CM:INIT
13:18:03.912 -> CM:Connect
13:18:03.949 -> CM:Connect After
13:18:03.949 -> STC:INIT
13:18:03.987 -> RTC:INIT
13:18:04.020 -> 2000-01-01T00:00:48R
13:18:04.020 -> RTC:TIME ERR
13:18:04.055 -> EEPROM OK
13:18:04.092 -> EEPROM DUMP
13:18:04.092 ->  RT1:0.00
13:18:04.125 ->  RP1:0.00
13:18:04.125 ->  RT2:0.00
13:18:04.163 ->  RP2:0.00
13:18:04.163 ->  RGTS:1757272298
13:18:04.197 ->  N2SFP:0
13:18:04.197 ->  CS:1757272298
13:18:04.234 ->  CSC:1757272298
13:18:04.234 -> OBS Interval:2m
13:18:04.271 -> PMIC:INIT
13:18:04.271 -> PM:FOUND
13:18:04.309 -> PM:CHRGR:DISABLED
13:18:04.309 -> PM:MSV=3.50
13:18:04.347 -> PM:BS=1
13:18:04.347 -> PM:PS=GOOD
13:18:04.381 -> RG1:ENABLED
13:18:04.414 -> RG2:NOT ENABLED
13:18:04.414 -> AS5600:INIT
13:18:04.448 -> WD:OK
13:18:04.448 -> BMX:INIT
13:18:04.486 -> get_Bosch_ChipID()
13:18:04.486 ->   I2C:77 Reg:00
13:18:04.520 ->   CHIPID:60 BME/390
13:18:04.558 -> BMP390_1 OK
13:18:04.558 -> get_Bosch_ChipID()
13:18:04.596 ->   I2C:76 Reg:00
13:18:04.596 ->   ERR_ET:2
13:18:04.633 ->   I2C:76 Reg:D0
13:18:04.633 ->   ERR_ET:2
13:18:04.670 -> BMX_2 NF
13:18:04.670 -> HTU21D:INIT
13:18:04.707 -> HTU NF
13:18:04.707 -> MCP9808:INIT
13:18:04.741 -> MCP1 OK
13:18:04.741 -> MCP2 NF
13:18:04.779 -> MCP3 NF
13:18:04.779 -> MCP4 NF
13:18:04.813 -> SHT:INIT
13:18:04.846 -> SHT1 OK
13:18:04.846 -> SHT2 NF
13:18:04.883 -> HIH8:INIT
13:18:04.883 -> HIH8 NF
13:18:04.883 -> LUX:INIT
13:18:05.030 -> LUX OK
13:18:05.065 -> PM25AQI:INIT
13:18:05.065 -> PM:NF
13:18:05.065 -> HDC:INIT
13:18:05.104 -> HDC1 NF
13:18:05.137 -> HDC2 NF
13:18:05.137 -> LPS:INIT
13:18:05.170 -> LPS1 NF
13:18:05.170 -> LPS2 NF
13:18:05.207 -> TLW:INIT
13:18:05.207 -> TLW NF
13:18:05.245 -> TSM:INIT
13:18:05.245 -> TSM NF
13:18:05.281 -> TMSM:INIT
13:18:05.281 -> TMSM NF
13:18:05.281 -> WBT:INIT
13:18:05.319 -> WBT:OK
13:18:05.319 -> HI:INIT
13:18:05.354 -> HI:OK
13:18:05.354 -> WBGT:INIT
13:18:05.392 -> WBGT:OK wo/Globe
13:18:05.392 -> CM:CHECK
13:18:15.084 -> SIM card ok
13:18:15.120 -> CM:CHECK AFTER
13:18:16.622 -> GPRS.attachGPRS(): 4
13:18:16.622 -> Connected to GPRS Network
13:18:16.657 -> NW:Connect
13:18:16.657 -> NW:[AT&T Twilio]
13:18:18.690 -> AES_KEY[FEEDCODEBEEF4242]
13:18:18.727 -> AES_MYIV[lu]
13:18:18.727 -> LORA CFV OK
13:18:19.060 -> LORA NF
13:18:19.098 -> IMEI:352753090600782
13:18:19.098 -> L0.0.00.00.05.12,A.02.21
13:18:19.136 -> CSS:28
13:18:19.173 -> WS:Enabled
13:18:19.173 -> Start Main Loop
13:18:19.208 -> WDA:Init()
13:18:19.243 -> +CCLK: "25/09/07,13:18:18-24"
13:18:19.243 -> GNWT:OK[1757272698]
13:18:19.243 -> NW:EPOCH 1757272698
13:18:19.279 -> RTC:SET
13:18:19.279 -> 2025-09-07T19:18:18R
13:18:19.315 -> STC:SET
13:18:19.352 -> 2025-09-07T19:18:18S
13:18:20.193 -> ......GPRS.isAccessAlive(): 1
13:18:25.280 -> Connected to Cellular Network
13:18:26.297 -> ..........GPRS.isAccessAlive(): 1
13:18:35.433 -> Connected to Cellular Network
13:18:36.434 -> ..........GPRS.isAccessAlive(): 1
13:18:45.572 -> Connected to Cellular Network
13:18:46.570 -> ..........GPRS.isAccessAlive(): 1
13:18:55.728 -> Connected to Cellular Network
13:18:56.709 -> ..........GPRS.isAccessAlive(): 1
13:19:05.866 -> Connected to Cellular Network
13:19:06.852 -> ..........GPRS.isAccessAlive(): 1
13:19:16.003 -> Connected to Cellular Network
13:19:16.976 -> ....
13:19:20.089 -> WS:0.00 WD:87
13:19:21.147 -> T>6, RT>6 - OK
13:19:21.258 -> EEPROM DUMP
13:19:21.258 ->  RT1:0.00
13:19:21.292 ->  RP1:0.00
13:19:21.327 ->  RT2:0.00
13:19:21.327 ->  RP2:0.00
13:19:21.364 ->  RGTS:1757272760
13:19:21.364 ->  N2SFP:0
13:19:21.400 ->  CS:1757272760
13:19:21.438 ->  CSC:1757272760
13:19:21.438 -> CRT:OK-NF
13:19:21.475 -> OBS_DO()
13:19:21.512 -> OBS_TAKE()
13:19:21.619 -> EEPROM RT UPDATED
13:19:22.015 -> OBS_BUILD()
13:19:22.048 -> OBS->URL
13:19:22.048 -> {"key":"21DE6A8A","devid":"012351576D29B51BEE","instrument_id":53,"at":"2025-09-07T19:19:20","css":25,"hth":29061649,"rg1":0.0,"rgt1":0.0,"rgp1":0.0,"ws":0.0,"wd":87,"wg":0.0,"wgd":-999,"bp1":847.3,"bt1":23.4,"st1":23.2,"sh1":36.2,"mt1":22.9,"lx":141.5,"hi":-999.9,"wbt":-4.1,"wbgt":-999.9}
13:19:22.084 -> OBS->SD
13:19:22.084 -> /OBS/20250907.log
13:19:22.159 -> OBS Logged to SD
13:19:22.196 -> OBS_SEND()
13:19:22.234 -> +CCLK: "25/09/07,13:19:21-24"
13:19:22.268 -> GNWT:OK[1757272761]
13:19:22.305 -> OBS:SEND->HTTP
13:19:23.115 -> OBS:HTTP CONNECTED
13:19:23.115 -> /measurements/url_create?key=21DE6A8A&devid=012351576D29B51BEE&instrument_id=53&at=2025-09-07T19%3A19%3A20&css=25&hth=29061649&rg1=0.0000&rgt1=0.0000&rgp1=0.0000&ws=0.0000&wd=87&wg=0.0000&wgd=-999&bp1=847.3000&bt1=23.4000&st1=23.2000&sh1=36.2000&mt1=22.9000&lx=141.5000&hi=-999.9000&wbt=-4.1000&wbgt=-999.9000
13:19:23.547 -> OBS:HTTP SENT
13:19:24.237 -> OBS:HTTP WAIT
13:19:24.272 -> OBS:HTTP RESP[HTTP/1.1 200 OK]
13:19:24.272 -> 48 54 54 50 2F 31 2E 31 20 32 30 30 20 4F 4B 
13:19:24.272 -> Server: nginx
13:19:24.272 -> Date: Sun, 07 Sep 2025 19:19:23 GMT
13:19:24.272 -> Content-Type: text/html; charset=utf-8
13:19:24.272 -> Transfer-Encoding: chunked
13:19:24.272 -> Connection: close
13:19:24.272 -> X-Frame-Options: SAMEORIGIN
13:19:24.272 -> X-XSS-Protection: 1; mode=block
13:19:24.272 -> X-Content-Type-Options: nosniff
13:19:24.272 -> X-Download-Options: noopen
13:19:24.272 -> X-Permitted-Cross-Domain-Policies: none
13:19:24.272 -> Referrer-Policy: strict-origin-when-cross-origin
13:19:24.272 -> Access-Control-Allow-Origin: *
13:19:24.272 -> ETag: W/"bf6a136cc1847d88e5896809c63b1aeb"
13:19:24.272 -> Cache-Control: max-age=0, private, must-revalidate
13:19:24.272 -> X-Request-Id: dfafaa9a-644c-4aed-9e50-a31b132ee27c
13:19:24.309 -> X-Runtime: 0.070150
13:19:24.309 -> 
13:19:24.309 -> 20
13:19:24.309 -> Measurement created successfully
13:19:24.309 -> 0
13:19:24.309 -> 
</pre>
</div>

## Setup Notes for a Windows Computer Serial Console using Putty

### Install PuTTY

- Download PuTTY from the official site: [https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html)

- Select the **64-bit x86 installer**: [putty-64bit-0.83-installer.msi](https://the.earth.li/~sgtatham/putty/latest/w64/putty-64bit-0.83-installer.msi)  

### Plug in the Arduino Device

### Locate the COM Port

- Open **Device Manager** → **Ports (COM & LPT)**  
- Identify the COM port assigned to your Arduino device

### Run PuTTY

1. Launch **PuTTY**.  
2. Select **Serial** as the connection type.  
3. Enter the **COM port** discovered from Device Manager.  
4. Set the **Speed** (baud rate) to **9600**.  
5. Click **Open** at the bottom to start the serial session.
