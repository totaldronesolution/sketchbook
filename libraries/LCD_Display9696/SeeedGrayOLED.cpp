/*
  SeeedGrayOLED.cpp - SSD1327 Gray OLED Driver Library
  2011 Copyright (c) Seeed Technology Inc.  All right reserved.
 
  Author: Visweswara R
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"

//#include "WProgram.h"
#include "Wire.h"

#include "SeeedGrayOLED.h"

#include <avr/pgmspace.h>

// 8x8 Font ASCII 32 - 127 Implemented
// Users can modify this to support more characters(glyphs)
// BasicFont is placed in code memory.

// This font can be freely used without any restriction(It is placed in public domain)
const unsigned char BasicFont[][8] PROGMEM=
{
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x07,0x00,0x07,0x00,0x00,0x00},
  {0x00,0x14,0x7F,0x14,0x7F,0x14,0x00,0x00},
  {0x00,0x24,0x2A,0x7F,0x2A,0x12,0x00,0x00},
  {0x00,0x23,0x13,0x08,0x64,0x62,0x00,0x00},
  {0x00,0x36,0x49,0x55,0x22,0x50,0x00,0x00},
  {0x00,0x00,0x05,0x03,0x00,0x00,0x00,0x00},
  {0x00,0x1C,0x22,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x22,0x1C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x2A,0x1C,0x2A,0x08,0x00,0x00},
  {0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x00},
  {0x00,0xA0,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
  {0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x10,0x08,0x04,0x02,0x00,0x00},
  {0x00,0x3E,0x51,0x49,0x45,0x3E,0x00,0x00},
  {0x00,0x00,0x42,0x7F,0x40,0x00,0x00,0x00},
  {0x00,0x62,0x51,0x49,0x49,0x46,0x00,0x00},
  {0x00,0x22,0x41,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x18,0x14,0x12,0x7F,0x10,0x00,0x00},
  {0x00,0x27,0x45,0x45,0x45,0x39,0x00,0x00},
  {0x00,0x3C,0x4A,0x49,0x49,0x30,0x00,0x00},
  {0x00,0x01,0x71,0x09,0x05,0x03,0x00,0x00},
  {0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x06,0x49,0x49,0x29,0x1E,0x00,0x00},
  {0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00},
  {0x00,0x00,0xAC,0x6C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x00},
  {0x00,0x14,0x14,0x14,0x14,0x14,0x00,0x00},
  {0x00,0x41,0x22,0x14,0x08,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x51,0x09,0x06,0x00,0x00},
  {0x00,0x32,0x49,0x79,0x41,0x3E,0x00,0x00},
  {0x00,0x7E,0x09,0x09,0x09,0x7E,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x22,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x22,0x1C,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x41,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x01,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x51,0x72,0x00,0x00},
  {0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00},
  {0x00,0x41,0x7F,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x40,0x41,0x3F,0x01,0x00,0x00},
  {0x00,0x7F,0x08,0x14,0x22,0x41,0x00,0x00},
  {0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00},
  {0x00,0x7F,0x02,0x0C,0x02,0x7F,0x00,0x00},
  {0x00,0x7F,0x04,0x08,0x10,0x7F,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x06,0x00,0x00},
  {0x00,0x3E,0x41,0x51,0x21,0x5E,0x00,0x00},
  {0x00,0x7F,0x09,0x19,0x29,0x46,0x00,0x00},
  {0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x00},
  {0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x00},
  {0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x00},
  {0x00,0x1F,0x20,0x40,0x20,0x1F,0x00,0x00},
  {0x00,0x3F,0x40,0x38,0x40,0x3F,0x00,0x00},
  {0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x00},
  {0x00,0x03,0x04,0x78,0x04,0x03,0x00,0x00},
  {0x00,0x61,0x51,0x49,0x45,0x43,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00},
  {0x00,0x41,0x41,0x7F,0x00,0x00,0x00,0x00},
  {0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x00},
  {0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00},
  {0x00,0x01,0x02,0x04,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x00},
  {0x00,0x7F,0x48,0x44,0x44,0x38,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x28,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x48,0x7F,0x00,0x00},
  {0x00,0x38,0x54,0x54,0x54,0x18,0x00,0x00},
  {0x00,0x08,0x7E,0x09,0x02,0x00,0x00,0x00},
  {0x00,0x18,0xA4,0xA4,0xA4,0x7C,0x00,0x00},
  {0x00,0x7F,0x08,0x04,0x04,0x78,0x00,0x00},
  {0x00,0x00,0x7D,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x80,0x84,0x7D,0x00,0x00,0x00,0x00},
  {0x00,0x7F,0x10,0x28,0x44,0x00,0x00,0x00},
  {0x00,0x41,0x7F,0x40,0x00,0x00,0x00,0x00},
  {0x00,0x7C,0x04,0x18,0x04,0x78,0x00,0x00},
  {0x00,0x7C,0x08,0x04,0x7C,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x38,0x00,0x00,0x00},
  {0x00,0xFC,0x24,0x24,0x18,0x00,0x00,0x00},
  {0x00,0x18,0x24,0x24,0xFC,0x00,0x00,0x00},
  {0x00,0x00,0x7C,0x08,0x04,0x00,0x00,0x00},
  {0x00,0x48,0x54,0x54,0x24,0x00,0x00,0x00},
  {0x00,0x04,0x7F,0x44,0x00,0x00,0x00,0x00},
  {0x00,0x3C,0x40,0x40,0x7C,0x00,0x00,0x00},
  {0x00,0x1C,0x20,0x40,0x20,0x1C,0x00,0x00},
  {0x00,0x3C,0x40,0x30,0x40,0x3C,0x00,0x00},
  {0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00},
  {0x00,0x1C,0xA0,0xA0,0x7C,0x00,0x00,0x00},
  {0x00,0x44,0x64,0x54,0x4C,0x44,0x00,0x00},
  {0x00,0x08,0x36,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x36,0x08,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x01,0x02,0x01,0x00,0x00},
  {0x00,0x02,0x05,0x05,0x02,0x00,0x00,0x00} 
};

void SeeedGrayOLED::init(void)
{
    sendCommand(0xFD); // Unlock OLED driver IC MCU interface from entering command. i.e: Accept commands
    sendCommand(0x12);
    sendCommand(0xAE); // Set display off
    sendCommand(0xA8); // set multiplex ratio
    sendCommand(0x5F); // 96
    sendCommand(0xA1); // set display start line
    sendCommand(0x00);
    sendCommand(0xA2); // set display offset
    sendCommand(0x60);
    sendCommand(0xA0); // set remap
    sendCommand(0x46);
    sendCommand(0xAB); // set vdd internal
    sendCommand(0x01); //
    sendCommand(0x81); // set contrasr
    sendCommand(0x53); // 100 nit
    sendCommand(0xB1); // Set Phase Length
    sendCommand(0X51); //
    sendCommand(0xB3); // Set Display Clock Divide Ratio/Oscillator Frequency
    sendCommand(0x01);
    sendCommand(0xB9); //
    sendCommand(0xBC); // set pre_charge voltage/VCOMH
    sendCommand(0x08); // (0x08);
    sendCommand(0xBE); // set VCOMH
    sendCommand(0X07); // (0x07);
    sendCommand(0xB6); // Set second pre-charge period
    sendCommand(0x01); //
    sendCommand(0xD5); // enable second precharge and enternal vsl
    sendCommand(0X62); // (0x62);
    sendCommand(0xA4); // Set Normal Display Mode
    sendCommand(0x2E); // Deactivate Scroll
    sendCommand(0xAF); // Switch on display
    delay(100);

    // Row Address
    sendCommand(0x75); 	  // Set Row Address 
    sendCommand(0x00); 	  // Start 0
    sendCommand(0x5f); 	  // End 95 


    // Column Address
    sendCommand(0x15); 	  // Set Column Address 
    sendCommand(0x08); 	  // Start from 8th Column of driver IC. This is 0th Column for OLED 
    sendCommand(0x37); 	  // End at  (8 + 47)th column. Each Column has 2 pixels(segments)

    // Init gray level for text. Default:Brightest White
    grayH= 0xF0;
    grayL= 0x0F;
}

void SeeedGrayOLED::sendCommand(unsigned char command)
{
    Wire.beginTransmission(SeeedGrayOLED_Address); // begin I2C communication
    Wire.write(SeeedGrayOLED_Command_Mode);	   // Set OLED Command mode
    Wire.write(command);
    Wire.endTransmission();    		           // End I2C communication
}

void SeeedGrayOLED::setContrastLevel(unsigned char ContrastLevel)
{
    sendCommand(SeeedGrayOLED_Set_ContrastLevel_Cmd);
    sendCommand(ContrastLevel);
}

void SeeedGrayOLED::setHorizontalMode()
{
    sendCommand(0xA0); // remap to
    sendCommand(0x42); // horizontal mode

    // Row Address
    sendCommand(0x75); 	  // Set Row Address 
    sendCommand(0x00); 	  // Start 0
    sendCommand(0x5f); 	  // End 95 

    // Column Address
    sendCommand(0x15); 	  // Set Column Address 
    sendCommand(0x08); 	  // Start from 8th Column of driver IC. This is 0th Column for OLED 
    sendCommand(0x37); 	  // End at  (8 + 47)th column. Each Column has 2 pixels(or segments)
}

void SeeedGrayOLED::setVerticalMode()
{
    sendCommand(0xA0); // remap to
    sendCommand(0x46); // Vertical mode
}

void SeeedGrayOLED::setTextXY(unsigned char Row, unsigned char Column)
{
    //Column Address
    sendCommand(0x15); 	           /* Set Column Address */
    sendCommand(0x08+(Column*4));  /* Start Column: Start from 8 */
    sendCommand(0x37); 	           /* End Column */
    // Row Address
    sendCommand(0x75); 	           /* Set Row Address */
    sendCommand(0x00+(Row*8));     /* Start Row*/
    sendCommand(0x07+(Row*8));     /* End Row*/
}

