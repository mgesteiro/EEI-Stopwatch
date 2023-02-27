/**
 * E.E.I. Stopwatch
 * A stopwatch for UVigo's E.E.I. Robotic Competition - https://eei.robots.webs.uvigo.es/
 *
 * @file      EEI-Stopwatch.ino
 * @author    mgesteiro
 * @date      20231702
 * @version   1.0.0
 * @copyright mgesteiro, OpenSource with LICENSE GPLv3
 * @note      Required libraries: TM1637TinyDisplay, Pololu's VL53L0X
 */

#define PROGRAM "E.E.I. Stopwatch"
#define VERSION "1.0.0"

// Configuration
#define NUMLAPS  4     // Number of Laps to store (maximum 10)
#define TRIGGERD 200   // Trigger Distance in mm: less than this is considered a "detection"
#define DCWINDOW 1900  // Detect Clear Window in ms: no more detections triggered during this time
#define SLWINDOW 2000  // Show Lap Window in ms: how long we show previous lap time while counting

/* Advanced: timing resolution
 * This is the 'time budget' value for the ToF laser sensor: time allowed to measure a distance.
 * The minimum value depends on the hardware revision (18700 in my case). Check your vendor's datasheet.
 * The bigger the number, the more accurate it gets measuring the distance and the less prone to
 * interference, but also the less responsive it becomes, as it requires more time for each measure.
 * The default factory value is ~33000 (i.e. ~33 ms). A value of 20000 means that the fastest it can
 * take a measure is 20 ms, thus becoming by de facto the maximum stopwatch timing resolution.
 * Use a value of 0 to let the device use its factory default.
 */
#define RESOLUTION 50000  // in µs

// Includes
#include <Arduino.h>
#include "Stopwatch.h"
#include "AKeypad-lib.h"
#include <TM1637TinyDisplay.h>
#include <VL53L0X.h>

// Stopwatch
Stopwatch sw(NUMLAPS);
uint8_t whatLap = 0;
uint32_t finishShowLapTime = 0;

// Keypad
#define AKP_PIN A3
#define K_YELLOW 1
#define K_RED 2
const int16_t AKP_VALUES[] = {550, 390};  // 22Kohm resistors
AKeypad keypad(AKP_PIN, 2, AKP_VALUES);


// TM1637TinyDisplay - 4 Digit Display
#define CLK 2
#define DIO 3
TM1637TinyDisplay display(CLK, DIO);

// Initial animation
// @see https://jasonacox.github.io/TM1637TinyDisplay/examples/7-segment-animator.html
const uint8_t INTRO_A[][4] PROGMEM = {
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x10, 0x00, 0x00, 0x00 },
	{ 0x20, 0x00, 0x00, 0x00 },
	{ 0x01, 0x00, 0x00, 0x00 },
	{ 0x00, 0x01, 0x00, 0x00 },
	{ 0x00, 0x00, 0x01, 0x00 },
	{ 0x00, 0x00, 0x00, 0x01 },
	{ 0x00, 0x00, 0x00, 0x02 },
	{ 0x00, 0x00, 0x00, 0x04 },
	{ 0x00, 0x00, 0x00, 0x08 },
	{ 0x00, 0x00, 0x08, 0x00 },
	{ 0x00, 0x08, 0x00, 0x00 },
	{ 0x00, 0x10, 0x00, 0x00 },
	{ 0x00, 0x20, 0x00, 0x00 },
	{ 0x00, 0x01, 0x00, 0x00 },
	{ 0x00, 0x00, 0x01, 0x00 },
	{ 0x00, 0x00, 0x02, 0x00 },
	{ 0x00, 0x00, 0x04, 0x00 },
	{ 0x00, 0x00, 0x08, 0x00 },
	{ 0x00, 0x08, 0x00, 0x00 },
	{ 0x08, 0x00, 0x00, 0x00 },
	{ 0x10, 0x00, 0x00, 0x00 },
	{ 0x20, 0x00, 0x00, 0x00 },
	{ 0x01, 0x00, 0x00, 0x00 },
	{ 0x00, 0x01, 0x00, 0x00 },
	{ 0x00, 0x00, 0x01, 0x00 },
	{ 0x00, 0x00, 0x00, 0x01 },
	{ 0x00, 0x00, 0x00, 0x02 },
	{ 0x00, 0x00, 0x00, 0x04 },
	{ 0x00, 0x00, 0x00, 0x08 },
	{ 0x00, 0x00, 0x08, 0x00 },
	{ 0x00, 0x08, 0x00, 0x00 },
	{ 0x08, 0x00, 0x00, 0x00 },
	{ 0x18, 0x00, 0x00, 0x00 },
	{ 0x38, 0x00, 0x00, 0x00 },
	{ 0x39, 0x00, 0x00, 0x00 },
	{ 0x39, 0x01, 0x00, 0x00 },
	{ 0x39, 0x01, 0x01, 0x00 },
	{ 0x39, 0x01, 0x01, 0x01 },
	{ 0x39, 0x01, 0x01, 0x03 },
	{ 0x39, 0x01, 0x01, 0x07 },
	{ 0x39, 0x01, 0x01, 0x0f },
	{ 0x39, 0x01, 0x09, 0x0f },
	{ 0x39, 0x09, 0x09, 0x0f },
	{ 0x06, 0x09, 0x09, 0x30 },
	{ 0x00, 0x39, 0x0f, 0x00 },
	{ 0x00, 0x06, 0x30, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 }
};
// Reset animation
const uint8_t RESET_A[][4] PROGMEM = {
	{ 0x00, 0x06, 0x30, 0x00 },
	{ 0x00, 0x46, 0x70, 0x00 },
	{ 0x40, 0x06, 0x30, 0x40 },
	{ 0x30, 0x06, 0x30, 0x06 },
	{ 0x30, 0x30, 0x06, 0x06 },
	{ 0x36, 0x00, 0x00, 0x36 },
	{ 0x30, 0x00, 0x00, 0x06 },
	{ 0x00, 0x00, 0x00, 0x00 }
};

