/**
 * A library to reproduce a stopwatch functionality.
 *
 * @file      Stopwatch.h
 * @author    mgesteiro
 * @date      20180429
 * @date      20230218
 * @version   1.1.0
 * @copyright OpenSource, LICENSE GPLv3
 */

#ifndef STOPWATCH_LIB_H
#define STOPWATCH_LIB_H

#define SWL_VERSION "1.1.0"

class Stopwatch
{
	public:
		// constructor
		/**
		 * Constructor
		 *
		 * @param nLaps Maximum number of laps to store. Hard-limited between 1 and 10.
		 */
		Stopwatch(uint8_t nLaps = 10);

		/**
		 * Destructor: releases the reserved memory.
		 */
		~Stopwatch();

		/**
		 * Starts the stopwatch if it was in a "reset" state (via reset() method).
		 * Resumes the stopwatch if it was in a "stopped" state (via stop() method).
		 * Does nothing if already running.
		 */
		void start();

		/**
		 * Stores current lap time while continues counting a new one.
		 * If the maximum number of laps has been reached, it stops running.
		 * It does nothing if it's not running.  
		 */
		void lap();

		/**
		 * Stores current lap time and stops running. It does nothing if not running.
		 */
		void stop();

		/**
		 * Stops running and discards current lap. It does nothing if not running.
		 */
		void abort();

		/**
		 * Resets the stopwatch: stops counting and clears lap times.
		 */
		void reset();

		/**
		 * Indicates if the stopwatch is currently running.
		 * 
		 * @return true if the stopwatch is running, false otherwise.
		 */
		bool running();

		/**
		 * Gets the time for the indicated lap.
		 *
		 * @param lNumber  The lap number to retrieve (from 0 to maximum number of laps -1)
		 *
		 * @return The time corresponding to the indicated lap number. 0 if the lap number is invalid.
		 */
		uint32_t getLapTime(byte lNumber);

		/**
		 * Gets the time for the previous lap to the one actually counting.
		 *
		 * @return The time corresponding to the lap previous to the one actually counting.
		 *         If it is the first lap, it returns the last of the list (round-robin).
		 */
		uint32_t getPreviousLapTime();

		/**
		 * Gets the current lap running time.
		 *
		 * @return The current lap counting time if running, the last measured lap if not.
		 */
		uint32_t getTime();

	private:
		uint32_t  _startTime;
		uint32_t* _laps;
		uint8_t   _nlaps, _currentLap;
		bool      _measuringTime;

};

#endif
