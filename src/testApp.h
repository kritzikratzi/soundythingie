#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
//#include "ofAddons.h"

#include "pointRecorder.h"

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
	
		pointRecorder	PR;
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
};

#endif

