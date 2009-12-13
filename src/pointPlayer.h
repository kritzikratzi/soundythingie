#ifndef __PEEPEE
#define __PEEPEE


#include "ofMain.h"
#include "pointRecorder.h" 

class pointPlayer{
	public: 
		pointPlayer(); 
		void setup( pointRecorder * pr ); 
		void draw(); 
		void update(); 
		void audioRequested(float * output, int bufferSize, int nChannels); 	
	
		int triggerRate; 
		pointRecorder * pr; 

		//------------------- for the simple sine wave synthesis
		float 	targetFrequency;
		float 	phase;
		float 	phaseAdder;
		float 	phaseAdderTarget;
		float	timeCounter;
		float	timeOfLastFrame;
		float 	volume;
		float 	pan;
		float 	sampleRate;
	
		bool	suicide; 
		static int idCount; 
		int id; 
};

#endif