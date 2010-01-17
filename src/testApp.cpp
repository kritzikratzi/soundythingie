#include "testApp.h"
#include "ofMain.h"

//--------------------------------------------------------------
void testApp::setup(){

	
	// macs by default run on non vertical sync, which can make animation very, very fast
	// this fixes that: 
	
	ofSetVerticalSync( true );
	ofSetFrameRate( 60 );
	
	// set background: 
	
	ofBackground( 0,0,0 );
	bFullscreen	= false;
	useEnvelope = true; 
	timeCounter			= 0;
	timeOfLastFrame		= ofGetElapsedTimef();
	lastMousePressed = 0; 
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
	
	showAudio = false; 
	
	spawnFocusPoint = -1; 
	spawnFocusRecorder = -1; 

	ofSoundStreamSetup(2,0,this, sampleRate,256, 4);
	lineHovered = -1; 
	beatMod = 32; 
	soundShape = 1; 
	
	chromaticMode = false; 
	
	// load images...
	beatImgs[0].loadImage( "beat_0.png" );
	beatImgs[1].loadImage( "beat_1.png" );
	beatImgs[2].loadImage( "beat_2.png" );
	beatImgs[3].loadImage( "beat_3.png" );
	beatImgs[4].loadImage( "beat_4.png" );
	beatImgs[5].loadImage( "beat_5.png" );
	shapeImgs[0].loadImage( "shape_flat.png" ); 
	shapeImgs[1].loadImage( "shape_sinus.png" ); 
	shapeImgs[2].loadImage( "shape_sawtooth.png" ); 
	shapeImgs[3].loadImage( "shape_rectangle.png" ); 
	envelopeImg.loadImage( "envelope.png" ); 
	selectionImg.loadImage( "selection.png" ); 
} 

