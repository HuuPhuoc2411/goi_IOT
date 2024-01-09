#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

const int numRows = 3;
const int numCols = 4;
int touchMatrix[numRows][numCols];

const String channelNames[12] = {
  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "10",
  "11"
};

void setup() {
  Serial.begin(9600);

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  
  // Initialize the touch matrix
  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      touchMatrix[row][col] = 0;
    }
  }
}

void loop() {
  // Get the currently touched pads
  currtouched = cap.touched(); // 000 011 001 110
  
  for (uint8_t i = 0; i < 12; i++) {
    // Update touch matrix based on touch events
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) { 
      int row = i / numCols;
      int col = i % numCols;
      touchMatrix[row][col] = 1;
    }
    else if (!(currtouched & _BV(i)) && (lasttouched & _BV(i))) {
      int row = i / numCols;
      int col = i % numCols;
      touchMatrix[row][col] = 0;
    }
  }

  bool anyChannelTouched = false; // Flag to check if at least one channel is touched
  
  // Print the touch matrix and check if any channel is touched
  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      Serial.print(touchMatrix[row][col]);
      Serial.print("\t");
      if (touchMatrix[row][col] == 1) {
        anyChannelTouched = true;
      }
    }
    Serial.println();
  }
  Serial.println();
  
  // Print the touched channels and their names
  if (anyChannelTouched)
  {
    Serial.print("Có người ngồi");
    for (uint8_t i = 0; i < 12; i++)
    {
      if (currtouched & _BV(i)) 
      {
        Serial.print(" ");
        Serial.print(channelNames[i]);
        Serial.print(" | ");
      }
    }
    Serial.println();
  }
  else {
    Serial.println("Không có người ngồi");
  }

  // Reset our state
  lasttouched = currtouched;

  delay(100);
}