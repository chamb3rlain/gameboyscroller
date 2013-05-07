/*******
 * Constructing a side-scrolling LED stock ticker controlled by a repurposed Gameboy
 * main file
 *
 *  Notes:
 * 1) 8 Mhz clock with 512 kbps serial speed is absolutely necessary for no-flicker.
 *    Hence, double-speed Gameboy Color is necessary!!
 * 2) 3 pins are used from Gameboy serial connection: GROUND, CLKOUT, and DATAOUT
 *
 *******/
#include <gb/gb.h>
#include <stdio.h>
#include <string.h>
#include "data50x7.h"

#define uint unsigned int

/* global variables*/
#define SCROLLCOUNT 4
char* scrollMessage[] = {"    Constructing a side-scrolling LED stock ticker controlled by a repurposed Gameboy    by Chamberlain Fong",
                         "    ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ",
                         "    Nuts and Volts magazine ",
                         "    Special thanks to Ryan Price, Dan Danknick, Jeff Frohwein, Wallace Ly, and Frank Dm"
                        };

unsigned char screenBuffer[100];

int cycleDelay;
unsigned char* SC_SFR = ((unsigned char*) 0xFF02);
unsigned char* SB_SFR = ((unsigned char*) 0xFF01);
int sink;
int state;


void delayer(unsigned int count)
{
  unsigned int a,i;
  
  for(i=0;i<count;i++) {
    a = i;
  }
}

void pauseDelay()
{
  delayer(10);
}

void sendByte(unsigned char value)
{
   int filtered;
  
  *SC_SFR = 0x3;
  *SB_SFR = value;
  *SC_SFR = 0x83;
  do {
    filtered = SC_SFR[0] & 0x80;
  } while( filtered == 0x80);
}



void dynamicDrive(unsigned char* messagePointer,int state, int sink)
{ 
  sendByte(messagePointer[0+state]);
  sendByte(messagePointer[5+state]);
  sendByte(messagePointer[10+state]);
  sendByte(messagePointer[15+state]);
  sendByte(messagePointer[20+state]);
  
  sendByte(messagePointer[25+state]);
  sendByte(messagePointer[30+state]);
  sendByte(messagePointer[35+state]);
  sendByte(messagePointer[40+state]);
  sendByte(messagePointer[45+state]);
  sendByte(sink);
}

/*****************************************************
 * display a still bitmap
 *****************************************************/
int dynamicDriveStill(unsigned char* messagePointer)
{
  int sink = 0x80;  
  int state = 0;
  int ret = 0;
  cycleDelay = 9;
  
  disable_interrupts();
  while(1) {
    delay(cycleDelay);
    
    sendByte(messagePointer[0+state]);
    sendByte(messagePointer[5+state]);
    sendByte(messagePointer[10+state]);
    sendByte(messagePointer[15+state]);
    sendByte(messagePointer[20+state]);
  
    sendByte(messagePointer[25+state]);
    sendByte(messagePointer[30+state]);
    sendByte(messagePointer[35+state]);
    sendByte(messagePointer[40+state]);
    sendByte(messagePointer[45+state]);
    sendByte(sink);
    sink = sink>>1;
    state++;
    if (sink==0x04) {
      sink = 0x80;
      state = 0x0;
    }
    if (state==6) {
      state = 0x0;
    }  
    
    ret = joypad();
    if (ret != 0x0) {
      break;
    }
  }
  enable_interrupts();
  sendByte(0x0);
  return ret;
}

/*****************************************************
 * display a still text message
 *****************************************************/
int dynamicDriveStill2(unsigned char* messagePointer)
{
  int sink = 0x80;  
  int state = 0;
  int ret = 0;
  cycleDelay = 9;
  
  disable_interrupts();
  while(1) {
    delay(cycleDelay);
    dynamicDrive(messagePointer,state,sink);   
    sink = sink>>1;
    state++;
    if (sink==0x04) {
      sink = 0x80;
      state = 0x0;
    }

    ret = joypad();
    if (ret != 0x0) {
      break;
    }
  }
  enable_interrupts();
  sendByte(0x0);
  return ret;
}

void fillScreenBuffer(char* message)
{
  int i;
  unsigned char* asciiIndex;
  int bufferIndex;
  
  bufferIndex = 0;
  for(i=0;i<9;i++) {
    asciiIndex = ascii + (message[i]<<1) + (message[i]<<1)+ message[i];
    /*printf("%c %x %x %x %x %x\n",message[i],(uint)asciiIndex[0],(uint)asciiIndex[1],(uint)asciiIndex[2],(uint)asciiIndex[3],(uint)asciiIndex[4]);
     */
    screenBuffer[bufferIndex++] = asciiIndex[0];
    asciiIndex++;
    screenBuffer[bufferIndex++] = asciiIndex[0];
    asciiIndex++;
    screenBuffer[bufferIndex++] = asciiIndex[0];
    asciiIndex++;
    screenBuffer[bufferIndex++] = asciiIndex[0];
    asciiIndex++;
    screenBuffer[bufferIndex++] = asciiIndex[0];
    asciiIndex++;
    screenBuffer[bufferIndex++] = 0xff;
  }
}

