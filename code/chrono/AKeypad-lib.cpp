/**
 * AKeypad-lib is a library to handle analog keypads.
 * 
 * Momentary switches in cascade with 10K resistors connected to an analog input.
 *
 * @file      AKeypad-lib.cpp
 * @author    mgesteiro
 * @date      20230217
 * @version   1.0.0
 * @copyright OpenSource, LICENSE GPLv3
 */

#include "AKeypad-lib.h"

// for code clarity
#define OFF      0
#define ON       1
#define STALLED  2
#define CURRENT  0
#define SAVED    1

AKeypad::AKeypad(
	uint8_t keypadPin,
	uint8_t numKeys,
	int16_t* keysValues,
	uint32_t lpDuration,
	uint32_t dbTime,
	uint32_t chkInterval)
{
	_pin = keypadPin;
	_num_keys = numKeys;
	_keys_values = new int16_t[_num_keys + 1];  // extra space for NONE key
	_keys_values[0] = 1023;  // Max analogRead() --> value for NONE key
	for (uint8_t i = 0; i < _num_keys; i ++)
		_keys_values[i + 1] = keysValues[i];
	LP_MIN_DURATION = lpDuration;
	DB_TIME = dbTime;
	CHECK_MIN_INTERVAL = chkInterval;
}

AKeypad::~AKeypad()
{
	delete [] _keys_values;
}

void AKeypad::clear()
{
	memset(_key, 0, sizeof(_key));
	memset(_key_state, OFF, sizeof(_key_state));
	memset(_pressed_time, 0, sizeof(_pressed_time));
	_nextCheckTime = millis();  // now
}

void AKeypad::init()
{
	pinMode(_pin, INPUT_PULLUP);  // 2-wires, works too if it's externally pulled-up
	clear();  // initialize internal state
}

int16_t AKeypad::readRawValue()
{
	return analogRead(_pin);
}

uint8_t AKeypad::getPressedKey()
{
	uint8_t result;
	int16_t value, diff, minor_diff;

	// defaults
	minor_diff = 1023;  // MAX analog read, worst case scenario
	result = 0;  // <- no key

	// check buttons
	value = readRawValue();
	for (uint8_t i = 0; i <= _num_keys; i ++)
	{
		diff = abs(_keys_values[i] - value);
		if (diff < minor_diff) {
			minor_diff = diff;
			result = i;
		}
	}
	return result;
}

uint8_t AKeypad::handleKeypad(uint32_t currentTime)
{
	// leave if it's not time to chek yet
	if (currentTime < _nextCheckTime) return 0;

	// time to check!
	_nextCheckTime = currentTime + CHECK_MIN_INTERVAL;  // mark next time we should check
	uint8_t result = 0;

	/* Check debouncing windows (initial/final) and perform a reading if cleared */
	if
	(
		(_key_state[CURRENT] == ON  && (currentTime - _pressed_time) > DB_TIME) // IDB - Initial Debouncing
		||
		(_key_state[CURRENT] == OFF && (currentTime - _held_time) > DB_TIME) // FDB - Final Debouncing
	)
	{
		// debouncing window correctly cleared -> let's read
		_key[CURRENT] = getPressedKey();

		// check for key change: while keeping a key pressed, another one is pressed
		/*
		being an analog keyboard, it doesn't make much sense, so we ignore this:
		the keycode returned is anyhow the saved one (i.e., the new key is ignored!).
		if (_key[CURRENT] && _key[SAVED] && (_key[CURRENT] != _key[SAVED]))
		{
			// do something 
		}
		*/

		// store key state: ON or OFF
		_key_state[CURRENT] = _key[CURRENT] ? ON : OFF; // if _key >0  -> a key was detected -> state = ON
	}

	/* Check state changes */
	// PRESSED: from OFF to ON
	if (_key_state[SAVED] == OFF && _key_state[CURRENT] == ON)
	{
		// save data
		_key[SAVED] = _key[CURRENT];
		_pressed_time = currentTime;
		_held_time = _pressed_time;
		_key_state[SAVED] = ON;
		// PRESSED event
		result = AKP_EVT_PRESSED << 4 | _key[SAVED];
	}
	else
	// LONG-PRESSED: still ON after a while
	if (_key_state[SAVED] == ON && _key_state[CURRENT] == ON)
	{
		// update held time
		_held_time = currentTime;
		if ((_held_time - _pressed_time) > LP_MIN_DURATION)
		{
			// save data
			_key_state[SAVED] = STALLED;
			// LONGPRESSED event
			result = AKP_EVT_LONGPRESSED << 4 | _key[SAVED];
		}
	}
	else
	// RELEASED: from ON to OFF
	if (_key_state[SAVED] == ON && _key_state[CURRENT] == OFF)
	{
		// save data
		_held_time = currentTime;
		_key_state[SAVED] = OFF;
		// RELEASED event
		result = AKP_EVT_RELEASED << 4 | _key[SAVED];
		// update key
		_key[SAVED] = 0; // == NONE == _key[CURRENT];
	}
	else
	// LONG-RELEASED: from STALLED to OFF
	if (_key_state[SAVED] == STALLED && _key_state[CURRENT] == OFF)
	{
		// save data
		_held_time = currentTime;
		_key_state[SAVED] = OFF;
		// LONGRELEASED event
		result = AKP_EVT_LONGRELEASED << 4 | _key[SAVED];
		// update key
		_key[SAVED] = 0; // == NONE == _key[CURRENT];
	}
	// result
	return result;
}  // handleKeypad()
