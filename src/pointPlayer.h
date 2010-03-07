#ifndef __PEEPEE
#define __PEEPEE


#include "pointRecorder.h"
#include "Tones.h"
#include "Helpers.h"

// shape functions
float shapeFlat( float t ); 
float shapeSinus( float t ); 
float shapeSawtooth( float t ); 
float shapeRectangle( float t ); 

class pointRecorder; 

class pointPlayer{
	public: 
		pointPlayer(); 
		void setup( pointRecorder * pr ); 
		void draw(); 
		void update(); 
		void audioRequested(float * output, int bufferSize, int nChannels, bool useEnvelope); 	
		void doCrazyMath( bool apply ); 
	
	
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
		float	amplitudeTarget; 
		float 	pan;
		float 	panTarget;
		float 	sampleRate;
		float	diffTime; 
		float   envelopeScale;
		
		bool	suicide; 
		bool	dead; 
		static int idCount; 
		int id; 
	
		ofPoint currentPoint;
		ofPoint currentVelocity; 
	
		// we use an "attack-hold-release" envelope. 
		// for t=0...attackTime: attack
		//     t=attackTime...releaseTime: hold
		//     t=releaseTime...duration: release
		float attackTime; 
		float releaseTime; 
		int samplesSinceUpdate; 
	
		// how long should we wait before starting? 
		float startDelay; 
		float startTime; 
};

#endif