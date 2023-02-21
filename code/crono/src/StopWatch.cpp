#include <Arduino.h>
#include <StopWatch.h>

//////////////////////////////////////////////////////////////////////////////
// PUBLIC
//
StopWatch::StopWatch(uint8_t nlaps)
{
	// hard limit = 10 laps
	if ( nlaps>10 ) _nlaps=10; else _nlaps=nlaps;
	_laps = new uint32_t[_nlaps];
}

StopWatch::~StopWatch()
{
	delete [] _laps;
}

void StopWatch::start()
{
	_currentLap    = 0;
	_startTime     = millis();
	_measuringTime = true;
}

void StopWatch::lap()
{
	uint32_t now = millis();
	_laps[_currentLap] =  now - _startTime;
	_startTime = now;
	_currentLap++;
	if ( _currentLap >= _nlaps )
	{
		// stop counting
		_measuringTime = false;
		_currentLap = _nlaps - 1; // current lap = last lap
	}
}

void StopWatch::stop()
{
	if ( _measuringTime )
	{
		_laps[_currentLap] = millis() - _startTime;
		_measuringTime = false;
	}
}

void StopWatch::reset()
{
	_measuringTime = false;
	_currentLap    = 0;
	memset(_laps, 0, sizeof(_laps[0])*_nlaps);
}

bool StopWatch::running()
{
	return _measuringTime;
}

unsigned long StopWatch::getLapTime(uint8_t lap)
{
	if ( (lap >= 0) && (lap < _nlaps) ) return _laps[lap];
	else return 0;
}

unsigned long StopWatch::getPreviousLapTime()
{
	if ( _currentLap > 0 ) 
		return getLapTime(_currentLap-1);
	else
		return getLapTime(_nlaps-1); // last <- round-robin
}

unsigned long StopWatch::getTime()
{
	if ( _measuringTime ) return (millis() - _startTime);
	else return getLapTime(_currentLap);
}
