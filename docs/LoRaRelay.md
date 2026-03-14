# LoRa Remote Relay
[←Top](../README.md)<BR>
When LoRa is enabled on the station by adding the LoRa Module and configuring CONFIG.TXT on the SD card. LoRa messages from remote LoRa stations are received and stored in memory. Up to 64 messages. At the next transmit windows these observations are sent to Particle. They will be added to the N2S file when necessary.

For information on LoRa Remote stations see [3D-PAWS-Feather-LoRaRemoteM0](https://github.com/3d-paws/3D-PAWS-Feather-LoRa-Remote) code base.

### LoRa message type received for relay are:

- "INFO" : Sent at boot providing station configuration information
- "LR" : LoRa Relay message containing observation information.

These type are sent to Particle as event types "INFO" and "LR". Where Particle Integrations (Webhooks) take over the processsing.

### Tibits of Information:
- LoRa messages are AES 128 bit encrypted.
- LoRa messages are in JSON format
- The Remote Lora device messages will include a device id (devid) and a CHORDS station id (id). 

### LoRa Module
For the Full Station to receive LoRa messages a LoRa Module needs to be added. 
[Adafruit RFM95W LoRa Radio Transceiver Breakout - 868 or 915 MHz](https://www.adafruit.com/product/3072)

### Muon Interconnect Wiring
| Pin | Description   | LoRa Module                |
| --- |-------------- | -------------------------- |
| 8   | MOSI D9  SPI1 | MOSI (Master Out Slave In) |
| 10  | MISO D10 SPI1 | MISO (Master In Slave Out) |
| 11  | SCK  D2  SPI1 | SCK                        |
| 36  | CS   D3  SPI1 | CS ChipSel                 |
| 38  | RST  D21      | RST Reset                  |
| 40  |      D20      | G0 IRQ                     |

### Boron/Argon Interconnect Wiring
| Pin | Description | LoRa Module                |
| --- |------------ | -------------------------- |
| D4  | SPI1 MISO   | MOSI (Master Out Slave In) |
| D3  | SPI1 MOSI   | MOSI (Master Out Slave In) |
| D2  | SPI1 SCK    | SCK                        |
| D9  | ChipSel     | CS ChipSel                 |
| D10 | Reset       | RST Reset                  |
| D6  | DIO/IRQ     | G0 IRQ                     |

---

## SD Configuration file CONFIG.TXT
The aes_pkey and aes_myiv must match the Remote Lora device configuration
<pre>
#
# CONFIG.TXT
#
# Line Length is limited to 63 characters
#12345678901234567890123456789012345678901234567890123456789012

# Private Key - 128 bits (16 bytes of ASCII characters)
aes_pkey=10FE2D3C4B5A6978

# Initialization Vector must be and always will be 128 bits (16 bytes.)
# The real iv is actually myiv repeated twice
# 1234567 -> 0x12D687 = 0x00 0x12 0xD6 0x87 0x00 0x12 0xD6 0x87 
aes_myiv=1234567

# This unit's LoRa ID for Receiving and Sending Messages
lora_unitid=1

# You can set transmitter power from 5 to 23 dBm
lora_txpower=5

# Valid entries are 433, 866, 915
lora_freq=915
</pre>

### LoRa Antenna Length
The recommended antenna wire lengths for Adafruit LoRa Feather boards are frequency-dependent quarter-wave whip antennas:
- For 433 MHz, the antenna length should be 6.5 inches (16.5 cm).
- For 868 MHz, the antenna length should be 3.25 inches (8.2 cm).
- For 915 MHz, the antenna length should be 3 inches (7.8 cm).
