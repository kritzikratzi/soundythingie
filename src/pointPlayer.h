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
		void audioRequested(float * output, int bufferSize, int nChannels, bool useEnvelope); 	
	
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
		float	amplitude; 
		float 	pan;
		float 	sampleRate;
		float	diffTime; 
	
		bool	suicide; 
		static int idCount; 
		int id; 
	
		// we use an "attack-hold-release" envelope. 
		// for t=0...attackTime: attack
		//     t=attackTime...releaseTime: hold
		//     t=releaseTime...duration: release
		float attackTime; 
		float releaseTime; 
		int samplesSinceUpdate; 
};

#endif