//--------------------------------------------------------------
void testApp::update(){
	pmouseX = cmouseX; 
	pmouseY = cmouseY; 
	cmouseX = mouseX; 
	cmouseY = mouseY; 
	
	
	for( int i = 0; i < PLAYERS; i++ ){
		if( !players[i].suicide ){
			players[i].update(); 
		}
	}
	
	
	
	// trigger players! 
	for( int i = 0; i < RECORDERS; i++ ){
		if( !recorders[i].bAmRecording && recorders[i].beatMod > 0 && recorders[i].startTime != 0 && ofGetFrameNum() % recorders[i].beatMod == 0 ){
			pairUpWithAnyPlayer( &recorders[i] ); 
		}
	}
	
	// trigger kids!
	//cout << "---------------" << endl; 
	for( int i = 0; i < RECORDERS; i++ ){
		if( recorders[i].kids.size() > 0 ){
			
			pointRecorder * rec = &recorders[i]; 
			for( int j = 0; j < rec->kids.size(); j++ ){
				float when = rec->pts[rec->kidPointNr[j]].time; 
				float duration = rec->pts[rec->pts.size()-1].time; 
				double x1, x2; 
				
				// is there a player that JUST played this? 
				for( int k = 0; k < PLAYERS; k++ ){
					x2 = players[k].timeCounter; 
					x1 = x2 - (double)players[k].diffTime; 
					
					if( players[k].suicide == false && players[k].pr == rec && x1 <= when && when < x2 ){
						// cout << "play line nr " << rec->kids[j] << ", kid of line nr. " << i << "; j=" << j << ", k= " << k << endl; 
						pairUpWithAnyPlayer( &(recorders[rec->kids[j]]) ); 
						k = PLAYERS; // "break" on the k-level
					}
				}
			}
				
		}
	}
	
	
	// trigger-fade-effect for the beat-mod selectors (no one understands what i mean, right?) 
	for( int i = 1; i < 6; i++ ){
		triggerAlpha[i] -= triggerAlpha[i]/(10+i*2.0); 
		
		if( ofGetFrameNum() % (16<<i) == 0 ){
			triggerAlpha[i] = 1; 
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	Tones::checkInit(); 
	if( chromaticMode ){
		Tones::draw(); 
	}
	
	pointRecorder *pr; 
	ofPoint *pt; 
	ofPoint *pt2; 
	// draw relations between recorders
	glEnable(GL_LINE_STIPPLE);
	glLineStipple( 1, 0xF0F0 ); 
	ofSetColor( 80, 80, 80 ); 
	for( int i = 0; i < RECORDERS; i++ ){
		pr = &recorders[i]; 
		if( pr->startTime != 0 ){
			for( int j = 0; j < pr->kids.size(); j++ ){
				if( recorders[pr->kids[j]].pts.size() > 0 ){
					pt = &pr->pts[pr->kidPointNr[j]].pos; 
					pt2 = &recorders[pr->kids[j]].pts[0].pos;
					ofNoFill(); 
					ofLine( pt->x, pt->y, pt2->x, pt2->y ); 
					ofFill(); 
					ofCircle( pt->x, pt->y, 2 ); 
				}
			}
		}
	}
	glDisable(GL_LINE_STIPPLE);
	// draw recorders
	for( int i = 0; i < RECORDERS; i++ ){
		if( recorders[i].startTime != 0 ){
			recorders[i].draw(); 
		}
	}
	
	ofSetRectMode(OF_RECTMODE_CORNER);	
	ofSetColor( 255, 255, 255 );
	ofNoFill(); 
	ofSetLineWidth( 1 );

	// Draw beat-images
	for( int i = 0; i < 6; i++ ){
		// draw images... 
		int alpha = 120*triggerAlpha[i];
		bool selected = 
			this->inRect( mouseX*1.0, mouseY*1.0, 10.0, 10.0+i*30.0, 20.0, 20.0 ) ||  // mouseover
			(16<<i) == beatMod || (beatMod == 0 && i == 0 ); // selected

		drawImage( &beatImgs[i], 10, 10+i*30, selected, alpha ); 
		
		// a little guide to show "where" in the beat we are
		if( i != 0 ){
			ofSetColor( 150, 150, 150 ); 
			int beat = 16<<i;
			float t = (ofGetFrameNum()%beat)/(float) beat; 
			ofRect( 10.5 + t*18, 11.5 + i*30, 1, 0 ); 
		}
		
	}
	
	// Draw images for wave forms... 
	for( int i = 0; i < 4; i++ ){
		bool selected = this->inRect( mouseX*1.0, mouseY*1.0, 10.0, 220.0+i*30.0, 20.0, 20.0 ) ||  // mouseover
		soundShape == i; // selected
		drawImage( &shapeImgs[i], 10, 220+i*30, selected ); 
	}
	
	// Draw point players
	for( int i = 0; i < PLAYERS; i++ ){
		if( !players[i].suicide ){
			players[i].draw(); 
		}
	}
	
	if( selectionMode ){
		ofSetRectMode(OF_RECTMODE_CORNER);	
		ofEnableAlphaBlending();
		ofSetColor( 0xFFFFFF ); 
		selectionImg.draw( 10, 10+6*30 ); 
		//selectionImg.draw( mouseX + 15, mouseY + 20 ); 
		ofDisableAlphaBlending(); 
		
		
		ofNoFill(); 
		glEnable(GL_LINE_STIPPLE);
		glLineStipple( 2, 0xF0F0 ); 
		ofSetColor( 255, 255, 255 ); 
		ofBeginShape(); 
		
		for( int i = 0; i < selectionLength; i++ ){
			ofVertex( selection[i].x, selection[i].y ); 
		}
		
		ofEndShape( false ); 
		glDisable(GL_LINE_STIPPLE);
	}
	
	//string report = "nPts = " + ofToString(nPts) + "\ntotal time = " + ofToString(totalDuration, 3);
	//ofDrawBitmapString(report, 10, 10);
	for( int i = 0; i < selectedRecorders.size(); i++ ){
		pointRecorder * pr = &recorders[ selectedRecorders[i] ]; 
		ofFill(); 
		ofSetColor( 255, 0, 0 );
		float radius = 2+2*pr->volume/0.1;
		ofCircle( pr->pts[0].pos.x, pr->pts[0].pos.y, radius ); 
	}
	
	if( lineHovered >= 0 ){
		ofFill(); 
		ofSetColor( 255, 0, 0 );
		float radius = 2+2*recorders[lineHovered].volume/0.1;
		ofCircle( recorders[lineHovered].pts[0].pos.x, recorders[lineHovered].pts[0].pos.y, radius ); 
	}
	
	if( spawnFocusPoint >= 0 ){
		ofNoFill(); 
		ofSetColor( 255, 255, 0 ); 
		ofPoint *p = &recorders[spawnFocusRecorder].pts[spawnFocusPoint].pos; 
		ofCircle( p->x, p->y, 5 ); 
	}
	
	if( showAudio ){
		int graphW = 107; 
		int graphH = 107; 
		ofSetLineWidth( 1 ); 
		ofTranslate( 50, 10 );
		ofScale( graphW/256.0f, graphH/200.0f ); 
		ofSetRectMode( OF_RECTMODE_CORNER ); 
		ofSetColor( 0x333333 );
		ofNoFill(); 
		ofRect(   0, 0, 256, 200 );
		ofRect( 300, 0, 256, 200 );
		ofSetColor( 0xFFFFFF );
		for( int i = 0; i < 256; i++ ){
			ofLine(   0 + i, 100,   0 + i ,100 + lAudio[i] * 100.0f );
			ofLine( 300 + i, 100, 300 + i ,100 + rAudio[i] * 100.0f );
		}
	}
	
}

void testApp::pairUpWithAnyPlayer( pointRecorder * pr ){
	for( int i = 0; i < PLAYERS; i++ ){
		if( players[i].suicide ){
			players[i].setup( pr ); 
			return; 
		}
	}
	
	cout << "there's toooo much going on. i can't focus. aaaaah!" << endl; 
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	glutModifiers = glutGetModifiers(); // can only be done in "core input callback"
	
	
	// set beat-mod using the keyboard
	if( key >= '1' && key <= '5' ){
		beatMod = 32<<(key-'1'); 
	}
	else if( key == '0' ){
		beatMod = 0; 
	}
	
	if( key == 'c' ){
		for( int i = 0; i < RECORDERS; i++ ){
			//recorders[i].clear(); 
			recorders[i].startTime = 0; 
			//players[i].suicide = true; 
		}
	}
	
	
	if( key == '-' ){
		if( lineHovered >= 0 ){
			recorders[lineHovered].volume -= 0.01; 
			if( recorders[lineHovered].volume < 0 ) recorders[lineHovered].volume = 0; 
		}
		
		for( int i = 0; i < selectedRecorders.size(); i++ ){
			recorders[selectedRecorders[i]].volume -= 0.01; 
			if( recorders[selectedRecorders[i]].volume < 0 ) recorders[selectedRecorders[i]].volume = 0; 
		}		
	}

	if( key == '+' ){
		if( lineHovered >= 0 ){
			recorders[lineHovered].volume += 0.01; 
			if( recorders[lineHovered].volume > 1 ) recorders[lineHovered].volume = 1; 
		}
		
		for( int i = 0; i < selectedRecorders.size(); i++ ){
			recorders[selectedRecorders[i]].volume += 0.01; 
			if( recorders[selectedRecorders[i]].volume > 1 ) recorders[selectedRecorders[i]].volume = 1; 
		}
	}
	
	if( key == 'd' || key == 127 ){
		// delete the one recorder being hovered, eventually... 
		if( lineHovered >= 0 ){
			deleteRecorder( lineHovered ); 
		}
		
		// delete all recorders in the selection
		for( int i = 0; i < selectedRecorders.size(); i++ ){
			deleteRecorder( selectedRecorders[i] );
		}
		
		// empty selection! 
		selectedRecorders.clear(); 
		cout << "CLEARED SEL REC: " << selectedRecorders.size() << endl; 
	}
	
	if( key == 'f' ){
		bFullscreen = !bFullscreen;
		ofSetFullscreen(bFullscreen);
	}
	
	
	if( key == 'e' ){
		useEnvelope = !useEnvelope; 
		cout << "Envelope: " << useEnvelope << endl; 
	}
	
	if( key == 'a' ){
		showAudio = !showAudio; 
	}
	
	if( key == 's' ){ // fresh selection
		selectionMode = !selectionMode; 
		selectionLength = 0; 
		if( selectionMode ){
			selectedRecorders.clear(); 
		}
	}
	
	if( key == 'S' ){ // add to selection
		selectionMode = !selectionMode; 
		selectionLength = 0; 
	}
	
	if( key == 'i' ){ // invert selection
		vector<int> temp; 
		for( int i = 0; i < selectedRecorders.size(); i++ ) temp.push_back( selectedRecorders[i] ); 
		selectedRecorders.clear(); 
		
		for( int i = 0; i < RECORDERS; i++ ){
			if( !binary_search( temp.begin(), temp.end(), i ) && recorders[i].startTime != 0 ){
				selectedRecorders.push_back( i ); 
			}
		}
	}
	
	if( key == 't' ){
		chromaticMode = true; 
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
	glutModifiers = glutGetModifiers(); // can only be done in "core input callback"

	
	if( key == 't' ){
		chromaticMode = false; 
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	lineHovered = -1; 
	
	// are we really really close to a line? 
	if( whichRecorder == -1 ){
		float dx, dy; 
		for( int i = 0; i < RECORDERS; i++ ){
			if( recorders[i].startTime != 0 && recorders[i].pts.size() > 0 ){
				// are we really close to the first point? 
				float dx = mouseX - recorders[i].pts[0].pos.x; 
				float dy = mouseY - recorders[i].pts[0].pos.y; 
				
				if( sqrt( dx*dx + dy*dy ) < 5 ){
					lineHovered = i; 
					break; 
				}
			}
		}
	}
	
	spawnFocusPoint = -1; 
	spawnFocusRecorder = -1; 
	
	if( lineHovered == -1 && whichRecorder == -1 ){
		float minDistance = 100; 
		float distance = 100;
		float dx, dy; 
		
		
		for( int i = 0; i < RECORDERS; i++ ){
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
	if( chromaticMode ){
		y = Tones::snap( y ); 
	}
	
	if( lineHovered >= 0 ){
		bool moveKids = (glutModifiers & GLUT_ACTIVE_ALT) == 0; 
		cout << glutModifiers << endl; 
		cout << "move kids? " << moveKids << endl; 
		moveRecorder( lineHovered, cmouseX - pmouseX, cmouseY - pmouseY, moveKids ); 
		
		for( int i = 0; i < selectedRecorders.size(); i++ ){
			if( i != lineHovered )
				moveRecorder( selectedRecorders[i], cmouseX - pmouseX, cmouseY - pmouseY, moveKids ); 
		}
		
		for( int i =0; i < RECORDERS; i++ ){
			if( recorders[i].startTime > 0 )
				recorders[i].applyOffset(); 
		}
		
	}
	
	if( selectionMode ){
		selection[selectionLength].x = x; 
		selection[selectionLength].y = y; 
		selectionLength++; 
		
		return; 
	}
	
	if( whichRecorder >= 0 ){
		recorders[whichRecorder].addPoint( ofPoint(x,y,0) );
	}
	
	
	// mouseDragged sometimes gets called twice in between 
	// the updartes. 
	// yes, this is an issue! ... 
	pmouseX = cmouseX; 
	pmouseY = cmouseY; 
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	glutModifiers = glutGetModifiers(); // can only be done in "core input callback"
	lastMousePressed = ofGetElapsedTimef(); 
	
	if( chromaticMode ){
		y = Tones::snap( y ); 
	}
	
	if( selectionMode ){
		selectionLength = 0; 
		selection[selectionLength].x = x; 
		selection[selectionLength].y = y; 
		selectionLength++; 
		return; 
	}

	if( !selectionMode && selectedRecorders.size() > 0 ){
		// Only clear recorders if no line is hovered, otherwise it's
		// perfectly fine to delete! 
		if( lineHovered < 0 ){
			selectedRecorders.clear(); 
		}
	}
	
	
	// handle the buttons... 
	if( mouseX <= 30 ){
		for( int i = 0; i < 6; i++ ){
			if( this->inRect( mouseX*1.0, mouseY*1.0, 10, 10+i*30.0, 20.0, 20.0 ) ){
				beatMod = i==0? 0 : (16<<i); 
				return;
			}
		}
		for( int i = 0; i < 4; i++ ){
			if( this->inRect( mouseX*1.0, mouseY*1.0, 10, 220+i*30.0, 20.0, 20.0 ) ){
				soundShape = i; 
				return; 
			}
		}
	}
	
	
	if( lineHovered >= 0 ){
		//deleteRecorder( lineHovered ); 
		//mouseX
		
		return; 
	}
	
	for( int i = 0; i < RECORDERS; i++ ){
		if( recorders[i].startTime == 0 ){
			whichRecorder = i; 
			recorders[whichRecorder].reset( this->beatMod ); 
			recorders[whichRecorder].soundShape = soundShape; 
			
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
	glutModifiers = glutGetModifiers(); // can only be done in "core input callback"

	if( chromaticMode ){
		mouseY = Tones::snap( mouseY ); 
	}
	
	if( selectionMode ){
		for( int i = 0; i < RECORDERS; i++ ){
			pointRecorder *pr = &recorders[i]; 
			if( pr->startTime != 0 && pr->pts.size() > 0 && inPoly( selection, selectionLength, pr->pts[0].pos ) ){
				selectedRecorders.push_back( i );
			}
		}
		
		// sort and shrink!
		sort( selectedRecorders.begin(), selectedRecorders.end() ); 
		unique( selectedRecorders.begin(), selectedRecorders.end() );
		
		selectionMode = false; 
		
		return; 
	}
	
	if( lineHovered >= 0 && ofGetElapsedTimef() - lastMousePressed < 0.20 ){
		deleteRecorder( lineHovered ); 
		lineHovered = -1; 
	}
	
	
	
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
	
	for( int i = 0; i < PLAYERS; i++ ){
		if( !players[i].suicide ){
			players[i].audioRequested( output, bufferSize, nChannels, useEnvelope ); 
		}
	}
	
	for( int i = 0; i < 256; i++ ){
		lAudio[i] = output[i*nChannels]; 
		rAudio[i] = output[i*nChannels+1]; 
	}
}


// ------------------------
void testApp::deleteRecorder( int rec ){
	recorders[rec].startTime = 0; 
	
	for( int i = 0; i < RECORDERS; i++ ){
		if( i != rec ){
			// remove the recorder from all the recorder it was triggered from 
			// this doesn't have to be the case, it might be... 
			for( int j = 0; j < recorders[i].kids.size(); j++ ){
				if( recorders[i].kids[j] == rec ){
					recorders[i].kids.erase( recorders[i].kids.begin()+j );
					j--; 
				}
			}
		}
		else{
			// delete all the recorders (if there are any) 
			// that were triggered by this recorder
			deleteRecordersKids( i ); 
		}
	}
	
	lineHovered = -1; 
	return; 
}

// ------------------------
void testApp::deleteRecordersKids( int rec ){
	for( int j = 0; j < recorders[rec].kids.size(); j++ ){
		recorders[recorders[rec].kids[j]].startTime = 0;
		deleteRecordersKids( recorders[rec].kids[j] ); 
	}
}

// ------------------------
void testApp::moveRecorder( int rec, int dx, int dy, bool moveKids ){
	recorders[rec].offsetX = dx;
	recorders[rec].offsetY = dy; 
	
	if( moveKids ){
		for( int i = 0; i < recorders[rec].kids.size(); i++ ){
			moveRecorder( recorders[rec].kids[i], dx, dy, true ); 
		}
	}
}

// ------------------------
void testApp::drawImage( ofImage * img, float x, float y, bool selected, float overlay ){
	ofEnableAlphaBlending();	
	ofSetColor( 0x999999 ); 
	img->draw( x, y );
	
	ofFill(); 
	ofSetColor( 255, 255, 255, overlay ); 
	ofRect( x + 0.5, y + 0.5, 18, 18 ); 
	
	ofNoFill(); 
	ofSetColor( 120, 120, 120 ); 
	ofRect( x + 0.5, y + 0.5, 18, 18 ); 
		
	ofDisableAlphaBlending();
	
	if( selected ){
		ofSetColor( 220, 220, 220 ); 
		ofRect( x + 0.5, y + 0.5, 18, 18 ); 
	}
}


// ------------------------
bool testApp::inRect( float pX, float pY, float x, float y, float width, float height ){
	return pX >= x && pX <= x + width && pY >= y && pY <= y + height; 
}

// ------------------------
bool testApp::inPoly(ofPoint *polygon,int N, ofPoint p ){
	int counter = 0;
	int i;
	double xinters;
	ofPoint p1,p2;
	
	p1 = polygon[0];
	for (i=1;i<=N;i++) {
		p2 = polygon[i % N];
		if (p.y > MIN(p1.y,p2.y)) {
			if (p.y <= MAX(p1.y,p2.y)) {
				if (p.x <= MAX(p1.x,p2.x)) {
					if (p1.y != p2.y) {
						xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
						if (p1.x == p2.x || p.x <= xinters)
							counter++;
					}
				}
			}
		}
		p1 = p2;
	}
	
	if (counter % 2 == 0)
		return false;
	else
		return true;
}