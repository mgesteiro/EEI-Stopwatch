/**
 * AKeypad-lib is a library to handle analog keypads.
 * 
 * Momentary switches in cascade with 10K resistors connected to an analog input.
 *
 * @file      AKeypad-lib.h
 * @author    mgesteiro
 * @date      20230217
 * @version   1.0.0
 * @copyright OpenSource, LICENSE GPLv3
 */

#ifndef AKEYPAD_LIB_H
#define AKEYPAD_LIB_H

#define AKP_VERSION "1.0.0"

#include <Arduino.h>
#include <stdint.h>


/**
 * Definition of all the possible EVENTs handling an analog keypad.
 */
typedef enum
{
	AKP_EVT_NONE         = 0,
	AKP_EVT_PRESSED      = 1,
	AKP_EVT_RELEASED     = 2,
	AKP_EVT_LONGPRESSED  = 3,
	AKP_EVT_LONGRELEASED = 4
} T_AKP_EVENTS;

/**
 * Main class implementing the stopwatch functionality.
 */
class AKeypad {
public:
	/**
	 * Constructor
	 *
	 * @param keypadPin  Analog pin to which the keypad is attached and to read input from
	 * @param numKeys  Number of keys the keypad has
	 * @param keysValues  An array of uint16_t values corresponding to every key when pressed.
	 * @param lpDuration Long press duration, in milliseconds. Default: 900
	 * @param dbTime  Debounce time, in milliseconds. Default: 30
	 * @param chkInterval Minimum check interval, in milliseconds. Default: 5
	 */
	AKeypad(
		uint8_t keypadPin,
		uint8_t numKeys,
		int16_t* keysValues,
		uint32_t lpDuration = 900,
		uint32_t dbTime = 30,
		uint32_t chkInterval = 5);
	/**
	 * Destructor
	 */
	~AKeypad();

	/**
	 * Clear all internal states of the keypad management process.
	 */
	void clear();

	/**
	 * Initialization function to be called in Arduino's setup().
	 *
	 * This function initializes the pin for the keypad as INPUT_PULLUP. If the pin has no internal
	 * pull-up resistor but is externally pulled-up, this works fine also.
	 *
	 * @see an init function is required due to Arduino's architecture and constructors should be avoided:
	 *      https://forum.arduino.cc/t/analogread-seems-not-working-into-a-class-constructor/109081/8
	 */
	void init();

	/**
	 * Lowest level reading function of the keypad input pin.
	 * 
	 * @return Analog reading output of the keypad pin.
	 */
	int16_t readRawValue();

	/**
	 * Scans the keypad pin and returns the current active key.
	 * 
	 * This is a low level raw function: no logic is performed.
	 *
	 * @return The current active (closed) key. It may be none [== 0].
	 */
	uint8_t getPressedKey();

	/**
	 * Processes the key strokes and converts them, using timing, into useful info.
	 * It also performs key press/release debouncing. It has an autolimited resolution
	 * of 5 milliseconds (by default), that can be configured in the class constructor.
	 * 
	 * This is a high level management function, valid for logic control.
	 * This function should be called in the loop() as often as possible.
	 *
	 * @param currentTime  Current time in milliseconds should be provided.
	 *
	 * @return  lo nibble -> key code,  hi nibble -> event [T_AKP_EVENTS]
	 */
	uint8_t handleKeypad(uint32_t currentTime);


private:
	uint32_t LP_MIN_DURATION;     // long-press minimum duration, ms
	uint32_t DB_TIME;             // debouncing time, ms
	uint32_t CHECK_MIN_INTERVAL;  // checking minimum interval, ms

	uint8_t  _key[2];             // key being processed: current and saved
	uint8_t  _key_state[2];       // key state of the key being processed: current and previous
	uint32_t _pressed_time;       // moment when the key being processed was pressed
	uint32_t _held_time;          // last moment the key being processed continues to be pressed
	uint32_t _nextCheckTime;      // next moment we have to check values
	
	uint8_t  _pin;                // pin in use
	uint8_t  _num_keys;           // number of keys in the keypad
	int16_t* _keys_values;        // array of analog reading values, for each key
};

#endif  //library
