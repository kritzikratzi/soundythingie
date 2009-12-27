#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
//#include "ofAddons.h"

#include "pointRecorder.h"
#include "pointPlayer.h" 

class testApp : public ofSimpleApp{

	public:

		void setup();
		void update();
		void draw();
	
		void keyPressed  (int key);
		void keyReleased (int key);

		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased();
		void pairUpWithAnyPlayer( pointRecorder * pr ); 
		bool inRect( float pX, float pY, float x, float y, float width, float height ); 
	
		float triggerAlpha[6]; 
	
		float			timeCounter;
		float			timeOfLastFrame;
	
		
		// --------------- for the audio
		void audioRequested(float * input, int bufferSize, int nChannels); 

		float 	pan;
		int		sampleRate;
		bool 	bNoise;
		float 	volume;

		float 	* lAudio;
		float   * rAudio;

		//------------------- for the simple sine wave synthesis
		float 	targetFrequency;
		float 	phase;
		float 	phaseAdder;
		float 	phaseAdderTarget;
	
		pointRecorder recorders[100];
		pointPlayer players[100];
		
		int whichRecorder; 
		int beatMod; 
	
		int lineToDelete; 
	
	
		// mouse focus? 
		int spawnFocusRecorder; 
		int spawnFocusPoint; 
	
		bool useEnvelope; 
};

#endif

