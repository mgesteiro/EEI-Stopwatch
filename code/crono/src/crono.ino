#include <Arduino.h>
#include <TM1637.h>
#include <StopWatch.h>

// CONFIGURATION
// - PINS
#define CLK 2
#define DIO 3
#define IR_PIN 7
#define KP_PIN A3 // A4
// - APP
#define NUM_LAPS 2 // # laps stored
#define SHOW_LAP_TIME 2000 // show lap time duration - milliseconds
#define CLEAR_WINDOW 1500  // stopwatch controls protection window - milliseconds
// END CONFIG

TM1637 display(CLK, DIO);
StopWatch sw(NUM_LAPS);
bool clearToGo = true;
bool detectedObject = false;
bool showLap = false;
uint32_t currentTime, showLapTime;
uint8_t whatLap = 0;
// protection
uint32_t PW_windowTime = 0;


byte readKeypad()
{
	// constant
	// 10k resistors
	#define BT_LEFT    223
	#define BT_RIGHT   355
	#define BT_WINDOW  35 // margin
	// 1k resistors
	//#define BT_LEFT    66
	//#define BT_RIGHT   41
	//#define BT_WINDOW  12 // margin

	int16_t valor = analogRead(KP_PIN);
	if ((valor > BT_LEFT-BT_WINDOW) && (valor < BT_LEFT+BT_WINDOW)) {
		return 1; // left button
	} else
	if ((valor > BT_RIGHT-BT_WINDOW) && (valor < BT_RIGHT+BT_WINDOW)) {
		return 2; // right button
	} else
		return 0; // null
}

void showTime(uint32_t t)
{
	int8_t cifers[4];
	for (int8_t i=3; i>=0; i--)
	{
		t /= 10; // we do this beforehand because only 4 digits needed <- discard last position
		cifers[i] = t % 10;
	}
	if ( cifers[0] == 0 ) cifers[0]=0x7f; // empty
	display.display(cifers);
}

void setup() {
	display.point(true); // clock separator on
	display.set(7); // brightness 0 to 7
	showTime(0); // 0:00
	pinMode(KP_PIN, INPUT_PULLUP); // keypad
}

void loop() {
	currentTime = millis();
	//=====================================================
	//  SENSOR PART
	//
	if (digitalRead(IR_PIN) == LOW)
	// === IR Sensor active ===
	{
		if ( !detectedObject )
		// sensor state change: rising edge
		{
			detectedObject = true;
			if ( !sw.running() )
			// first time seen!
			{
				// check before if protection window has passed
				if ( clearToGo && ((currentTime - PW_windowTime) > CLEAR_WINDOW) ) // only measure if reset was pressed
				{
					sw.start();
					clearToGo = false;
					PW_windowTime = currentTime; // protect!
				}
			}
			else
			// in a lap!
			{
				// check before if protection window has passed
				if ( (currentTime - PW_windowTime) > CLEAR_WINDOW )
				{
					sw.lap();
					PW_windowTime = currentTime; // protect!
					showLap = true;
					showLapTime = currentTime;
				}
			}
		}
	}
	else
	// === IR Sensor inactive ===
	{
		if ( detectedObject )
		// sensor state change: falling edge
		{
			detectedObject = false;
		}
	}
	//=====================================================
	//  KEYPAD PART
	//
	byte key = readKeypad();
	if ( key == 1 )
	// left button
	{
		clearToGo = true;
		sw.reset();
		showTime(0);
		delay(250); // avoid bouncing
	}
	if ( (key == 2) && !sw.running() )
	// right button
	{
		currentTime = sw.getLapTime(whatLap);
		showTime(currentTime);
		whatLap++;
		if ( whatLap >= NUM_LAPS ) whatLap = 0;
		delay(350); // avoid bouncing
	}
	//=====================================================
	//  SHOW TIME PART
	//
	if ( sw.running() )
	{
		if ( showLap )
		// show previous lap time
		{
			showLap = (currentTime - showLapTime) < SHOW_LAP_TIME;
			currentTime = sw.getPreviousLapTime();
		}
		else
		// show current time
		{
			currentTime = sw.getTime();
		}
		showTime(currentTime);
	}
}
