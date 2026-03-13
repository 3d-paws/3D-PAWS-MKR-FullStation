# Modem Firmware Upgrading MKR NB 1500
[←Top](../README.md)<BR>
### Firmware Update
Hopefully you will never need to do this. We are seeing with the latest shipped MKR NB 1500 boards we they are coming with the most current firmware. 

- UBLOX Firmware (2022-06-17 Modem: L0.0.00.00.05.12, Application: A.02.21)

## Obtaining Latest Firmware
It took quite the effort to obtain the firmware and infrastructure to be able to do updates. We are legally not allowed to share the firmware.  

To legally obtain u-blox SARA firmware, you must go through their controlled distribution process, as it's not publicly available due to proprietary licensing.

### Registration Process
Register your project on the u-blox customer portal at portal.u-blox.com, providing details about your use case (e.g., module model like SARA-R4/R5, intended application). 
This often requires an NDA or project approval before access is granted. 
[portal.u-blox](https://portal.u-blox.com/s/question/0D52p00008HKEAqCAP/sarag350-firmware-update-procedure)

### Access and Download
Once approved, log in to download firmware files (.dof or similar) and tools like EasyFlash from the SARA product resources section. 
Board manufacturers (e.g., SparkFun, Arduino) may provide links, but direct u-blox access is primary. 
[learn.sparkfun](https://learn.sparkfun.com/tutorials/micromod-update-tool-hookup-guide/software-setup)

### Update Methods
Use approved tools post-download:
- **EasyFlash**: Connect via UART/USB, select SARA model and COM port, power cycle module during flash. 
[mouser](https://www.mouser.com/pdfDocs/LEXI-R520-SARA-R5-FW-Update_AppNote_UBX-20033314.pdf)
- **FOAT/uFOTA**: AT commands like +UFWUPD for tethered/OTA updates (requires host or server). 
[mouser](https://www.mouser.com/pdfDocs/LEXI-R520-SARA-R5-FW-Update_AppNote_UBX-20033314.pdf)

Contact your u-blox FAE, sales rep, or support case for model-specific files if portal access stalls. [portal.u-blox](https://portal.u-blox.com/s/question/0D52p0000CBnir6CQB/where-is-the-firmware-to-update-sarar410)

Here’s an updated, clearer and more polished version of your firmware update procedure. I’ve standardized formatting, added clarifications, and improved readability without changing the technical content.  


## Firmware Update Procedure for MKR NB 1500 (SARA-R410M Module)

1. **Install Qualcomm USB Drivers**  
   - Ensure the drivers are properly installed before connecting your board.  
   - You should see a *Qualcomm HS-USB* entry appear under *Ports (COM & LPT)* in Windows Device Manager after installation.

2. **Install EasyFlash Tool**  
   - After installation, copy the separately downloaded firmware file (e.g., **xxx.dof**) into EasyFlash’s working directory.  
   - Before launching, confirm the firmware file is in the same folder as the EasyFlash executable.  
   - Use a **short, direct USB cable** (avoid USB hubs).  
   - If flashing does not begin, **run EasyFlash as Administrator**.

3. **Connect Antenna to MKR NB 1500**  
   - The SARA-R410M modem may not respond to AT commands unless an antenna is connected.

4. **Connect MKR NB 1500 to Computer via USB**

5. **Load the Arduino Sketch**  
   - Open: **File → Examples → MKRNB → Tools → SerialSARAPassthrough**  
   - Once uploaded, open the Serial Monitor and test with:  
     - `ATI9` → should return `L0.0.00.00.05.06,A.02.00`  
     - `AT+GMM` → should return `SARA-R410M-02B`

6. **Prepare for Firmware Flashing**  
   - Unplug the MKR board from USB.  
   - Connect the MKR board to the programming board.  
   - Connect the computer’s USB to the **programming board** (not the MKR directly).

7. **Verify Connection in Windows**  
   - Open *Computer Management → Device Manager → Ports (COM & LPT)*.  
   - Confirm the **Qualcomm HS-USB** device is visible.  
   - Note the **COM port number**.

8. **Launch EasyFlash (v13.03.1.2)**  
   - Select the appropriate firmware file (e.g., `.dor`).  
   - Set:  
     - **Device**: SARA-R4  
     - **COM Port**: (from step 7)  
     - **Baud Rate**: 115200

9. **Start the Flashing Process**  
   - Click **Start** to begin.  
   - Wait for the process to complete successfully.

10. **Finalize**  
   - Unplug the USB cable and remove the MKR board from the programming board.

11. **Verify Firmware Update**  
   - Reconnect the MKR NB 1500 to your computer via USB.  
   - Run **SerialSARAPassthrough** again and check:  
     - `ATI9` → `L0.0.00.00.05.12,A.02.21`  
     - `AT+GMR` → `L0.0.00.00.05.12 [Mar 09 2022 17:00:00]`  
     - `AT+GMM` → `SARA-R410M-02B`
	
### Links that were collected along the way
- https://content.u-blox.com/system/files/EasyFlash_13.03.1.2.zip?hash=rG0hfY558j8FTMU3NbbeXx8nWrm1wAaRLFzrC_sUsZo (Easyflash Software)
- https://support.arduino.cc/hc/en-us/articles/360016119159-How-to-check-the-firmware-version-the-SARA-radio-module-u-blox-on-the-MKR-NB-1500
- https://docs.particle.io/reference/technical-advisory-notices/tan001-sara-r410m-124-day/
- https://portal.u-blox.com/s/feed/0D52p0000CUPR6qCQH
- https://www.u-blox.com/en/product/sara-r4-series
- https://www.u-blox.com/en/product/sara-r4-series#tab-documentation-resources
- https://www.u-blox.com/sites/default/files/SARA-R4-FW-Update_AppNote_UBX-17049154.pdf
- https://www.u-blox.com/en/product/m-center
- https://www.ebay.com/itm/185498266481?chn=ps&norover=1&mkevt=1&mkrid=711-117182-37290-0&mkcid=2&itemid=185498266481&targetid=1263433205254
- https://github.com/hologram-io/hologram-tools/tree/master/novaupdate
