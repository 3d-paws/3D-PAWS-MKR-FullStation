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
09:12:08.524 -> 
09:12:08.524 -> OLED:Enabled
09:12:08.562 -> SC:Enabled
09:12:08.562 -> NW:DEBUG ENABLED
09:12:08.562 -> SER:OK
09:12:10.577 -> Copyright [2026] [University Corporation for Atmospheric Research]
09:12:10.577 -> MKRFS-260316
09:12:10.615 -> DevID:8120067b801e913404027822
09:12:10.684 -> CryptoID:XXXXXXXXXXXXXXXXXX
09:12:10.684 -> REBOOTPN SET
09:12:10.717 -> HEARTBEAT SET
09:12:10.995 -> SD:INIT
09:12:11.031 -> SD:Online
09:12:11.031 -> SD:OBS DIR Exists
09:12:11.068 -> CF:webserver=[3d.chordsrt.com]
09:12:11.101 -> CF:webserver_port=[80]
09:12:11.101 -> CF:urlpath=[/measurements/url_create]
09:12:11.139 -> CF:apikey=[XXXXXXXX]
09:12:11.176 -> CF:instrument_id=[1]
09:12:11.211 -> CF:info_server=[iot.icdp.ucar.edu]
09:12:11.211 -> CF:info_server_port=[80]
09:12:11.244 -> CF:info_urlpath=[/info/log.php]
09:12:11.280 -> CF:info_apikey=[XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX]
09:12:11.314 -> CF:sim_apn=[super]
09:12:11.381 -> CF:sim_pin=[]
09:12:11.419 -> CF:sim_username=[]
09:12:11.490 -> CF:sim_password=[]
09:12:11.525 -> CF:aes_pkey=[XXXXXXXXXXXXX]
09:12:11.561 -> CF:aes_myiv=[XXXXXXXX]
09:12:11.595 -> CF:lora_unitid=[1]
09:12:11.630 -> CF:lora_txpwr=[5]
09:12:11.663 -> CF:lora_freq=[915]
09:12:11.837 -> CF:nowind=[0]
09:12:11.837 -> CF:rg1_enable=[1]
09:12:11.837 -> CF:op1=[0]
09:12:11.837 -> CF:op2=[2]
09:12:11.843 -> CF:ds_baseline=[0]
09:12:11.913 -> CF:obs_period=[1]
09:12:11.949 -> CF:cf_rtro=[0]
09:12:11.986 -> CF:daily_reboot=[22]
09:12:12.059 -> CF:no_network_reset_count=[60]
09:12:12.059 -> OBS Interval:1m
09:12:12.095 -> CM:INIT
09:12:12.095 -> CM:Connect
09:12:12.129 -> CM:Connect After
09:12:12.129 -> STC:INIT
09:12:12.165 -> RTC:INIT
09:12:12.199 -> 2026-02-17T15:12:11R
09:12:12.199 -> RTC:VALID
09:12:12.234 -> STC:VALID
09:12:12.268 -> EEPROM OK
09:12:12.301 -> EEPROM DUMP
09:12:12.301 ->  RT1:0.00
09:12:12.335 ->  RP1:26.00
09:12:12.335 ->  RT2:0.00
09:12:12.368 ->  RP2:0.00
09:12:12.368 ->  RGTS:1773759843
09:12:12.402 ->  N2SFP:0
09:12:12.437 ->  CS:1773759869
09:12:12.437 ->  CSC:1773759869
09:12:12.473 -> OBS Interval:1m
09:12:12.473 -> INFO_Server:OK
09:12:12.508 -> PMIC:INIT
09:12:12.508 -> PM:FOUND
09:12:12.543 -> PM:CHRGR:DISABLED
09:12:12.543 -> PM:MSV=3.50
09:12:12.581 -> PM:BS=1
09:12:12.581 -> PM:PS=GOOD
09:12:12.618 -> RG1:ENABLED
09:12:12.618 -> RG2:NOT ENABLED
09:12:12.653 -> WIND:ENABLED
09:12:12.653 -> AS5600:INIT
09:12:12.687 -> WD:OK
09:12:12.687 -> MUX:INIT
09:12:12.720 -> MUX OK
09:12:12.757 -> MUX:SCAN
09:12:12.757 ->   CH-0 TSM NF
09:12:12.757 ->   CH-1 TSM NF
09:12:12.790 ->   CH-2 TSM NF
09:12:12.828 ->   CH-3 TSM NF
09:12:12.828 ->   CH-4 TSM NF
09:12:12.862 ->   CH-5 TSM NF
09:12:12.862 ->   CH-6 TSM NF
09:12:12.895 ->   CH-7 TSM NF
09:12:12.895 -> DSMUX:INIT
09:12:12.928 -> DSMUX Channel Scan
09:12:16.876 ->   dst-0=19.81 28:3B:18:79:A2:19:03:77
09:12:16.876 ->   dst-1=19.81 28:FB:90:18:04:00:00:99
09:12:16.876 ->   dst-4=19.87 28:64:89:36:04:00:00:E6
09:12:17.874 ->   dst-7=19.93 28:D0:86:36:04:00:00:52
09:12:17.912 -> DSMUX 4 Found
09:12:17.912 -> BMX:INIT
09:12:17.946 -> get_Bosch_ChipID()
09:12:17.946 ->   I2C:77 Reg:00
09:12:17.980 ->   CHIPID:60 BME/390
09:12:18.017 -> BMP390_1 OK
09:12:18.017 -> get_Bosch_ChipID()
09:12:18.052 ->   I2C:76 Reg:00
09:12:18.052 ->   ERR_ET:2
09:12:18.087 ->   I2C:76 Reg:D0
09:12:18.121 ->   ERR_ET:2
09:12:18.121 -> BMX_2 NF
09:12:18.156 -> HTU21D:INIT
09:12:18.156 -> HTU NF
09:12:18.189 -> MCP9808:INIT
09:12:18.189 -> MCP1 OK
09:12:18.225 -> MCP2 NF
09:12:18.225 -> MCP3 NF
09:12:18.259 -> MCP4 NF
09:12:18.259 -> SHT:INIT
09:12:18.293 -> SHT1 OK
09:12:18.330 -> SHT2 NF
09:12:18.330 -> HIH8:INIT
09:12:18.366 -> HIH8 NF
09:12:18.366 -> LUX:INIT
09:12:18.506 -> LUX OK
09:12:18.506 -> PM25AQI:INIT
09:12:18.540 -> PM:NF
09:12:18.540 -> HDC:INIT
09:12:18.573 -> HDC1 NF
09:12:18.573 -> HDC2 NF
09:12:18.610 -> LPS:INIT
09:12:18.610 -> LPS1 NF
09:12:18.647 -> LPS2 NF
09:12:18.647 -> TLW:INIT
09:12:18.681 -> TLW NF
09:12:18.681 -> WBT:INIT
09:12:18.715 -> WBT:OK
09:12:18.715 -> HI:INIT
09:12:18.749 -> HI:OK
09:12:18.749 -> WBGT:INIT
09:12:18.783 -> WBGT:OK wo/Globe
09:12:18.783 -> MSLP:INIT
09:12:18.816 -> MSLP:OK
09:12:18.816 -> CM:CHECK
09:12:28.846 -> SIM card ok
09:12:28.883 -> CM:CHECK AFTER
09:12:30.376 -> GPRS.attachGPRS(): 4
09:12:30.376 -> Connected to GPRS Network
09:12:30.410 -> NW:Connect
09:12:30.443 -> NW:[AT&T Twilio]
09:12:32.441 -> AES_KEY[FEEDCODEBEEF4242]
09:12:32.477 -> AES_MYIV[lu]
09:12:32.477 -> LORA CFV OK
09:12:32.826 -> LORA NF
09:12:32.826 -> IMEI:352753090600782
09:12:32.861 -> L0.0.00.00.05.12,A.02.21
09:12:32.896 -> CSS:28
09:12:32.932 -> Start Main Loop
09:12:32.932 -> WDA:Init()
09:12:32.965 -> +CCLK: "26/03/17,09:12:32-24"
09:12:33.002 -> GNWT:OK[1773760352]
09:12:33.002 -> NW:EPOCH 1773760352
09:12:33.002 -> RTC:SET
09:12:33.040 -> 2026-03-17T15:12:32R
09:12:33.040 -> STC:SET
09:12:33.113 -> 2026-03-17T15:12:32S
09:12:33.925 -> ......GPRS.isAccessAlive(): 1
09:12:39.040 -> Connected to Cellular Network
09:12:40.015 -> ..........GPRS.isAccessAlive(): 1
09:12:49.180 -> Connected to Cellular Network
09:12:50.153 -> ..........GPRS.isAccessAlive(): 1
09:12:59.289 -> Connected to Cellular Network
09:13:00.311 -> ..........GPRS.isAccessAlive(): 1
09:13:09.429 -> Connected to Cellular Network
09:13:10.422 -> ..........GPRS.isAccessAlive(): 1
09:13:19.596 -> Connected to Cellular Network
09:13:20.558 -> ..........GPRS.isAccessAlive(): 1
09:13:29.726 -> Connected to Cellular Network
09:13:30.710 -> ....
09:13:33.811 -> WS:0.00 WD:88
09:13:34.835 -> T>6, RT>6 - OK
09:13:34.982 -> EEPROM DUMP
09:13:34.982 ->  RT1:0.00
09:13:35.020 ->  RP1:26.00
09:13:35.020 ->  RT2:0.00
09:13:35.055 ->  RP2:0.00
09:13:35.088 ->  RGTS:1773760414
09:13:35.088 ->  N2SFP:0
09:13:35.123 ->  CS:1773760440
09:13:35.156 ->  CSC:1773760440
09:13:35.156 -> CRT:OK-NF
09:13:35.194 -> {"MT":"INFO","at":"2026-03-17T15:13:34","devid":"8120067b801e913404027822","board":"AFM0WiFi","ver":"MKRFS-260316","bcs":!CHARGING,"hth":1,"elev":0,"rtro":0,"ls":"3d.chordsrt.com","lsp":80,"lsurl":"/measurements/url_create","lsapi":"XXXXXXXX","lsid":53,"obsi":"1m","t2nt":"4294907s","drbt":"22m","n2s":"NF","devs":"rtc,sd,eeprom,mux,dsmux,oled(32)","sensors":"BMX1(BMP390),MCP1,SHT1,VEML,WIND,WS(D0),AS5600,DST(0,1,4,7),HI,WBT,WBGT WO/GLOBE,RG1(D1),VBV(A2)"}
09:13:35.231 -> +CCLK: "26/03/17,09:13:34-24"
09:13:35.231 -> GNWT:OK[1773760414]
09:13:35.268 -> OBS:SEND->HTTP
09:13:36.686 -> OBS:HTTP CONNECTED
09:13:37.358 -> OBS:HTTP SENT
09:13:38.502 -> OBS:HTTP WAIT
09:13:38.540 -> OBS:HTTP RESP[HTTP/1.1 200 OK]
09:13:38.540 -> 48 54 54 50 2F 31 2E 31 20 32 30 30 20 4F 4B 
09:13:38.540 -> Date: Tue, 17 Mar 2026 15:13:37 GMT
09:13:38.540 -> Server: Apache
09:13:38.540 -> Strict-Transport-Security: max-age=63072000
09:13:38.540 -> X-Frame-Options: SAMEORIGIN
09:13:38.540 -> X-Content-Type-Options: nosniff
09:13:38.540 -> X-XSS-Protection: 1; mode=block
09:13:38.540 -> Content-Length: 17
09:13:38.540 -> Connection: close
09:13:38.540 -> Content-Type: text/html; charset=UTF-8
09:13:38.540 -> 
09:13:38.540 -> Log entry saved.
09:13:38.720 -> 
09:13:38.756 -> OBS:Posted
09:13:38.789 -> INFO->PUB WiFi OK
09:13:38.789 -> {"MT":"INFO","at":"2026-03-17T15:13:34","devid":"8120067b801e913404027822","board":"AFM0WiFi","ver":"MKRFS-260316","bcs":!CHARGING,"hth":1,"elev":0,"rtro":0,"ls":"3d.chordsrt.com","lsp":80,"lsurl":"/measurements/url_create","lsapi":"XXXXXXXX","lsid":53,"obsi":"1m","t2nt":"4294907s","drbt":"22m","n2s":"NF","devs":"rtc,sd,eeprom,mux,dsmux,oled(32)","sensors":"BMX1(BMP390),MCP1,SHT1,VEML,WIND,WS(D0),AS5600,DST(0,1,4,7),HI,WBT,WBGT WO/GLOBE,RG1(D1),VBV(A2)"}
09:13:38.824 -> INFO->SD OK
09:13:38.859 -> OBS_DO()
09:13:38.859 -> OBS_TAKE()
09:13:38.997 -> EEPROM RT UPDATED
09:13:39.461 -> MUX:OBSDO
09:13:42.642 -> OBS_BUILD()
09:13:42.677 -> OBS->URL
09:13:42.677 -> {"key":"XXXXXXXX","devid":"8120067b801e913404027822","instrument_id":53,"at":"2026-03-17T15:13:38","css":30,"hth":1,"rg1":0.2,"rgt1":0.2,"rgp1":26.0,"vbv":1.9,"vpc":67.6,"ws":0.0,"wd":88,"wg":0.0,"wgd":-999,"bp1":840.6,"bt1":20.2,"st1":19.7,"sh1":40.8,"mt1":19.3,"vlx":148.6,"hi":-999.9,"wbt":-3.9,"wbgt":-999.9,"mslp":840.6,"dst0":19.8,"dst1":20.1,"dst4":20.1,"dst7":20.1}
09:13:42.711 -> OBS->SD
09:13:42.711 -> /OBS/20260317.log
09:13:42.745 -> OBS Logged to SD
09:13:42.780 -> OBS_SEND()
09:13:42.849 -> +CCLK: "26/03/17,09:13:42-24"
09:13:42.849 -> GNWT:OK[1773760422]
09:13:42.885 -> OBS:SEND->HTTP
09:13:43.814 -> OBS:HTTP CONNECTED
09:13:43.814 -> /measurements/url_create?key=XXXXXXXX&devid=8120067b801e913404027822&instrument_id=53&at=2026-03-17T15%3A13%3A38&css=30&hth=1&rg1=0.2000&rgt1=0.2000&rgp1=26.0000&vbv=1.9000&vpc=67.6000&ws=0.0000&wd=88&wg=0.0000&wgd=-999&bp1=840.6000&bt1=20.2000&st1=19.7000&sh1=40.8000&mt1=19.3000&vlx=148.6000&hi=-999.9000&wbt=-3.9000&wbgt=-999.9000&mslp=840.6000&dst0=19.8000&dst1=20.1000&dst4=20.1000&dst7=20.1000
09:13:44.347 -> OBS:HTTP SENT
09:13:44.418 -> GPRS.isAccessAlive(): 1
09:13:44.418 -> Connected to Cellular Network
09:13:45.516 -> OBS:HTTP WAIT
09:13:45.550 -> OBS:HTTP RESP[HTTP/1.1 200 OK]
09:13:45.550 -> 48 54 54 50 2F 31 2E 31 20 32 30 30 20 4F 4B 
09:13:45.550 -> Server: nginx
09:13:45.550 -> Date: Tue, 17 Mar 2026 15:13:44 GMT
09:13:45.550 -> Content-Type: text/html; charset=utf-8
09:13:45.550 -> Transfer-Encoding: chunked
09:13:45.550 -> Connection: close
09:13:45.550 -> X-Frame-Options: SAMEORIGIN
09:13:45.550 -> X-XSS-Protection: 1; mode=block
09:13:45.550 -> X-Content-Type-Options: nosniff
09:13:45.550 -> X-Download-Options: noopen
09:13:45.550 -> X-Permitted-Cross-Domain-Policies: none
09:13:45.550 -> Referrer-Policy: strict-origin-when-cross-origin
09:13:45.550 -> Access-Control-Allow-Origin: *
09:13:45.550 -> ETag: W/"bf6a136cc1847d88e5896809c63b1aeb"
09:13:45.550 -> Cache-Control: max-age=0, private, must-revalidate
09:13:45.550 -> X-Request-Id: ad939d58-7471-4458-8612-28595b3fc057
09:13:45.584 -> X-Runtime: 0.067761
09:13:45.584 -> 
09:13:45.584 -> 20
09:13:45.584 -> Measurement created successfully
09:13:45.584 -> 0
09:13:45.584 -> 
09:13:45.798 -> 
09:13:45.831 -> OBS:Posted
09:13:45.865 -> FS->PUB OK
09:13:45.865 -> N2S Publish
09:13:45.900 -> LOOP 66:893005872:0:0
09:13:54.909 -> GPRS.isAccessAlive(): 1
09:13:54.909 -> Connected to Cellular Network
09:14:05.881 -> GPRS.isAccessAlive(): 1
09:14:05.881 -> Connected to Cellular Network
09:14:16.877 -> GPRS.isAccessAlive(): 1
09:14:16.877 -> Connected to Cellular Network
09:14:27.836 -> GPRS.isAccessAlive(): 1
09:14:27.836 -> Connected to Cellular Network
09:14:38.848 -> GPRS.isAccessAlive(): 1
09:14:38.848 -> Connected to Cellular Network
09:14:45.835 -> OBS_DO()
09:14:45.873 -> OBS_TAKE()
09:14:45.977 -> EEPROM RT UPDATED
09:14:46.436 -> MUX:OBSDO
09:14:49.628 -> OBS_BUILD()
09:14:49.665 -> OBS->URL
09:14:49.665 -> {"key":"XXXXXXXX","devid":"8120067b801e913404027822","instrument_id":53,"at":"2026-03-17T15:14:45","css":29,"hth":0,"rg1":0.0,"rgt1":0.2,"rgp1":26.0,"vbv":1.9,"vpc":67.7,"ws":0.0,"wd":88,"wg":0.0,"wgd":-999,"bp1":840.7,"bt1":20.3,"st1":19.7,"sh1":40.8,"mt1":19.3,"vlx":147.2,"hi":-999.9,"wbt":-3.9,"wbgt":-999.9,"mslp":840.7,"dst0":19.9,"dst1":20.1,"dst4":20.2,"dst7":20.2}
09:14:49.701 -> OBS->SD
09:14:49.701 -> /OBS/20260317.log
09:14:49.775 -> OBS Logged to SD
09:14:49.775 -> OBS_SEND()
09:14:49.842 -> +CCLK: "26/03/17,09:14:49-24"
09:14:49.877 -> GNWT:OK[1773760489]
09:14:49.877 -> OBS:SEND->HTTP
09:14:50.817 -> OBS:HTTP CONNECTED
09:14:50.817 -> /measurements/url_create?key=XXXXXXXX&devid=8120067b801e913404027822&instrument_id=53&at=2026-03-17T15%3A14%3A45&css=29&hth=0&rg1=0.0000&rgt1=0.2000&rgp1=26.0000&vbv=1.9000&vpc=67.7000&ws=0.0000&wd=88&wg=0.0000&wgd=-999&bp1=840.7000&bt1=20.3000&st1=19.7000&sh1=40.8000&mt1=19.3000&vlx=147.2000&hi=-999.9000&wbt=-3.9000&wbgt=-999.9000&mslp=840.7000&dst0=19.9000&dst1=20.1000&dst4=20.2000&dst7=20.2000
09:14:51.352 -> OBS:HTTP SENT
09:14:51.389 -> GPRS.isAccessAlive(): 1
09:14:51.389 -> Connected to Cellular Network
09:14:52.527 -> OBS:HTTP WAIT
09:14:52.565 -> OBS:HTTP RESP[HTTP/1.1 200 OK]
09:14:52.565 -> 48 54 54 50 2F 31 2E 31 20 32 30 30 20 4F 4B 
09:14:52.565 -> Server: nginx
09:14:52.565 -> Date: Tue, 17 Mar 2026 15:14:51 GMT
09:14:52.565 -> Content-Type: text/html; charset=utf-8
09:14:52.565 -> Transfer-Encoding: chunked
09:14:52.565 -> Connection: close
09:14:52.565 -> X-Frame-Options: SAMEORIGIN
09:14:52.565 -> X-XSS-Protection: 1; mode=block
09:14:52.565 -> X-Content-Type-Options: nosniff
09:14:52.565 -> X-Download-Options: noopen
09:14:52.565 -> X-Permitted-Cross-Domain-Policies: none
09:14:52.565 -> Referrer-Policy: strict-origin-when-cross-origin
09:14:52.565 -> Access-Control-Allow-Origin: *
09:14:52.565 -> ETag: W/"bf6a136cc1847d88e5896809c63b1aeb"
09:14:52.565 -> Cache-Control: max-age=0, private, must-revalidate
09:14:52.565 -> X-Request-Id: 348448f3-9c04-4fd0-8267-a3b3ad771764
09:14:52.600 -> X-Runtime: 0.069287
09:14:52.600 -> 
09:14:52.600 -> 20
09:14:52.600 -> Measurement created successfully
09:14:52.600 -> 0
09:14:52.600 -> 
09:14:52.810 -> 
09:14:52.848 -> OBS:Posted
09:14:52.848 -> FS->PUB OK
09:14:52.882 -> N2S Publish
09:14:52.916 -> LOOP 133:893005812:0:0
09:15:01.921 -> GPRS.isAccessAlive(): 1
09:15:01.921 -> Connected to Cellular Network
09:15:12.903 -> GPRS.isAccessAlive(): 1
09:15:12.903 -> Connected to Cellular Network
09:15:23.856 -> GPRS.isAccessAlive(): 1
09:15:23.856 -> Connected to Cellular Network
09:15:33.860 -> GPRS.isAccessAlive(): 1
09:15:33.860 -> Connected to Cellular Network
09:15:44.849 -> GPRS.isAccessAlive(): 1
09:15:44.849 -> Connected to Cellular Network
09:15:52.851 -> OBS_DO()
09:15:52.889 -> OBS_TAKE()
09:15:52.996 -> EEPROM RT UPDATED
09:15:53.454 -> MUX:OBSDO
09:15:56.655 -> OBS_BUILD()
09:15:56.693 -> OBS->URL
09:15:56.693 -> {"key":"XXXXXXXX","devid":"8120067b801e913404027822","instrument_id":53,"at":"2026-03-17T15:15:52","css":29,"hth":0,"rg1":0.0,"rgt1":0.2,"rgp1":26.0,"vbv":1.9,"vpc":67.9,"ws":0.0,"wd":88,"wg":0.0,"wgd":-999,"bp1":840.7,"bt1":20.3,"st1":19.8,"sh1":40.9,"mt1":19.4,"vlx":151.0,"hi":-999.9,"wbt":-3.9,"wbgt":-999.9,"mslp":840.7,"dst0":19.9,"dst1":20.2,"dst4":20.2,"dst7":20.3}
09:15:56.693 -> OBS->SD
09:15:56.728 -> /OBS/20260317.log
09:15:56.764 -> OBS Logged to SD
09:15:56.798 -> OBS_SEND()
09:15:56.834 -> +CCLK: "26/03/17,09:15:56-24"
09:15:56.867 -> GNWT:OK[1773760556]
09:15:56.904 -> OBS:SEND->HTTP
09:15:57.724 -> OBS:HTTP CONNECTED
09:15:57.724 -> /measurements/url_create?key=XXXXXXXX&devid=8120067b801e913404027822&instrument_id=53&at=2026-03-17T15%3A15%3A52&css=29&hth=0&rg1=0.0000&rgt1=0.2000&rgp1=26.0000&vbv=1.9000&vpc=67.9000&ws=0.0000&wd=88&wg=0.0000&wgd=-999&bp1=840.7000&bt1=20.3000&st1=19.8000&sh1=40.9000&mt1=19.4000&vlx=151.0000&hi=-999.9000&wbt=-3.9000&wbgt=-999.9000&mslp=840.7000&dst0=19.9000&dst1=20.2000&dst4=20.2000&dst7=20.3000
09:15:58.234 -> OBS:HTTP SENT
09:15:58.303 -> GPRS.isAccessAlive(): 1
09:15:58.303 -> Connected to Cellular Network
09:15:59.424 -> OBS:HTTP WAIT
09:15:59.459 -> OBS:HTTP RESP[HTTP/1.1 200 OK]
09:15:59.459 -> 48 54 54 50 2F 31 2E 31 20 32 30 30 20 4F 4B 
09:15:59.459 -> Server: nginx
09:15:59.459 -> Date: Tue, 17 Mar 2026 15:15:58 GMT
09:15:59.459 -> Content-Type: text/html; charset=utf-8
09:15:59.459 -> Transfer-Encoding: chunked
09:15:59.459 -> Connection: close
09:15:59.459 -> X-Frame-Options: SAMEORIGIN
09:15:59.459 -> X-XSS-Protection: 1; mode=block
09:15:59.459 -> X-Content-Type-Options: nosniff
09:15:59.459 -> X-Download-Options: noopen
09:15:59.459 -> X-Permitted-Cross-Domain-Policies: none
09:15:59.459 -> Referrer-Policy: strict-origin-when-cross-origin
09:15:59.459 -> Access-Control-Allow-Origin: *
09:15:59.459 -> ETag: W/"bf6a136cc1847d88e5896809c63b1aeb"
09:15:59.459 -> Cache-Control: max-age=0, private, must-revalidate
09:15:59.459 -> X-Request-Id: 704b1bdc-215c-4383-9411-759dbb4737de
09:15:59.497 -> X-Runtime: 0.074500
09:15:59.497 -> 
09:15:59.497 -> 20
09:15:59.497 -> Measurement created successfully
09:15:59.497 -> 0

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
