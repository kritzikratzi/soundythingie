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

	ofSoundStreamSetup(2,0,this, sampleRate,256, 4);
	lineToDelete = -1; 
	
} 

//--------------------------------------------------------------
void testApp::update(){
	for( int i = 0; i < 100; i++ ){
		if( !players[i].suicide ){
			players[i].update(); 
		}
	}
	
	// figure out how much time elapsed from frame to frame:
	float diffTime		= ofGetElapsedTimef() - timeOfLastFrame;
	timeOfLastFrame		= ofGetElapsedTimef(); 
	timeCounter			+= diffTime;
	
	
	if (recorders[whichRecorder].bAmRecording == false && recorders[whichRecorder].pts.size() > 1){
		ofPoint vel = recorders[whichRecorder].getVelocityForTime(timeCounter);
		ofPoint pt = recorders[whichRecorder].getPointForTime(timeCounter);
		
		float lengthOfVel = sqrt(vel.x * vel.x + vel.y * vel.y);
		if (lengthOfVel > 40) lengthOfVel = 40;
		volume = 0.94f * volume + 0.06f *( powf((lengthOfVel / 40.0f),2.0) * 0.5f);
		int width = ofGetWidth();
		pan = 0.95f * pan + 0.05f * ((float)pt.x / (float)width);
		float height = (float)ofGetHeight();
		float heightPct = ((height-pt.y) / height);
		targetFrequency = 1000.0f * heightPct;
		phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	}
	else {
		volume = 0;
		targetFrequency = 100;
		phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	for( int i = 0; i < 100; i++ ){
		if( recorders[i].startTime != 0 ){
			recorders[i].draw(); 
		}
	}
	
	ofSetRectMode(OF_RECTMODE_CORNER);	
	ofSetColor( 255, 255, 255 );
	ofFill(); 
	for( int i = 0; i < 5; i++ ){
		if( ( ofGetFrameNum() % (32<<i) ) == 0 ){
			if( i == 1 ){
				for( int j = 0; j < 100; j++ ){
					if( recorders[j].startTime != 0 ){
						pairUpWithAnyPlayer( &recorders[j] ); 
					}
				}
			}
			triggerAlpha[i] = 1; 
		}
		
		ofSetColor( 50*triggerAlpha[i], 50*triggerAlpha[i], 50*triggerAlpha[i] ); 
		ofRect( 5+i*30, 5, 20, 20 ); 
		ofSetColor( 150, 150, 150 ); 
		ofDrawBitmapString( ofToString( i+1, 0 ), 11+i*30, 18 );
		
		triggerAlpha[i] -= triggerAlpha[i]/30.; 
	}
	
	for( int i = 0; i < 100; i++ ){
		if( !players[i].suicide ){
			players[i].draw(); 
		}
	}
	
	
	//string report = "nPts = " + ofToString(nPts) + "\ntotal time = " + ofToString(totalDuration, 3);
	//ofDrawBitmapString(report, 10, 10);
	
	if( lineToDelete >= 0 ){
		ofFill(); 
		ofSetColor( 255, 0, 0 ); 
		ofCircle( recorders[lineToDelete].pts[0].pos.x, recorders[lineToDelete].pts[0].pos.y, 6 ); 
	}
}

void testApp::pairUpWithAnyPlayer( pointRecorder * pr ){
	for( int i = 0; i < 100; i++ ){
		if( players[i].suicide ){
			players[i].setup( pr ); 
			cout << "found " << i << endl; 
			return; 
		}
	}
	
	cout << "there's toooo much going on. i can't focus. aaaaah!" << endl; 
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	lineToDelete = -1; 
	
	// are we really really close to a line? 
	if( whichRecorder == -1 ){
		for( int i = 0; i < 100; i++ ){
			if( recorders[i].startTime != 0 && recorders[i].pts.size() > 0 ){
				// are we really close to the first point? 
				float dx = mouseX - recorders[i].pts[0].pos.x; 
				float dy = mouseY - recorders[i].pts[0].pos.y; 
				
				if( sqrt( dx*dx + dy*dy ) < 5 ){
					lineToDelete = i; 
					break; 
				}
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	recorders[whichRecorder].addPoint( ofPoint(x,y,0) );
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	if( lineToDelete >= 0 ){
		recorders[lineToDelete].startTime = 0; 
		lineToDelete = -1; 
	}
	
	for( int i = 0; i < 100; i++ ){
		if( recorders[i].startTime == 0 ){
			whichRecorder = i; 
			recorders[whichRecorder].bAmRecording = true;
			recorders[whichRecorder].clear();
			
			return; 
		}
	}

	
	// NONE??? 
	whichRecorder = 0; 
	recorders[whichRecorder].bAmRecording = true;
	recorders[whichRecorder].clear();
}

//--------------------------------------------------------------
void testApp::mouseReleased(){
	if( recorders[whichRecorder].pts.size() > 0 ){
		recorders[whichRecorder].addPoint( ofPoint(mouseX, mouseY,0) );
		recorders[whichRecorder].bAmRecording = false;
	}
	else{
		recorders[whichRecorder].clear();
		recorders[whichRecorder].startTime = 0; 
	}
	
	timeCounter = 0;
	whichRecorder = -1; 
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
			//phase += phaseAdder;
			//float sample = sin(phase);
		//output[i*nChannels    ] = sample * volume * leftScale;
		//output[i*nChannels + 1] = sample * volume * rightScale;
		output[i*nChannels    ] = 0; 
		output[i*nChannels + 1] = 0; 
	}
	
	for( int i = 0; i < 100; i++ ){
		if( !players[i].suicide ){
			players[i].audioRequested( output, bufferSize, nChannels ); 
		}
	}
}