// VL53L0X ToF sensor
VL53L0X tof;
uint32_t detectionClearTime = 0;

// working modes
uint8_t wmode = 0;
#define M_DISTANCE 1
#define M_STOPWATCH 2
#define M_RESULTS 3


/*********************************************************************************************
 *
 *   S E T U P   &   L O O P
 *
 */

void setup() {
	// Serial port banner
	Serial.begin(115200);
	Serial.print(PROGRAM);
	Serial.print(" [v");
	Serial.print(VERSION);
	Serial.println("]");
	Serial.println("Configuration: ");
	Serial.print("  Laps: ");
	Serial.println(NUMLAPS);
	Serial.print("  Trigger distance: ");
	Serial.println(TRIGGERD);
	Serial.print("  Detect clear window: ");
	Serial.println(DCWINDOW);
	Serial.print("  Show lap window: ");
	Serial.println(SLWINDOW);
	Serial.print("  Timing resolution: ");
	Serial.print(RESOLUTION);
	Serial.print(" [");

	// Keypad
	keypad.init();

	// TM1637 display
	display.begin();
	display.setBrightness(BRIGHT_7);

	// VL53L0X
	Wire.begin();
	tof.setTimeout(500);
	if (! tof.init())
	{
		Serial.println("Failed to detect and initialize ToF sensor!");
		display.showString("Tofe");
		while(1);  // HALT 
	}
	tof.setMeasurementTimingBudget(RESOLUTION);  // defined in the configuration section
	Serial.print(tof.getMeasurementTimingBudget()); Serial.println("]");  // show real configured value
	tof.startContinuous();  // start measuring distance

	// Animation sequence in PROGMEM flash memory
	display.showAnimation_P(INTRO_A, FRAMES(INTRO_A), TIME_MS(30));
	delay(300);

	// initial mode
	wmode = M_DISTANCE;
}  // setup()

void loop() {
	uint32_t currenTime = millis();

	// watch keypad and process actions
	uint8_t kp_code = keypad.handleKeypad(currenTime);
	if (kp_code) processKeyEvent(kp_code);

	// Operate depending on the mode:
	//
	// DISTANCE mode: show distance to help position the device
	if (wmode == M_DISTANCE)
	{
		showDistance(tof.readRangeContinuousMillimeters());
	} else
	//
	// STOPWATCH mode: show stopwatch
	if (wmode == M_STOPWATCH)
	{
		if (isDetected(currenTime))
		{
			if (! sw.running()) sw.start();  // start counting
			else
			{
				sw.lap();  // time the lap
				finishShowLapTime = currenTime + SLWINDOW;  // when to stop showing the new lap time
				if (! sw.running())
				{
					// storage space depleted, i.e. it was the last lap
					showTime(sw.getTime());  // update display
					delay(SLWINDOW);  // show it the same amount of time as any lap
					display.showString(" End");  // indicate it has finished
					wmode = M_RESULTS;  // go to results mode
				}
			}
		}
		if (sw.running())
			if (currenTime < finishShowLapTime) showTime(sw.getPreviousLapTime());  // still showing last lap time
			else showTime(sw.getTime());  // live timing
	} /* else
	//
	// RESULTS mode: show results
	if (wmode == M_RESULTS)
	{
		nothing to do here because all actions are related to events driven by
		the keypad, that has already been handled at the beginning of this loop.
	}
	*/
}  // loop()