void SeeedGrayOLED::clearDisplay()
{
    unsigned char i,j;
    for(j=0;j<48;j++)
    {
        for(i=0;i<96;i++)  //clear all columns
        {
            sendData(0x00);
        }
    }

}

void SeeedGrayOLED::sendData(unsigned char Data)
{
    Wire.beginTransmission(SeeedGrayOLED_Address); // begin I2C transmission
    Wire.write(SeeedGrayOLED_Data_Mode);            // data mode
    Wire.write(Data);
    Wire.endTransmission();                    // stop I2C transmission
}

void SeeedGrayOLED::setGrayLevel(unsigned char grayLevel)
{
    grayH = (grayLevel << 4) & 0xF0;
    grayL =  grayLevel & 0x0F;
}

void SeeedGrayOLED::putChar(unsigned char C)
{
    if(C < 32 || C > 127) //Ignore non-printable ASCII characters. This can be modified for multilingual font.
    {
        C=' '; //Space
    }	

    
    for(char i=0;i<8;i=i+2)
    {
        for(char j=0;j<8;j++)
        {
            // Character is constructed two pixel at a time using vertical mode from the default 8x8 font
            char c=0x00;
            char bit1=(pgm_read_byte(&BasicFont[C-32][i]) >> j)  & 0x01;  
            char bit2=(pgm_read_byte(&BasicFont[C-32][i+1]) >> j) & 0x01;
           // Each bit is changed to a nibble
            c|=(bit1)?grayH:0x00;
            c|=(bit2)?grayL:0x00;
            sendData(c);
        }
    }
}

