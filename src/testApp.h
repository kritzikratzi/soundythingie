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
		void drawPR( pointRecorder pr ); 
	
		void keyPressed  (int key);
		void keyReleased (int key);

		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased();
		void pairUpWithAnyPlayer( pointRecorder pr ); 
	
		vector<pointRecorder> recorders;
		float triggerAlpha[5]; 
		pointRecorder	PR;
		pointPlayer     PP;
	
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
	
		pointPlayer players[100];
};

#endif