void shiftScreenBuffer()
{
  int i,j;
  int state;
  int sink;
  
  state = 0;
  sink = 0x80;
  
  for(i=0;i<55;i++) {
    screenBuffer[i] = screenBuffer[i+1];
  }
  state = 0;
  sink = 0x80;
  for(j=0;j<1;j++) {
    for(i=0;i<10;i++) {
      delay(cycleDelay);
      
      dynamicDrive(screenBuffer,state,sink);
      
      sink = sink>>1;
      state++;
      if (sink==0x04) {
        sink = 0x80;
        state = 0x0;
      }
      if (state==6) {
        state = 0x0;
      }  
    }
  }
}

void nextChar(char ch)
{
  unsigned char* asciiIndex;
  
  asciiIndex = ascii + (ch<<1) + (ch<<1)+ ch;
  screenBuffer[50] = asciiIndex[0];
  asciiIndex++;
  screenBuffer[51] = asciiIndex[0];
  asciiIndex++;
  screenBuffer[52] = asciiIndex[0];
  asciiIndex++;
  screenBuffer[53] = asciiIndex[0];
  asciiIndex++;
  screenBuffer[54] = asciiIndex[0];
  asciiIndex++;
  screenBuffer[55] = 0xff;
}



/*****************************************************
 * sidescroll a text message
 *****************************************************/
int sidescroll(char* message)
{
  int ret;
  int length = strlen(message);
  int count = 9;
  char* messagePtr = message+9;
  fillScreenBuffer(message);
  shiftScreenBuffer();
  shiftScreenBuffer();
  shiftScreenBuffer();
  shiftScreenBuffer();
  while(1) {
    nextChar(messagePtr[0]);
    count++;
    messagePtr++;
    if (count>length) {
      messagePtr = message;
      count = 0;
    }
    shiftScreenBuffer();
    shiftScreenBuffer();
    shiftScreenBuffer();
    shiftScreenBuffer();
    shiftScreenBuffer();
    shiftScreenBuffer();
    ret= joypad();
    if (ret!= 0x0)
      return ret;
  }
  return ret;
}

extern int setupColor();

void main(void)
{
  int scrollNumber = 0;
  int messageNumber = 5;
  int keypadValue;
  unsigned int cycleDelay = 9; 
  unsigned char* messagePointer = messageArray[messageNumber];
  
  setupColor();
  disable_interrupts();
  cpu_fast();
  enable_interrupts();
     
  printf("press ARROWS to change message\n");
  printf("press SELECT to side-scroll\n");
  keypadValue = dynamicDriveStill(messagePointer);
  //keypadValue = dynamicDriveStill2("Ready ...");
  
  while(1) {
 
    switch(keypadValue) {
      case(J_UP):
      messageNumber = (messageNumber+BITMESSAGECOUNT+1)%BITMESSAGECOUNT;
      messagePointer = messageArray[messageNumber];
      keypadValue = dynamicDriveStill(messagePointer);
      break;
      case(J_DOWN):
      messageNumber = (messageNumber+BITMESSAGECOUNT-1)%BITMESSAGECOUNT;
      messagePointer = messageArray[messageNumber];
      keypadValue = dynamicDriveStill(messagePointer);
      break;
      case(J_LEFT):
      scrollNumber = (scrollNumber+SCROLLCOUNT-1)%SCROLLCOUNT;
      keypadValue = sidescroll(scrollMessage[scrollNumber]);
      break;
      case(J_RIGHT):
      scrollNumber = (scrollNumber+SCROLLCOUNT+1)%SCROLLCOUNT;
      keypadValue = sidescroll(scrollMessage[scrollNumber]);
      break;
      case(J_A):
      fillScreenBuffer("NUTSVOLTS");
      keypadValue = dynamicDriveStill2(screenBuffer);
      break;
      case(J_B):
      fillScreenBuffer("  SERVO  ");
      keypadValue = dynamicDriveStill2(screenBuffer);
      break;
      case(J_SELECT):
      keypadValue= sidescroll(scrollMessage[2]);  
      break;
      case(J_START):
      keypadValue = sidescroll(scrollMessage[3]);
      break;
    }
    waitpadup();
  }
}