void SeeedGrayOLED::putString(const char *String)
{
    unsigned char i=0;
    while(String[i])
    {
        putChar(String[i]);     
        i++;
    }
}

unsigned char SeeedGrayOLED::putNumber(long long_num)
{
    unsigned char char_buffer[10]="";
    unsigned char i = 0;
    unsigned char f = 0;

    if (long_num < 0)
    {
        f=1;
        putChar('-');
        long_num = -long_num;
    }
    else if (long_num == 0)
    {
        f=1;
        putChar('0');
        return f;
    }

    while (long_num > 0)
    {
        char_buffer[i++] = long_num % 10;
        long_num /= 10;
    }

    f=f+i;
    for(; i > 0; i--)
    {
        putChar('0'+ char_buffer[i - 1]);
    }
    return f;

}



void SeeedGrayOLED::drawBitmap(unsigned char *bitmaparray,int bytes)
{
    char localAddressMode = addressingMode;
    if(addressingMode != HORIZONTAL_MODE)
    {
        //Bitmap is drawn in horizontal mode
        setHorizontalMode();
    }

    for(int i=0;i<bytes;i++)
    {

    for(int j=0;j<8;j=j+2) 
    {
        char c=0x00;
        char bit1=pgm_read_byte(&bitmaparray[i]) << j  & 0x80;  
        char bit2=pgm_read_byte(&bitmaparray[i]) << (j+1) & 0x80;

        // Each bit is changed to a nibble
        c|=(bit1)?grayH:0x00;
        // Each bit is changed to a nibble
        c|=(bit2)?grayL:0x00;
        sendData(c);
     }
    }
    if(localAddressMode == VERTICAL_MODE)
    {
        //If Vertical Mode was used earlier, restore it.
        setVerticalMode();
    }

}

void SeeedGrayOLED::setHorizontalScrollProperties(bool direction,unsigned char startRow, unsigned char endRow,unsigned char startColumn, unsigned char endColumn, unsigned char scrollSpeed)
{
    /*
Use the following defines for 'direction' :

 Scroll_Left			
 Scroll_Right			

Use the following defines for 'scrollSpeed' :

 Scroll_2Frames		
 Scroll_3Frames
 Scroll_4Frames
 Scroll_5Frames	
 Scroll_25Frames
 Scroll_64Frames
 Scroll_128Frames
 Scroll_256Frames

*/

    if(Scroll_Right == direction)
    {
        //Scroll Right
        sendCommand(0x27);
    }
    else
    {
        //Scroll Left  
        sendCommand(0x26);
    }
    sendCommand(0x00);       //Dummmy byte
    sendCommand(startRow);
    sendCommand(scrollSpeed);
    sendCommand(endRow);
    sendCommand(startColumn+8);
    sendCommand(endColumn+8);
    sendCommand(0x00);      //Dummmy byte

}

void SeeedGrayOLED::activateScroll()
{
    sendCommand(SeeedGrayOLED_Activate_Scroll_Cmd);
}

void SeeedGrayOLED::deactivateScroll()
{
    sendCommand(SeeedGrayOLED_Dectivate_Scroll_Cmd);
}

void SeeedGrayOLED::setNormalDisplay()
{
    sendCommand(SeeedGrayOLED_Normal_Display_Cmd);
}

void SeeedGrayOLED::setInverseDisplay()
{
    sendCommand(SeeedGrayOLED_Inverse_Display_Cmd);
}

SeeedGrayOLED SeeedGrayOled;  // Preinstantiate Objects

