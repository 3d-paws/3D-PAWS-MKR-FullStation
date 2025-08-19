/*
 * ======================================================================================================================
 *  Output.h - OLED and Serial Console Initialization
 * ======================================================================================================================
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void HeartBeat(); // Prototype this function to aviod compile function unknown issue.

/*
 * ======================================================================================================================
 *  OLED Display 
 * ======================================================================================================================
 */
#define SCREEN_WIDTH        128 // OLED display width, in pixels
#define OLED32_I2C_ADDRESS  0x3C // 128x32 - https://www.adafruit.com/product/4440
#define OLED64_I2C_ADDRESS  0x3D // 128x64 - https://www.adafruit.com/product/326
#define OLED_RESET          -1 // -1 = Not in use, but could share LoRa reset ping A5
#define OLED32              (oled_type == OLED32_I2C_ADDRESS)
#define OLED64              (oled_type == OLED64_I2C_ADDRESS)

bool DisplayEnabled = true;
int  oled_type = 0;
char oled_lines[8][23];
Adafruit_SSD1306 display32(SCREEN_WIDTH, 32, &Wire, OLED_RESET);
Adafruit_SSD1306 display64(SCREEN_WIDTH, 64, &Wire, OLED_RESET);


/*
 * ======================================================================================================================
 * OLED_sleepDisplay()
 * ======================================================================================================================
 */
