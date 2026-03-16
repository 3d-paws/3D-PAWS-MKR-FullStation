/*
 * ======================================================================================================================
 * sdcard.cpp - SD Card https://store-usa.arduino.cc/products/mkr-sd-proto-shield
 *    MKR SD Board Pins
 *        CS  4
 *        CLK 9  SCK
 *        DI  8  MOSI
 *        DO  10 MISO
 * ======================================================================================================================
 */
#include <SdFat.h>

#include "include/ssbits.h"
#include "include/eeprom.h"
#include "include/time.h"
#include "include/main.h"
#include "include/cf.h"
#include "include/output.h"
#include "include/network.h"
#include "include/lora.h"
#include "include/obs.h"
#include "include/sdcard.h"

SdFat SD;                                   // File system object.
File SD_fp;
char SD_obsdir[] = "/OBS";                  // Observations stored in this directory. Created at power on if not exist
bool SD_exists = false;                     // Set to true if SD card found at boot
char SD_n2s_file[] = "N2SOBS.TXT";          // Need To Send Observation file
uint32_t SD_n2s_max_filesz = 512 * 60 * 48; // Keep a little over 1 day. When it fills, it is deleted and we start over.

char SD_crt_file[] = "CRT.TXT";         // if file exists clear rain totals and delete file

char SD_INFO_FILE[] = "INFO.TXT";       // Store INFO information in this file. Every INFO call will overwrite content


/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * SD_initialize()
 *=======================================================================================================================
 */
void SD_initialize() {

  if (!SD.begin(SD_ChipSelect)) {
    Output ("SD:NF");
    SystemStatusBits |= SSB_SD;
    delay (5000);
  }
  else {
    if (!SD.exists(SD_obsdir)) {
      if (SD.mkdir(SD_obsdir)) {
        Output (F("SD:MKDIR OBS OK"));
        Output (F("SD:Online"));
        SD_exists = true;
      }
      else {
        Output (F("SD:MKDIR OBS ERR"));
        Output (F("SD:Offline"));
        SystemStatusBits |= SSB_SD;  // Turn On Bit     
      } 
    }
    else {
      Output (F("SD:Online"));
      Output (F("SD:OBS DIR Exists"));
      SD_exists = true;
    }
  }
}

/* 
 *=======================================================================================================================
 * SD_LogObservation()
 *=======================================================================================================================
 */
void SD_LogObservation(char *observations) {
  char SD_logfile[24];
  File fp;

  time_t ts = stc.getEpoch();
  tm *dt = gmtime(&ts); 
    
  if (!SD_exists) {
    Output (F("SD:NOT EXIST"));
    return;
  }

  if (!STC_valid) {
    Output (F("SD:STC NOT VALID"));
    return;
  }
  
  sprintf (SD_logfile, "%s/%4d%02d%02d.log", SD_obsdir, dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday);
  Output (SD_logfile);
   
  fp = SD.open(SD_logfile, FILE_WRITE); 
  if (fp) {
    fp.println(observations);
    fp.close();
    SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
    Output (F("OBS Logged to SD"));
  }
  else {
    SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
    Output (F("SD:Open(Log)ERR"));
    // At thins point we could set SD_exists to false and/or set a status bit to report it
    // sd_initialize();  // Reports SD NOT Found. Library bug with SD
  }
}

/* 
 *=======================================================================================================================
 * SD_N2S_Delete()
 *=======================================================================================================================
 */
bool SD_N2S_Delete() {
  bool result;

  if (SD_exists && SD.exists(SD_n2s_file)) {
    if (SD.remove (SD_n2s_file)) {
      SystemStatusBits &= ~SSB_N2S; // Turn Off Bit
      Output (F("N2S->DEL:OK"));
      result = true;
    }
    else {
      Output (F("N2S->DEL:ERR"));
      SystemStatusBits |= SSB_SD; // Turn On Bit
      result = false;
    }
  }
  else {
    SystemStatusBits &= ~SSB_N2S; // Turn Off Bit
    Output (F("N2S->DEL:NF"));
    result = true;
  }
  eeprom.n2sfp = 0;
  EEPROM_Update();
  return (result);
}

/* 
 *=======================================================================================================================
 * SD_NeedToSend_Add()
 *=======================================================================================================================
 */
void SD_NeedToSend_Add(char *observation) {
  File fp;

  if (!SD_exists) {
    return;
  }
  
  fp = SD.open(SD_n2s_file, FILE_WRITE); // Open the file for reading and writing, starting at the end of the file.
                                         // It will be created if it doesn't already exist.
  if (fp) {  
    if (fp.size() > SD_n2s_max_filesz) {
      fp.close();
      Output (F("N2S:Full"));
      if (SD_N2S_Delete()) {
        // Only call ourself again if we truely deleted the file. Otherwise infinate loop.
        SD_NeedToSend_Add(observation); // Now go and log the data
      }
    }
    else {
      fp.println(observation); //Print data, followed by a carriage return and newline, to the File
      fp.close();
      SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
      SystemStatusBits |= SSB_N2S; // Turn on Bit that says there are entries in the N2S File
      Output (F("N2S:OBS Added"));
    }
  }
  else {
    SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
    Output (F("N2S:Open Error"));
    // At thins point we could set SD_exists to false and/or set a status bit to report it
    // sd_initialize();  // Reports SD NOT Found. Library bug with SD
  }
}

/* 
 * =======================================================================================================================
 * SD_NeedToSend_Status() - Send back the state 
 * =======================================================================================================================
 */
