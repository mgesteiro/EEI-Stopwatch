/**
 * Utility to check EEI-Stopwatch keypad
 * For 22kΩ resistors should give ~390 and ~550 values
 */

#include <TM1637TinyDisplay.h>

#define CLK 2
#define DIO 3
TM1637TinyDisplay display(CLK, DIO);

void setup() {
  pinMode(A3, INPUT_PULLUP);
  display.begin();
  display.setBrightness(BRIGHT_7);
}

void loop() {
  display.showNumber(analogRead(A3));
  delay(100);
}
