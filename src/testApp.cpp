#include "testApp.h"
#include "ofMain.h"

//--------------------------------------------------------------
void testApp::setup(){

	
	// macs by default run on non vertical sync, which can make animation very, very fast
	// this fixes that: 
	
	ofSetVerticalSync(true);
	ofSetFrameRate(100);
	
	// set background: 
	
	ofBackground(30,30,30);

	
	timeCounter			= 0;
	timeOfLastFrame		= ofGetElapsedTimef();
	
	// ------------------------------------ audio stuff: 
	// 2 output channels, 
	// 0 input channels
	// 44100 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	
	sampleRate 			= 44100;
	phase 				= 0;
	phaseAdder 			= 0.1f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	pan					= 0.5f;
	bNoise 				= false;

	ofSoundStreamSetup(2,0,this, sampleRate,512, 4);
	
} 

//--------------------------------------------------------------
void testApp::update(){

	// figure out how much time elapsed from frame to frame:
	float diffTime		= ofGetElapsedTimef() - timeOfLastFrame;
	timeOfLastFrame		= ofGetElapsedTimef(); 
	timeCounter			+= diffTime;
	
	
	if (PR.bAmRecording == false && PR.pts.size() > 1){
		ofPoint vel = PR.getVelocityForTime(timeCounter);
		ofPoint pt = PR.getPointForTime(timeCounter);
		
		float lengthOfVel = sqrt(vel.x * vel.x + vel.y * vel.y);
		if (lengthOfVel > 40) lengthOfVel = 40;
		volume = 0.94f * volume + 0.06f *( powf((lengthOfVel / 40.0f),2.0) * 0.5f);
		int width = ofGetWidth();
		pan = 0.95f * pan + 0.05f * ((float)pt.x / (float)width);
		float height = (float)ofGetHeight();
		float heightPct = ((height-pt.y) / height);
		targetFrequency = 1000.0f * heightPct;
		phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	} else {
		volume = 0;
		targetFrequency = 100;
		phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	
	PR.draw();
	
	ofSetColor(255,255,255);
	ofFill();
	ofSetRectMode(OF_RECTMODE_CENTER);
	// if we are not recording, get the point for a given time: 
	if (PR.bAmRecording == false && PR.pts.size() > 1){
		
		ofPoint vel = PR.getVelocityForTime(timeCounter);
		ofPoint pt = PR.getPointForTime(timeCounter);
		
		float lengthOfVel = sqrt(vel.x * vel.x + vel.y * vel.y);
		//if (lengthOfVel > 20 ) cout << "vel length = " << lengthOfVel << endl;
		float angle = atan2(vel.y, vel.x);
		
		glPushMatrix();
		glTranslatef(pt.x, pt.y, 0);
		glRotatef(angle * RAD_TO_DEG, 0,0,1);
		ofRect(0,0, 3 + lengthOfVel , 10 + lengthOfVel/3);
		glPopMatrix();
		
	}
		
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
		
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	PR.addPoint( ofPoint(x,y,0) );
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	PR.bAmRecording = true;
	PR.clear();
}

//--------------------------------------------------------------
void testApp::mouseReleased(){
	PR.addPoint( ofPoint(mouseX, mouseY,0) );
	PR.bAmRecording = false;
	timeCounter = 0;
}


//--------------------------------------------------------------
void testApp::audioRequested(float * output, int bufferSize, int nChannels){	
	
	float leftScale = 1 - pan;
	float rightScale = pan;
	
	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
	
	phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
	
	for (int i = 0; i < bufferSize; i++){
			phase += phaseAdder;
			float sample = sin(phase);
			output[i*nChannels    ] = sample * volume * leftScale;
			output[i*nChannels + 1] = sample * volume * rightScale;
		}
	
	
}

