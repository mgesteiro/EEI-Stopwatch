class StopWatch
{
	public:
		// constructor
		StopWatch(uint8_t nlaps = 10);
		~StopWatch();

		// methods
		void start();
		void lap();
		void stop();
		void reset();
		bool running();
		unsigned long getLapTime(byte lap);
		unsigned long getPreviousLapTime();
		unsigned long getTime();

	private:
		uint32_t _startTime;
		uint32_t *_laps;
		uint8_t _nlaps, _currentLap = 0;
		bool _measuringTime = false;

};
