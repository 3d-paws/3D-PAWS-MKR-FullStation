# Station Information - (Particle Message type "INFO")
[←Top](../README.md)<BR>
Station Information is generated, stored on the SD card and transmitted with every boot and then every 6 hours from boot.

INFO messages are handle differently between WiFi and LoRaWAN.

CHORDS can not accept INFO messages.  An alternative site is used to log the messages.  Configuration for this site is setup in the CONFIG.TXT file. Here are those configuration settings.

```
# Information web server, Not Chords
info_server=some.domain.com
info_server_port=80
info_urlpath=/info/log.php
# Will be added to the HTML Header as X-API-Key
info_apikey=1234
```

### INFO Event Eessage
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 625px; border: 1px solid black; padding: 10px;">
<pre>
{
  "MT": "INFO",
  "at": "2026-03-15T01:50:53",
  "devid": "8120067b801e913404027822",
  "board": "AFM0WiFi",
  "ver": "MKRFS-260312",
  "bcs": "!CHARGING",
  "hth": 1,
  "elev": 0,
  "rtro": 0,
  "ls": "3d.chordsrt.com",
  "lsp": 80,
  "lsurl": "/measurements/url_create",
  "lsapi": "APIKEY",
  "lsid": 1,
  "obsi": "1m",
  "t2nt": "4294907s",
  "drbt": "22m",
  "n2s": 337,
  "devs": "rtc, sd, eeprom, mux, dsmux, oled(32)",
  "sensors": "BMX1(BMP390), MCP1, SHT1, VEML, WIND, WS(D0), AS5600, DST(0,1,4,7), HI, WBT, WBGT WO/GLOBE, RG1(D1), VBV(A2)"
}

bcs = battery charging status
bpc = battery percent charge
css = cell signal strength
csq = cell signal quality
imsi = international mobile subscriber identity
obsi = observation interval (seconds)
obsti = observation transmit interval (minutes)
t2nt = time to next transmit (seconds)
drct = daily reboot countdown timer (counter)
sce = serial console enabled
scepin = status of the actual serial console pin
op1 = configuration of this pin (DIST 5M, DIST 10M, RG2, RAW, NS[Not Set])
op1 = configuration of this pin (RAW, VBV[Voltaic Battery Voltage], NS[Not Set])
dsmux = dallas sensor i2c to 1-wire mux
dst = dallas sensor temperature (dst0-8)
</pre>
</div>