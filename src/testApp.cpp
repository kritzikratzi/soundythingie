#include "testApp.h"
#include "ofMain.h"

//--------------------------------------------------------------
void testApp::setup(){

	
	// macs by default run on non vertical sync, which can make animation very, very fast
	// this fixes that: 
	
	ofSetVerticalSync( true );
	ofSetFrameRate( 60 );
	
	// set background: 
	
	ofBackground( 30,30,30 );

	
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
	
	spawnFocusPoint = -1; 
	spawnFocusRecorder = -1; 

	ofSoundStreamSetup(2,0,this, sampleRate,256, 4);
	lineToDelete = -1; 
	beatMod = 32; 
	useEnvelope = false; 
} 

//--------------------------------------------------------------
void testApp::update(){
	for( int i = 0; i < 100; i++ ){
		if( !players[i].suicide ){
			players[i].update(); 
		}
	}
	
	
	
	// trigger players! 
	for( int i = 0; i < 100; i++ ){
		if( recorders[i].beatMod > 0 && recorders[i].startTime != 0 && ofGetFrameNum() % recorders[i].beatMod == 0 ){
			pairUpWithAnyPlayer( &recorders[i] ); 
		}
	}
	
	// trigger kids!
	//cout << "---------------" << endl; 
	for( int i = 0; i < 100; i++ ){
		if( recorders[i].kids.size() > 0 ){
			
			pointRecorder * rec = &recorders[i]; 
			for( int j = 0; j < rec->kids.size(); j++ ){
				float when = rec->pts[rec->kidPointNr[j]].time; 
				float duration = rec->pts[rec->pts.size()-1].time; 
				double x1, x2; 
				
				// is there a player that JUST played this? 
				for( int k = 0; k < 100; k++ ){
					x2 = players[k].timeCounter; 
					x1 = x2 - (double)players[k].diffTime; 
					
					if( players[k].suicide == false && players[k].pr == rec && x1 <= when && when < x2 ){
						// cout << "play line nr " << rec->kids[j] << ", kid of line nr. " << i << "; j=" << j << ", k= " << k << endl; 
						pairUpWithAnyPlayer( &(recorders[rec->kids[j]]) ); 
						k = 100; // "break" on the k-level
					}
				}
			}
				
		}
	}
	
	
	// trigger-fade-effect for the beat-mod selectors (no one understands what i mean, right?) 
	for( int i = 1; i < 6; i++ ){
		triggerAlpha[i] -= triggerAlpha[i]/30.; 
		
		if( ofGetFrameNum() % (16<<i) == 0 ){
			triggerAlpha[i] = 1; 
		}
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
	
	for( int i = 0; i < 6; i++ ){
		if( this->inRect( mouseX*1.0, mouseY*1.0, 5+i*30.0, 5.0, 20.0, 20.0 ) || (16<<i) == beatMod ){
			ofSetColor( 50+205*triggerAlpha[i], 0, 0 ); 
		}
		else{
			ofSetColor( 50*triggerAlpha[i], 50*triggerAlpha[i], 50*triggerAlpha[i] ); 
		}
		ofRect( 5+i*30, 5, 20, 20 ); 
		ofSetColor( 150, 150, 150 ); 
		ofDrawBitmapString( ofToString( i, 0 ), 11+i*30, 19 );
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
		float radius = 2+2*recorders[lineToDelete].volume/0.1;
		ofCircle( recorders[lineToDelete].pts[0].pos.x, recorders[lineToDelete].pts[0].pos.y, radius ); 
	}
	
	if( spawnFocusPoint >= 0 ){
		ofNoFill(); 
		ofSetColor( 255, 255, 0 ); 
		ofPoint *p = &recorders[spawnFocusRecorder].pts[spawnFocusPoint].pos; 
		ofCircle( p->x, p->y, 5 ); 
	}
}

void testApp::pairUpWithAnyPlayer( pointRecorder * pr ){
	for( int i = 0; i < 100; i++ ){
		if( players[i].suicide ){
			players[i].setup( pr ); 
			return; 
		}
	}
	
	cout << "there's toooo much going on. i can't focus. aaaaah!" << endl; 
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	// set beat-mod using the keyboard
	if( key >= '1' && key <= '5' ){
		beatMod = 32<<(key-'1'); 
	}
	else if( key == '0' ){
		beatMod = 0; 
	}
	
	if( key == 'c' ){
		for( int i = 0; i < 100; i++ ){
			//recorders[i].clear(); 
			recorders[i].startTime = 0; 
			//players[i].suicide = true; 
		}
	}
	
	
	if( key == '-' && lineToDelete >= 0 ){
		recorders[lineToDelete].volume -= 0.01; 
		if( recorders[lineToDelete].volume < 0 ) recorders[lineToDelete].volume = 0; 
	}

	if( key == '+' && lineToDelete >= 0 ){
		recorders[lineToDelete].volume += 0.01; 
		if( recorders[lineToDelete].volume > 1 ) recorders[lineToDelete].volume = 1; 
	}
	
	if( key == 'd' && lineToDelete >= 0 ){
		// delete! 
		mousePressed(mouseX, mouseY, 1);
	}
	
	if( key == 'f' ){
		ofSetFrameRate( ofGetFrameRate()+1 ); 
		cout << ofGetFrameRate() << endl; 
	}	
	
	if( key == 's' ){
		ofSetFrameRate( ofGetFrameRate()-1 ); 
		cout << ofGetFrameRate() << endl; 
	}
	
	if( key == 'e' ){
		useEnvelope = !useEnvelope; 
		cout << "Envelope: " << useEnvelope << endl; 
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	lineToDelete = -1; 
	
	
	// are we really really close to a line? 
	if( whichRecorder == -1 ){
		float dx, dy; 
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
	
	spawnFocusPoint = -1; 
	spawnFocusRecorder = -1; 
	
	if( lineToDelete == -1 && whichRecorder == -1 ){
		float minDistance = 100; 
		float distance = 100;
		float dx, dy; 
		
		for( int i = 0; i < 100; i++ ){
			if( recorders[i].startTime != 0 && recorders[i].pts.size() > 0 ){
				for( int j = 0; j < recorders[i].pts.size(); j++ ){
					// are we really close to the first point? 
					dx = mouseX - recorders[i].pts[j].pos.x; 
					dy = mouseY - recorders[i].pts[j].pos.y; 
					distance = sqrt( dx*dx + dy*dy ); 
					if( distance < 10 && distance < minDistance ){
						minDistance = distance; 
						spawnFocusRecorder = i; 
						spawnFocusPoint = j; 
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	if( whichRecorder >= 0 ){
		recorders[whichRecorder].addPoint( ofPoint(x,y,0) );
	}
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	// no mouse-thingie whatsover when hovering over the controls
	if( mouseX <= 150 && mouseY <= 30 ){
		
		for( int i = 0; i < 6; i++ ){
			if( this->inRect( mouseX*1.0, mouseY*1.0, 5+i*30.0, 5.0, 20.0, 20.0 ) ){
				if( i == 0 ){
					beatMod = 0; 
				}
				else{
					beatMod = 16 << i; 
				}
				return; 
			}
		}
		
		
		
		return; 
	}
	
	
	if( lineToDelete >= 0 ){
		recorders[lineToDelete].startTime = 0; 
		lineToDelete = -1; 
		return; 
	}
	
	for( int i = 0; i < 100; i++ ){
		if( recorders[i].startTime == 0 ){
			whichRecorder = i; 
			recorders[whichRecorder].reset( this->beatMod ); 
			
			if( spawnFocusRecorder >= 0 ){
				pointRecorder * rec = &recorders[spawnFocusRecorder]; 
				rec->kids.push_back( whichRecorder ); 
				rec->kidPointNr.push_back( spawnFocusPoint );  
				cout << "ADDED KID!!!!" << endl; 
				ofPoint p = rec->pts[spawnFocusPoint].pos; 
				recorders[whichRecorder].addPoint( p );
				recorders[whichRecorder].beatMod = -1; // this will never launch it's own players! 
			}
			
			return; 
		}
	}

	
	// NONE??? 
	whichRecorder = -1; 
}

//--------------------------------------------------------------
void testApp::mouseReleased(){
	if( whichRecorder >= 0 ){
		if( recorders[whichRecorder].pts.size() >= 1 ){
			recorders[whichRecorder].addPoint( ofPoint(mouseX, mouseY,0) );
			recorders[whichRecorder].bAmRecording = false;
			
			if( recorders[whichRecorder].beatMod == 0 ){
				pairUpWithAnyPlayer( &recorders[whichRecorder] ); 
			}
		}
		else{
			recorders[whichRecorder].reset( this->beatMod ); 
		}
		
		timeCounter = 0;
		whichRecorder = -1; 
	}
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
			players[i].audioRequested( output, bufferSize, nChannels, useEnvelope ); 
		}
	}
}

// ... 
bool testApp::inRect( float pX, float pY, float x, float y, float width, float height ){
	return pX >= x && pX <= x + width && pY >= y && pY <= y + height; 
}