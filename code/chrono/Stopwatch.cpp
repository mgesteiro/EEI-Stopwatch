/**
 * A library to reproduce a stopwatch functionality.
 *
 * @file      Stopwatch.cpp
 * @author    mgesteiro
 * @date      20180429
 * @date      20230218
 * @version   1.1.0
 * @copyright OpenSource, LICENSE GPLv3
 */

#include <Arduino.h>
#include "Stopwatch.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC
//
Stopwatch::Stopwatch(uint8_t nLaps)
{
	// hard-limit == 10 laps <-- default value
	if (nLaps > 10) _nlaps = 10;
	else if (nLaps < 1) _nlaps = 1;
	else _nlaps = nLaps;
	
	_laps = new uint32_t[_nlaps];
	reset();
}

Stopwatch::~Stopwatch()
{
	delete [] _laps;
}

void Stopwatch::start()
{
	if (_measuringTime) return; // already started
	_startTime     = millis() - _laps[_currentLap];  // start counting where we left it (may be 0)
	_measuringTime = true;
}

void Stopwatch::lap()
{
	if (! _measuringTime) return; // not running
	uint32_t now = millis();
	_laps[_currentLap] =  now - _startTime;
	_startTime = now;
	_currentLap ++;
	if (_currentLap >= _nlaps)
	{
		// stop counting
		_measuringTime = false;
		_currentLap = _nlaps - 1; // current lap = last lap
	}
}

void Stopwatch::stop()
{
	if ( _measuringTime )
	{
		_laps[_currentLap] = millis() - _startTime;
		_measuringTime = false;
		_startTime = 0;
	}
}

void Stopwatch::abort()
{
	if ( _measuringTime )
	{
		_measuringTime = false;
		_startTime = 0;
	}
}

void Stopwatch::reset()
{
	_measuringTime = false;
	_startTime     = 0;
	_currentLap    = 0;
	memset(_laps, 0, sizeof(_laps[0]) * _nlaps);
}

bool Stopwatch::running()
{
	return _measuringTime;
}

uint32_t Stopwatch::getLapTime(uint8_t lNumber)
{
	if (lNumber < _nlaps) return _laps[lNumber];
	else return 0;
}

uint32_t Stopwatch::getPreviousLapTime()
{
	if ( _currentLap > 0 ) 
		return getLapTime(_currentLap - 1);
	else
		return getLapTime(_nlaps - 1); // last <- round-robin
}

uint32_t Stopwatch::getTime()
{
	if ( _measuringTime ) return (millis() - _startTime);
	else return getLapTime(_currentLap);
}