void SD_NeedToSend_Status(char *status) {

  if (SD_exists) {
    if (SD.exists(SD_n2s_file)) {
      File fp = SD.open(SD_n2s_file, FILE_WRITE);
      if (fp) {
        sprintf (status, "%d", fp.size());
        fp.close();
      }
      else {
        sprintf (status, "-1");
      }
    }
    else {
      sprintf (status, "\"NF\"");
    }
  }
  else {
    sprintf (status, "\"!SD\"");
  }
}

/* 
 *=======================================================================================================================
 * SD_ClearRainTotals() -- If CRT.TXT exists on SD card clear rain totals - Checked at Boot
 *=======================================================================================================================
 */
void SD_ClearRainTotals() {
  if (STC_valid && SD_exists) {
    if (SD.exists(SD_crt_file)) {
      if (SD.remove (SD_crt_file)) {
        Output (F("CRT:OK-CLR"));
        uint32_t current_time = stc.getEpoch();
        EEPROM_ClearRainTotals(current_time);
        EEPROM_Dump();
      }
      else {
        Output (F("CRT:ERR-RM"));
      }
    }
    else {
      Output (F("CRT:OK-NF"));
    }
  }
  else {
    Output (F("CRT:ERR-CLK"));
  }
}

/* 
 * =======================================================================================================================
 * SD_UpdateInfoFile() - Update INFO.TXT file 
 * =======================================================================================================================
 */
void SD_UpdateInfoFile(char *info) {
  // Update INFO.TXT file
  if (SD_exists) {
    File fp = SD.open(SD_INFO_FILE, FILE_WRITE | O_TRUNC); 
    if (fp) {
      fp.println(info);
      fp.close();
      SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
      Output ("INFO->SD OK");
      // Output ("INFO Logged to SD");
    }
    else {
      SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
      Output ("SD:Open(Info)ERR");
    }
  }
  else {
    Output ("INFO->!SD");
  }
}

/* 
 *=======================================================================================================================
 * SD_N2S_Publish()
 *=======================================================================================================================
 */
void SD_N2S_Publish() {
  File fp;
  char ch;
  int i;
  int sent=0;

  memset(obsbuf, 0, sizeof(obsbuf));

  Output (F("N2S Publish"));

  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  
  if (SD_exists && SD.exists(SD_n2s_file)) {
    Output (F("N2S:Exists"));

    fp = SD.open(SD_n2s_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

    if (fp) {
      // Delete Empty File or too small of file to be valid
      if (fp.size()<=20) {
        fp.close();
        Output (F("N2S:Empty"));
        SD_N2S_Delete();
      }
      else {
        if (eeprom.n2sfp) {
          if (fp.size()<=eeprom.n2sfp) {
            // Something wrong. Can not have a file position that is larger than the file
            eeprom.n2sfp = 0; 
          }
          else {
            fp.seek(eeprom.n2sfp);  // Seek to where we left off last time. 
          }
        } 

        // Loop through each line / obs and transmit
        
        // set timer on when we need to stop sending n2s obs
        unsigned long  TimeFromNow;
        if (cf_obs_period == 1) {
          TimeFromNow = time_to_next_obs() - (15 * 1000); // stop sending 15s before next observation period if 1m obs
        }
        else {
          TimeFromNow = time_to_next_obs() - (60 * 1000); // stop sending 1m before next observation period if not 1m obs
        }

        i = 0;
        while (fp.available() && (i < MAX_HTTP_SIZE )) {
          ch = fp.read();

          if (ch == 0x0A) {
            
            Serial_writeln (obsbuf);
            
            if (Send_http(obsbuf, cf_webserver, cf_webserver_port, cf_urlpath, METHOD_GET, cf_apikey)) {
              sprintf (Buffer32Bytes, "N2S[%d]->PUB:OK", sent++);
              Output (Buffer32Bytes);
              //Serial_writeln (obsbuf);

              // setup for next line in file
              i = 0;

              // file position is at the start of the next observation or at eof
              eeprom.n2sfp = fp.position();

              BackGroundWork();
              sprintf (Buffer32Bytes, "N2S[%d] Contunue", sent);
              Output (Buffer32Bytes); 

              if(millis() > TimeFromNow) {
                // need to break out so new obs can be made
                Output (F("N2S->TIME2EXIT"));
                break;                
              }
            }
            else {
                sprintf (Buffer32Bytes, "N2S[%d]->PUB:ERR", sent);
                Output (Buffer32Bytes);
                // On transmit failure, stop processing file.
                break;
            }
            
            // At this point file pointer's position is at the first character of the next line or at eof
            
          } // Newline
          else if (ch == 0x0D) { // CR, LF follows and will trigger the line to be processed       
            obsbuf[i] = 0; // null terminate then wait for newline to be read to process OBS
          }
          else {
            obsbuf[i++] = ch;
          }

          // Check for buffer overrun
          if (i >= MAX_HTTP_SIZE) {
            sprintf (Buffer32Bytes, "N2S[%d]->BOR:ERR", sent);
            Output (Buffer32Bytes);
            fp.close();
            SD_N2S_Delete(); // Bad data in the file so delete the file           
            return;
          }
        } // end while 

        if (fp.available() <= 20) {
          // If at EOF or some invalid amount left then delete the file
          fp.close();
          SD_N2S_Delete();
        }
        else {
          // At this point we sent 0 or more observations but there was a problem.
          // eeprom.n2sfp was maintained in the above read loop. So we will close the
          // file and next time this function is called we will seek to eeprom.n2sfp
          // and start processing from there forward. 
          fp.close();
          EEPROM_Update(); // Update file postion in the eeprom.
        }
      }
    }
    else {
        Output (F("N2S->OPEN:ERR"));
    }
  }
}