void OLED_sleepDisplay() {
  if (DisplayEnabled) {
    if (OLED32) {
      display32.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    else {
      display64.ssd1306_command(SSD1306_DISPLAYOFF);
    }
  }
}

/*
 * ======================================================================================================================
 * OLED_wakeDisplay()
 * ======================================================================================================================
 */
void OLED_wakeDisplay() {
  if (DisplayEnabled) {
    if (OLED32) {
      display32.ssd1306_command(SSD1306_DISPLAYON);
    }
    else {
      display64.ssd1306_command(SSD1306_DISPLAYON);
    }
  }
}

/*
 * ======================================================================================================================
 * OLED_spin() 
 * ======================================================================================================================
 */
void OLED_spin() {
  static int spin=0;
    
  if (DisplayEnabled) {
    if (OLED32) {
      display32.setTextColor(WHITE, BLACK); // Draw 'inverse' text
      display32.setCursor(120,24);
      display32.print(" ");
      display32.setCursor(120,24); 
    }
    else {
      display64.setTextColor(WHITE, BLACK); // Draw 'inverse' text
      display64.setCursor(120,24);
      display64.print(" ");
      display64.setCursor(120,24);
      display64.setCursor(120,56);
      display64.print(" ");
      display64.setCursor(120,56);       
    } 
    switch (spin++) {
      case 0 : msgp = (char *) "|"; break;
      case 1 : msgp = (char *) "/"; break;
      case 2 : msgp = (char *) "-"; break;
      case 3 : msgp = (char *) "\\"; break;
    }
    if (OLED32) {
      display32.print(msgp);
      display32.display();
    }
    else {
      display64.print(msgp);
      display64.display();
    }
    spin %= 4;
  }
}

/*
 * ======================================================================================================================
 * OLED_update() -- Output oled in memory map to display
 * ======================================================================================================================
 */
void OLED_update() {  
  if (DisplayEnabled) {
    if (OLED32) {
      display32.clearDisplay();
      display32.setCursor(0,0);             // Start at top-left corner
      display32.print(oled_lines [0]);
      display32.setCursor(0,8);
      display32.print(oled_lines [1]);
      display32.setCursor(0,16);
      display32.print(oled_lines [2]);
      display32.setCursor(0,24);  
      display32.print(oled_lines [3]);
      display32.display();
    }
    else {
      display64.clearDisplay();
      display64.setCursor(0,0);             // Start at top-left corner
      display64.print(oled_lines [0]);
      display64.setCursor(0,8);
      display64.print(oled_lines [1]);
      display64.setCursor(0,16);
      display64.print(oled_lines [2]);
      display64.setCursor(0,24);  
      display64.print(oled_lines [3]);
      display64.setCursor(0,32);  
      display64.print(oled_lines [4]);
      display64.setCursor(0,40);  
      display64.print(oled_lines [5]);
      display64.setCursor(0,48);  
      display64.print(oled_lines [6]);
      display64.setCursor(0,56);  
      display64.print(oled_lines [7]);
      display64.display();
     
    }
  }
}

/*
 * ======================================================================================================================
 * OLED_write() 
 * ======================================================================================================================
 */
void OLED_write(const char *str) {
  int c, len, bottom_line = 3;
  
  if (DisplayEnabled) {
    // move lines up
    for (c=0; c<=21; c++) {
      oled_lines [0][c] = oled_lines [1][c];
      oled_lines [1][c] = oled_lines [2][c];
      oled_lines [2][c] = oled_lines [3][c];
      if (OLED64) {
        oled_lines [3][c] = oled_lines [4][c];
        oled_lines [4][c] = oled_lines [5][c];
        oled_lines [5][c] = oled_lines [6][c];  
        oled_lines [6][c] = oled_lines [7][c];  
        bottom_line = 7;          
      }
    }

    // check length on new output line string
    len = strlen (str);
    if (len>21) {
      len = 21;
    }
    for (c=0; c<=len; c++) {
      oled_lines [bottom_line][c] = *(str+c);
    }

    // Adding Padding
    for (;c<=21; c++) {
      oled_lines [bottom_line][c] = ' ';
    }
    oled_lines [bottom_line][22] = (char) NULL;
    
    OLED_update();
  }
}

/*
 * ======================================================================================================================
 * OLED_write()  - Handel flash string
 * ======================================================================================================================
 */
void OLED_write(const __FlashStringHelper *str) {
  int c, len, bottom_line = 3;
  
  if (DisplayEnabled) {
    // move lines up
    for (c=0; c<=21; c++) {
      oled_lines [0][c] = oled_lines [1][c];
      oled_lines [1][c] = oled_lines [2][c];
      oled_lines [2][c] = oled_lines [3][c];
      if (OLED64) {
        oled_lines [3][c] = oled_lines [4][c];
        oled_lines [4][c] = oled_lines [5][c];
        oled_lines [5][c] = oled_lines [6][c];  
        oled_lines [6][c] = oled_lines [7][c];  
        bottom_line = 7;          
      }
    }

    memset(msgbuf, 0, sizeof(msgbuf));

    // strncpy_P to safely copy the flash string (str) into a RAM buffer (msgbuf)
    strncpy_P(msgbuf, (const char*)str, sizeof(msgbuf) - 1);
    
    // check length on new output line string
    len = strlen (msgbuf);
    if (len>21) {
      len = 21;
    }
    for (c=0; c<=len; c++) {
      oled_lines [bottom_line][c] = *(msgbuf+c);
    }

    // Adding Padding
    for (;c<=21; c++) {
      oled_lines [bottom_line][c] = ' ';
    }
    oled_lines [bottom_line][22] = (char) NULL;
    
    OLED_update();
  }
}

/*
 * ======================================================================================================================
 * OLED_write_noscroll() -- keep lines 1-3 and output on line 4
 * ======================================================================================================================
 */
void OLED_write_noscroll(const char *str) {
  int c, len, bottom_line = 3;

  if (OLED64) {
    bottom_line = 7;
  }

  if (DisplayEnabled) {
    len = strlen (str);
    if (len>21) {
      len = 21;
    }
    
    for (c=0; c<=len; c++) {
      oled_lines [bottom_line][c] = *(str+c);
    }

    // Adding Padding
    for (;c<=21; c++) {
      oled_lines [bottom_line][c] = ' ';
    }
    oled_lines [bottom_line][22] = (char) NULL;
    
    OLED_update();
  }
}

/*
 * ======================================================================================================================
 * Serial_flush() 
 * ======================================================================================================================
 */
void Serial_flush() {
  if (SerialConsoleEnabled) {
    Serial.flush();
  }
}

/*
 * ======================================================================================================================
 * Serial_write() 
 * ======================================================================================================================
 */
void Serial_write(char c) {
  if (SerialConsoleEnabled) {
    Serial.print(c);
  }
}

/*
 * ======================================================================================================================
 * Serial_write() 
 * ======================================================================================================================
 */
void Serial_write(const char *str, int base) {
  if (SerialConsoleEnabled) {
    if (base == HEX) {
      while (*str) {
        byte b = *str++;
        if (b < 16) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
      }
    }
    else {
      Serial.print(str); // default to plain string print
    }
  }
}

/*
 * ======================================================================================================================
 * Serial_write() 
 * ======================================================================================================================
 */
void Serial_write(const char *str) {
  if (SerialConsoleEnabled) {
    Serial.print(str);
  }
}

/*
 * ======================================================================================================================
 * Serial_writeln() 
 * ======================================================================================================================
 */
void Serial_writeln(const char *str) {
  if (SerialConsoleEnabled) {
    Serial.println(str);
    Serial.flush();
  }
}

/*
 * ======================================================================================================================
 * Serial_writeln() - Handel flash string
 * ======================================================================================================================
 */
void Serial_writeln(const __FlashStringHelper *str) {
  if (SerialConsoleEnabled) {
    Serial.println(str);
    Serial.flush();
  }
}

/*
 * ======================================================================================================================
 * Output() - Count, delay between, delay at end
 * ======================================================================================================================
 */
void Output(const char *str) {
  OLED_write(str);
  Serial_writeln(str);
}

/*
 * ======================================================================================================================
 * Output() - Count, delay between, delay at end
 * ======================================================================================================================
 */
void Output(const __FlashStringHelper *str) {
  OLED_write(str);
  Serial_writeln(str);
}

/*
 * ======================================================================================================================
 * OLED_initialize() -- Initialize oled if enabled
 * ======================================================================================================================
 */
void OLED_initialize() {
  if (DisplayEnabled) {
    if (I2C_Device_Exist (OLED32_I2C_ADDRESS)) {
      oled_type = OLED32_I2C_ADDRESS;
      display32.begin(SSD1306_SWITCHCAPVCC, OLED32_I2C_ADDRESS);
      display32.clearDisplay();
      display32.setTextSize(1); // Draw 2X-scale text
      display32.setTextColor(WHITE);
      display32.setCursor(0, 0);
      for (int r=0; r<4; r++) {
        oled_lines[r][0]=0;
      }
      OLED_write("OLED32:OK");
    }
    else if (I2C_Device_Exist (OLED64_I2C_ADDRESS)) {
      oled_type = OLED64_I2C_ADDRESS;
      display64.begin(SSD1306_SWITCHCAPVCC, OLED64_I2C_ADDRESS);
      display64.clearDisplay();
      display64.setTextSize(1); // Draw 2X-scale text
      display64.setTextColor(WHITE);
      display64.setCursor(0, 0);
      for (int r=0; r<8; r++) {
        oled_lines[r][0]=0;
      }
      OLED_write("OLED64:OK");
    }
    else {
      DisplayEnabled = false;
      SystemStatusBits |= SSB_OLED; // Turn on Bit
    }
  }
}

/*
 * ======================================================================================================================
 * Serial_Initialize() -
 * ======================================================================================================================
 */
void Serial_Initialize() {
  // serial console enable pin
  pinMode(SCE_PIN, INPUT_PULLUP);   // Internal pullup resistor biases the pin to supply voltage.
                                    // If jumper set to ground, we enable serial console (low = enable)
  if (digitalRead(SCE_PIN) == LOW) {
    SerialConsoleEnabled = true;
  }

  // There are libraries that print to Serial Console so we need to initialize no mater what the jumper is set to.
  Serial.begin(9600);
  delay(1000); // prevents usb driver crash on startup, do not omit this

  if (SerialConsoleEnabled) {
    // Wait for serial port to be available
    if (!Serial) {
      OLED_write("Wait4 Serial Console");
    }
    int countdown=60; // Wait N seconds for serial connection, then move on.
    while (!Serial && countdown) {
      HeartBeat(); // Provides 250ms delay
      Blink(1, 750);
      countdown--;
    }

    Serial_writeln(""); // Send carriage return and linefeed 

    
    if (DisplayEnabled) {
      Serial_writeln (F("OLED:Enabled"));
    }
    else {
      Serial_writeln (F("OLED:Disabled"));
    }
    Output (F("SC:Enabled"));

    // Arduino_DebugUtils - Used by Network Libraries for Connection Debugging
    // `DBG_NONE` - no debug output is shown
    // `DBG_ERROR` - critical errors
    // `DBG_WARNING` - non-critical errors
    // `DBG_INFO` - information
    // `DBG_DEBUG` - more information
    // `DBG_VERBOSE` - most information
    setDebugMessageLevel(DBG_VERBOSE);
    Serial_writeln("NW:DEBUG ENABLED");
  }
  else {
    setDebugMessageLevel(DBG_NONE);
  }
}

/*
 * ======================================================================================================================
 * Output_Initialize() -
 * ======================================================================================================================
 */
void Output_Initialize() {
  OLED_initialize();
  Output(F("SER:Init"));
  Serial_Initialize();
  Output(F("SER:OK"));
}
