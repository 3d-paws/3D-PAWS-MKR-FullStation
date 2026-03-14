### System Health Bits
[←Top](../README.md)<BR>
A register is maintained where its individual bits correspond to various state information and sensor state. This register is reported in each observation as a decimal number with tag id "hth".<BR>
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 120px; border: 1px solid black; padding: 10px;">
<pre>
Name       Bit   HEX  Decimal
PWRON      00001 0x1        1  Set at power on, but cleared after first observation
SD         00010 0x2        2  Set if SD missing at boot or other SD related issues
N2S        00100 0x4        4  Set when Need to Send observations exist
FROM_N2S   01000 0x8        8  Set in transmitted N2S observation when finally transmitted
RTC        10000 0x10      16  Set if RTC missing at boot
</pre>
</div><BR>
Example "hth" values reported. This is often what people actually need when decoding logs:
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 120px; border: 1px solid black; padding: 10px;">
<pre>
Decimal Meaning
1       Just powered on
3       Power on + SD issue (1 + 2)
5       Power on + N2S pending (1 + 4)
17      Power on + RTC missing (1 + 16)
20      RTC missing + N2S pending (16 + 4)
</pre>
</div>
<br>
Status bits for v42 and before releases - Many of the below will be cleared after the first observation is made after startup.
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 380px; border: 1px solid black; padding: 10px;">
<pre>
#define SSB_PWRON           0x1       // Set at power on, but cleared after first observation
#define SSB_SD              0x2       // Set if SD missing at boot or other SD related issues
#define SSB_RTC             0x4       // Set if RTC missing at boot
#define SSB_OLED            0x8       // Set if OLED missing at boot, but cleared after first observation
#define SSB_N2S             0x10      // Set when Need to Send observations exist
#define SSB_FROM_N2S        0x20      // Set in transmitted N2S observation when finally transmitted
#define SSB_AS5600          0x40      // Set if wind direction sensor AS5600 has issues
#define SSB_BMX_1           0x80      // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_BMX_2           0x100     // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_HTU21DF         0x200     // Set if Humidity & Temp Sensor missing
#define SSB_SI1145          0x400     // Set if UV index & IR & Visible Sensor missing
#define SSB_MCP_1           0x800     // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_MCP_2           0x1000    // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_MCP_3           0x2000    // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_LORA            0x4000    // Set if LoRa Radio missing at startup
#define SSB_SHT_1           0x8000    // Set if SHTX1 Sensor missing
#define SSB_SHT_2           0x10000   // Set if SHTX2 Sensor missing
#define SSB_HIH8            0x20000   // Set if HIH8000 Sensor missing
#define SSB_VLX             0x40000   // Set if VEML7700 Sensor missing
#define SSB_PM25AQI         0x80000   // Set if PM25AQI Sensor missing
#define SSB_HDC_1           0x100000  // Set if HDC302x I2C Temperature Sensor missing
#define SSB_HDC_2           0x200000  // Set if HDC302x I2C Temperature Sensor missing
#define SSB_BLX             0x400000  // Set if BLUX30 I2C Sensor missing
#define SSB_LPS_1           0x800000  // Set if LPS35HW I2C Sensor missing
#define SSB_LPS_2           0x1000000 // Set if LPS35HW I2C Sensor missing
#define SSB_TLW             0x2000000 // Set if Tinovi Leaf Wetness I2C Sensor missing
#define SSB_TSM             0x4000000 // Set if Tinovi Soil Moisture I2C Sensor missing
#define SSB_TMSM            0x8000000 // Set if Tinovi MultiLevel Soil Moisture I2C Sensor missing
</pre>
</div>