/*********************************************************************************************
 *
 *   S U P P O R T   F U N C T I O N S
 *
 */

/**
 * Displays the provided time in human readable format: Seconds:Tenths  // no leading "0"
 *
 * @param lapTime  Time to be shown, in milliseconds.
 */
void showTime(uint32_t lapTime)
{
	uint8_t segments[4];
	for (int8_t i = 3; i >= 0; i --)
	{
		lapTime /= 10; // we do this beforehand because only 4 digits needed <- discard last position
		segments[i] = display.encodeDigit(lapTime % 10);
	}
	if (lapTime == 0) segments[0] = 0;  // delete unnecessary leading 0
	segments[1] |= SEG_DP;  // active colon
	display.setSegments(segments);  // show data
}

/**
 * Displays the provided distance (in cm).
 *
 * @param distance Distance in milimiters to be converted and displayed
 */
void showDistance(int16_t distance)
{
	distance /= 10;  // convert to cms
	display.showString("d", 1, 1, B10000000);  // "d:"
	display.showNumber(distance, false, 2, 2);  // two last display digits
}

/**
 * Displays the indicated lap time, showing before briefly the lap number.
 *
 * @param lap Which lap to show.
 */
void showLapTime(uint8_t lap)
{
	display.clear();
	// show lap indicator first briefly
	display.showString("L", 1, 1); // "L"
	display.showNumber(lap+1, false, 1, 2);
	delay(350);
	// show finally the lap time
	showTime(sw.getLapTime(lap));
}

/**
 * High level function that contains the logic to process the keypad actions/events.
 *
 * @param kp_code packed byte returning from the handleKeyPad() function
 *                lo nibble -> key code,  hi nibble -> event [T_AKP_EVENTS]
 */
void processKeyEvent(uint8_t kp_code)
{
	uint8_t key, event;
	key = kp_code & B00001111; // lower nibble
	event = kp_code >> 4;  // upper nibble

	// SHORT key press
	if (event == AKP_EVT_PRESSED)
	{
		if (wmode == M_DISTANCE && key == K_YELLOW)
		{
			// change to STOPWATCH mode
			wmode = M_STOPWATCH;
			showTime(0);
			return;
		}
		if (wmode == M_STOPWATCH && key == K_YELLOW && sw.running())
		{
			// stop stopwatch and go to results
			sw.abort();
			wmode = M_RESULTS;
			display.showString(" End");  // indicate it has finished
			return;
		}
		if (wmode == M_RESULTS && key == K_YELLOW)
		{
			// show lap time
			showLapTime(whatLap);
			// select next lap
			whatLap ++;
			if (whatLap >= NUMLAPS) whatLap = 0;
		}
	} else
	// LONG key press
	if (event == AKP_EVT_LONGPRESSED)
	{
		if ((wmode == M_STOPWATCH || wmode == M_RESULTS) && key == K_RED)
		{
			// reset stopwatch
			sw.reset();
			whatLap = 0;
			display.showAnimation_P(RESET_A, FRAMES(RESET_A), TIME_MS(30));
			showTime(0);
			wmode = M_STOPWATCH; // assert STOPWATCH mode
		}
	}
} // processKeyEvent()

/**
 * Check the sensor with a security margin/window.
 *
 * @param currentTime current time in milliseconds 
 * @return true if an object is nearer than the threshold distance and the security
 *         margin window (in milliseconds) has been cleared. False otherwise.
 */
bool isDetected(uint32_t currentTime)
{
	if (currentTime < detectionClearTime) return false;  // if not clear, quit
	bool result = tof.readRangeContinuousMillimeters() < TRIGGERD;  // something close?
	// update detection clear window
	if (result) detectionClearTime = millis() + DCWINDOW;  // we call millis() because the ToF sensor call is very time-consuming
	return result;
}
