/**
 * Utility to check EEI-Stopwatch ToF sensor VL53L0X
 */

#include <TM1637TinyDisplay.h>
#include <VL53L0X.h>

#define CLK 2
#define DIO 3
TM1637TinyDisplay display(CLK, DIO);

VL53L0X tof;

void setup() {
  // 4-digit display
  display.begin();
  display.setBrightness(BRIGHT_7);
  // VL53L0X
  Wire.begin();
  tof.setTimeout(500);
  if (! tof.init())
  {
    display.showString("Tofe");
    while (1);
  }
  tof.startContinuous();
}

void loop() {
  int distance = tof.readRangeContinuousMillimeters();
  display.showNumber(distance);
  delay(100);
}
