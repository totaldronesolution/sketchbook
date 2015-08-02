#include <OWSlave.h>
//#include <OneWireSlave.h>
//#include <PinChangeInterrupt.h>
/********************************************************************************
    
    Program. . . . OWGeneric
    Author . . . . Ursin Solèr (according to design by Ian Evans)
    Written. . . . 1 Aug 2015.
    Description. . Act as One Wire Slave (emulates RTC DS2415)
                   Read all the analog inputs (A0-A5) and return values on demand
                       Returns Value (5 Bytes) depending on value written last (5 Bytes)
                       1 Device Control Byte and 4 Data Bytes

            AT.... Pin Layout (Arduino UNO)
                  ---_---
                  .     .
 VCC    B_5V   5V |?   ?| PD13       LED
 Ground BGND  GND |?   ?| PD12
                  .     .
 Sens 1 B_S01  A0 |?   ?| PD07
 Sens 2 B_S02  A1 |?   ?| PD06 
                  .     .
 Sens 4 B_S04  A3 |?   ?| PD02       One Wire
 Sens 5 B_S05  A4 |?   ?| PD01      
 Sens 6 B_S06  A5 |?   ?| PD00      
                  -------

Most simple read/write device with 4 byte (32 bit) memory: RTC DS2415
http://pdfserv.maximintegrated.com/en/ds/DS2415.pdf
http://owfs.sourceforge.net/DS2415.3.html

********************************************************************************/
//    One Wire Slave Data
//                          {Fami, <---, ----, ----, ID--, ----, --->,  CRC} 
unsigned char rom[8]      = {0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
//         char rom[8]      = {0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
//                          {MEM0, MEM1, MEM2, MEM3}
  char rtccountr_in[4]    = {0x00, 0x00, 0x00, 0x00};
//  char rtccountr_out[4]   = {0x01, 0x00, 0x00, 0x04};
  char rtccountr_out[4]   = {0x00, 0x00, 0x00, 0x00};
//char ctrlbyte = B10000001;    // U4 U3 U2 U1 OSC OSC 0 0
char ctrlbyte = B00000000;    // U4 U3 U2 U1 OSC OSC 0 0
// If the oscillator is intentionally stopped the 
// real time clock counter behaves as a 4-byte nonvolatile
// memory. -> OSC = 0 ALWAYS / U4:U1 memory


//    Pin Layouts
#define LEDPin    13
#define OWPin      2
#define OWPinInt   0

// LED Flash Variables
volatile long    flash       = 0;         // Flash achnowledgement counter
long             flashPause  = 100;       // LED between flash delay
#define          flashLength 50           // Flash length
long             flashStart  = 0;
long             flashStop   = 0;

// OW Interrupt Variables
volatile long prevInt    = 0;      // Previous Interrupt micros
volatile boolean owReset = false;

OWSlave ds(OWPin); 
//OneWireSlave ds(OWPin); 

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
    // Initialise the Pn usage
    pinMode(LEDPin, OUTPUT); 
    pinMode(OWPin, INPUT); 

    digitalWrite(LEDPin, LOW);    

//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);
    
    // Initialise the One Wire Slave Library
    ds.setRom(rom);

    flash +=2;
}

/********************************************************************************
    Repeatedly check for action to execute
********************************************************************************/
void loop() {
    
    if (flash > 0) FlashLED();    // Control the flashing of the LED

    if (owReset) owHandler();    // Handle the OW reset that was received
}

//************************************************************
// Process One Wire Handling
//************************************************************
void owHandler(void) {

    owReset=false;
//    detachPcInterrupt(OWPin);
    detachInterrupt(OWPinInt);
    
    if (ds.presence()) {
        if (ds.recvAndProcessCmd()) {
//    if (ds.waitForRequest(false)) {
            uint8_t cmd = ds.recv();
            emuDS2415(cmd);
        }
    }
//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    pinMode(OWPin, INPUT);   // recover the output mode in case of error in ds.send (should work without; error test)
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);
}

/********************************************************************************
    Emulate DS2415
    http://pdfserv.maximintegrated.com/en/ds/DS2415.pdf

  Transaction Sequence
  The protocol for accessing the DS2415 via the 1-Wire port is as follows:
  * Initialization
  * ROM Function Command
  * Clock Function Command
  (no crc... ;)
********************************************************************************/
void emuDS2415(uint8_t cmd){
    if (cmd == 0x66) {          // READ CLOCK 
        // send device control byte
        delayMicroseconds(100);
        ds.send(ctrlbyte);
        // send 4 bytes of the RTC counter "4-byte nonvolatile memory"
        //ds.sendData(rtccountr_out, 4);
        for( int i = 0; i < 4; i++) {
            delayMicroseconds(75);
            ds.send(rtccountr_out[i]);
        }
        flash++;
    } else if (cmd == 0x99) {   // WRITE CLOCK
        // receive device control byte
        ctrlbyte = ds.recv();
        // receive 4 bytes of the RTC counter "4-byte nonvolatile memory"
        for( int i = 0; i < 4; i++) {
            rtccountr_in[i] = ds.recv();
//            rtccountr_out[i] = ds.recv();
        }
        flash++;
        // process device control byte
        //ctrlbyte = (ctrlbyte>>4)<<4;  // set most-right 4 bits to 0
        ctrlbyte = ctrlbyte & 0xF0;  // set most-right 4 bits to 0
        // process cmd given in rtccountr_in memory
        process();
    }
}

/********************************************************************************
    Process sensor readings (rtccountr_in and prepare rtccountr_out)
********************************************************************************/
void process(void){
    // process LSB only ... 256 are enough commands for now
    // (rtccountr_in[3] == 00)           // nop/pass
    if (rtccountr_in[3] == 01) {         // A0
        unsigned int val = analogRead(A0);
        // http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c
//        bytes[0] = (val >> 24) & 0xFF;
//        bytes[1] = (val >> 16) & 0xFF;
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 02) {  // A1
        unsigned int val = analogRead(A1);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 03) {  // A2
        unsigned int val = analogRead(A2);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 04) {  // A3
        unsigned int val = analogRead(A3);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 05) {  // A4
        unsigned int val = analogRead(A4);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 06) {  // A5
        unsigned int val = analogRead(A5);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    }
}

//************************************************************
// Onw WireInterrupt handling 
//************************************************************
void onewireInterrupt(void) {
    volatile long lastMicros = micros() - prevInt;
    prevInt = micros();
    if (lastMicros >= 410 && lastMicros <= 550) { //  OneWire Reset Detected
        owReset=true;
    }
}


//************************************************************
// Flash the LED
//************************************************************
void FlashLED(void) {
    if (flashStart == 0 && flashStop == 0) {
        flashStart = millis();
    }
    else if (flashStart > 0 && (millis()-flashStart > flashPause)) {
        digitalWrite(LEDPin, HIGH);
        flashStart = 0;
        flashStop = millis() ;
    }
    else if (flashStop > 0 && (millis() - flashStop > flashLength)) {
        digitalWrite(LEDPin, LOW);
        flashStop = 0;
        flash--;
    }
}

