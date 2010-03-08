#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){


	// macs by default run on non vertical sync, which can make animation very, very fast
	// this fixes that:

	ofSetVerticalSync( true );
	ofSetFrameRate( 60 );
	ofEnableSmoothing();
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
	hovering = NULL;
	recording = NULL;
	beatMod = 1;
	soundShape = 1;

	chromaticMode = false;
	selectionMode = false;
	holdSpawnMode = false;
	toConsole = false;
	holdSpawnMode = false;
	triggerAlwaysMode = false;


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
	triggerAlwaysImg.loadImage( "trigger_always.png" );
	triggerOnceImg.loadImage( "trigger_once.png" );

	bpmRates[0] =   0; bpmLastTriggered[0] = 0; bpmTriggerNow[0] = false;
	bpmRates[5] =  20; bpmLastTriggered[1] = 0; bpmTriggerNow[1] = false;
	bpmRates[4] =  30; bpmLastTriggered[2] = 0; bpmTriggerNow[2] = false;
	bpmRates[3] =  40; bpmLastTriggered[3] = 0; bpmTriggerNow[3] = false;
	bpmRates[2] =  60; bpmLastTriggered[4] = 0; bpmTriggerNow[4] = false;
	bpmRates[1] = 120; bpmLastTriggered[5] = 0; bpmTriggerNow[5] = false;

	for( int i = 0; i < RECORDERS; i++ ){
		recorders[i].index = i;
	}

}

//--------------------------------------------------------------
void testApp::update(){

	for( int i = 0; i < PLAYERS; i++ ){
		if( !players[i].suicide ){
			players[i].update();
		}
		else if( players[i].suicide && !players[i].dead ){
			players[i].dead = true;
			vector<pointPlayer*> * them = &playersOfRecorders[ players[i].pr->index ];
			for( int j = 0; j < them->size(); j++ ){
				if( them->at(j) == &players[i] ){
					them->erase( them->begin()+j );
					break;
				}
			}
		}
	}


	// which bpm rates should be triggered?
	for( int i = 1; i < 6; i++ ){
		if( bpmLastTriggered[i] + 60.0/bpmRates[i] <= ofGetElapsedTimef() ){
			// This is not perfect, but should work for now...
			bpmLastTriggered[i] += 60.0/bpmRates[i];
			bpmTriggerNow[i] = true;
			triggerAlpha[i] = 1;
		}
		else{
			bpmTriggerNow[i] = false;
			triggerAlpha[i] -= triggerAlpha[i]/(10+i*2.0);
		}
	}

	// trigger players!
	for( int i = 0; i < RECORDERS; i++ ){
		if( !recorders[i].bAmRecording && recorders[i].active() ){
			// Lines with beatMod 0 are only triggered if there is no
			// player already
			if( recorders[i].beatMod == 0 ){
				if( playersOfRecorders[i].size() == 0 ){
					pairUpWithAnyPlayer( &recorders[i] );
				}
			}
			// Lines with beatMod >0 are always triggered, except if they
			// are babysitters for other lines then they also are triggered
			// only if there is no other line.
			else if( recorders[i].beatMod > 0 ){
				if( bpmTriggerNow[ recorders[i].beatMod ] && ( recorders[i].triggerAlways == true || playersOfRecorders[i].size() == 0 ) ){
					pairUpWithAnyPlayer( &recorders[i] );
				}
			}
		}
	}

	// and then trigger kids!
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
						pairUpWithAnyPlayer( rec->kids[j] );
						k = PLAYERS; // "break" on the k-level
					}
				}
			}
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
				if( pr->kids[j]->pts.size() > 0 ){
					pt = &pr->pts[pr->kidPointNr[j]].pos;
					pt2 = &pr->kids[j]->pts[0].pos;
					ofNoFill();
					ofLine( pt->x, pt->y, pt2->x, pt2->y );
					ofFill();
					ofCircle( pt->x, pt->y, 2 );
				}
			}
		}
	}
	glDisable(GL_LINE_STIPPLE);

	// draw recorders themselves
	for( int i = 0; i < RECORDERS; i++ ){
		if( recorders[i].active() ){
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
			i == beatMod; // selected

		drawImage( &beatImgs[i], 10, 10+i*30, selected, alpha );

		// a little guide to show "where" in the beat we are
		/*if( i != 0 ){
			ofSetColor( 150, 150, 150 );
			int beat = 16<<i;
			float t = (ofGetFrameNum()%beat)/(float) beat;
			ofRect( 10.5 + t*18, 11.5 + i*30, 1, 0 );
		}*/

	}

	// Draw images for wave forms...
	for( int i = 0; i < 4; i++ ){
		bool selected = this->inRect( mouseX*1.0, mouseY*1.0, 10.0, 220.0+i*30.0, 20.0, 20.0 ) ||  // mouseover
		soundShape == i; // selected
		drawImage( &shapeImgs[i], 10, 220+i*30, selected );
	}

	bool triggerHovering = this->inRect( mouseX*1.0, mouseY*1.0, 10.0, 350.0, 20.0, 20.0 );
	drawImage( triggerAlwaysMode?&triggerAlwaysImg:&triggerOnceImg, 10.0, 350.0, triggerHovering );

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

		for( int i = 0; i < selectionPolyLength; i++ ){
			ofVertex( selectionPoly[i].x, selectionPoly[i].y );
		}

		ofEndShape( false );
		glDisable(GL_LINE_STIPPLE);
	}

	//string report = "nPts = " + ofToString(nPts) + "\ntotal time = " + ofToString(totalDuration, 3);
	//ofDrawBitmapString(report, 10, 10);
	for (vector<pointRecorder *>::iterator pr = selection.begin(); pr != selection.end(); ++pr ){
		ofFill();
		ofSetColor( 255, 0, 0 );
		float radius = 2+2*(*pr)->volume/0.1;
		ofCircle( (*pr)->pts[0].pos.x, (*pr)->pts[0].pos.y, radius );
	}

	if( hovering != NULL ){
		ofFill();
		ofSetColor( 255, 0, 0 );
		float radius = 2+2*hovering->volume/0.1;
		ofCircle( hovering->pts[0].pos.x, hovering->pts[0].pos.y, radius );
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
		if( players[i].dead ){
			playersOfRecorders[pr->index].push_back( &players[i] );
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
	if( key >= '0' && key <= '5' ){
		beatMod = key - '0';
	}

	if( key == ' ' ){
		toConsole = !toConsole;
	}

	if( key == 9 /* tab */ ){
		for( int i = 1; i < 6; i++ ){
			bpmLastTriggered[i] = ofGetElapsedTimef();
		}
	}

	if( key == 'h' ){
		holdSpawnMode = true;
		return;
	}

	if( key == 'c' ){
		for( int i = 0; i < RECORDERS; i++ ){
			//recorders[i].clear();
			recorders[i].startTime = 0;
			//players[i].suicide = true;
		}
	}


	if( key == '-' ){
		if( hovering != NULL ){
			hovering->volume -= 0.01;
			if( hovering->volume < 0 ) hovering->volume = 0;
		}

		for (vector<pointRecorder *>::iterator pr = selection.begin(); pr != selection.end(); ++pr ){
			(*pr)->volume -= 0.01;
			if( (*pr)->volume < 0 ) (*pr)->volume = 0;
		}
	}

	if( key == '+' ){
		if( hovering != NULL ){
			hovering->volume += 0.01;
			if( hovering->volume > 1 ) hovering->volume = 1;
		}

		for (vector<pointRecorder *>::iterator pr = selection.begin(); pr != selection.end(); ++pr ){
			(*pr)->volume += 0.01;
			if( (*pr)->volume > 1 ) (*pr)->volume = 1;
		}
	}

	if( key == 'd' || key == 127 ){
		// delete the one recorder being hovered, eventually...
		if( hovering != NULL ){
			deleteRecorder( hovering );
		}

		// delete all recorders in the selection
		for( int i = 0; i < selection.size(); i++ ){
			deleteRecorder( selection[i] );
		}

		// empty selection!
		selection.clear();
		cout << "CLEARED SEL REC: " << selection.size() << endl;
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
		selectionPolyLength = 0;
		if( selectionMode ){
			selection.clear();
		}
	}

	if( key == 'S' ){ // add to selection
		selectionMode = !selectionMode;
		selectionPolyLength = 0;
	}

	if( key == 'i' ){ // invert selection
		vector<pointRecorder * > temp;
		for( int i = 0; i < selection.size(); i++ ) temp.push_back( selection[i] );
		selection.clear();

		for( int i = 0; i < RECORDERS; i++ ){
			if( !binary_search( temp.begin(), temp.end(), &recorders[i] ) && recorders[i].active() ){
				selection.push_back( &recorders[i] );
			}
		}
	}

	if( key == 't' ){
		chromaticMode = true;
	}


	if( key >= OF_KEY_F1 && key <= OF_KEY_F12 ){
		int nr = key - OF_KEY_F1;

		if( (glutModifiers & GLUT_ACTIVE_SHIFT ) == 0 ){
			// enable/disable
			setEnabled[nr] = !setEnabled[nr];
			for (vector<pointRecorder *>::iterator pr = sets[nr].begin(); pr != sets[nr].end(); ++pr ){
				(*pr)->enabled = setEnabled[nr];
			}

			selection.clear();
			if( setEnabled[nr] ){
				for (vector<pointRecorder *>::iterator pr = sets[nr].begin(); pr != sets[nr].end(); ++pr ){
					selection.push_back( (*pr) );
				}
				sort( selection.begin(), selection.end() );
				unique( selection.begin(), selection.end() );
			}
		}
		else{
			sets[nr].clear();
			for (vector<pointRecorder *>::iterator pr = selection.begin(); pr != selection.end(); ++pr ){
				sets[nr].push_back( (*pr) );
			}

			selection.clear();
			setEnabled[nr] = true;
		}
	}

	if( key == 'x' ){
		ofPoint * pt;
		for( int i = 0; i < RECORDERS; i++ ){
			if( recorders[i].active() ){
				pt = &recorders[i].pts[0].pos;
				if( pt->x < 0 || pt->y < 0 || pt->x > ofGetHeight() || pt->y > ofGetHeight() ){
					deleteRecorder( &recorders[i] );
				}
			}
		}
	}


	if( key == '.' ){
		// IO::save( this, "test.txt" );
		save();
	}
	if( key == ',' ){
		// IO::save( this, "test.txt" );
		load();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
	glutModifiers = glutGetModifiers(); // can only be done in "core input callback"


	if( key == 'h' ){
		holdSpawnMode = false;
	}

	if( key == 't' ){
		chromaticMode = false;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	hovering = NULL;

	// are we really really close to a line?
	if( recording == NULL ){
		float dx, dy;
		for( int i = 0; i < RECORDERS; i++ ){
			if( recorders[i].active() && recorders[i].pts.size() > 0 ){
				// are we really close to the first point?
				float dx = mouseX - recorders[i].pts[0].pos.x;
				float dy = mouseY - recorders[i].pts[0].pos.y;

				if( sqrt( dx*dx + dy*dy ) < 5 ){
					hovering = &recorders[i];
					break;
				}
			}
		}
	}

	if( holdSpawnMode ){
		return;
	}

	spawnFocusPoint = -1;
	spawnFocusRecorder = -1;

	if( hovering == NULL && recording == NULL ){
		float minDistance = 100;
		float distance = 100;
		float dx, dy;


		for( int i = 0; i < RECORDERS; i++ ){
			if( recorders[i].active() && recorders[i].pts.size() > 0 ){
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

	// move, but only if shift is not pressed!
	if( hovering != NULL && ( glutModifiers & GLUT_ACTIVE_SHIFT ) == 0 ){
		bool moveKids = (glutModifiers & GLUT_ACTIVE_ALT) == 0;
		cout << glutModifiers << endl;
		cout << "move kids? " << moveKids << endl;
		float dx = -hovering->pts[0].pos.x + x;
		float dy = -hovering->pts[0].pos.y + y;
		moveRecorder( hovering, dx, dy, moveKids );

		for (vector<pointRecorder *>::iterator pr = selection.begin(); pr != selection.end(); ++pr ){
			if( *pr != hovering )
				moveRecorder( *pr, dx, dy, moveKids );
		}

		for( int i =0; i < RECORDERS; i++ ){
			if( recorders[i].active() )
				recorders[i].applyOffset();
		}
	}

	if( selectionMode ){
		if( selectionPolyLength < 1000 ){
			selectionPoly[selectionPolyLength].x = x;
			selectionPoly[selectionPolyLength].y = y;
			selectionPolyLength++;
		}

		return;
	}

	if( recording != NULL ){
		recording->addPoint( ofPoint(x,y,0) );
	}
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	glutModifiers = glutGetModifiers(); // can only be done in "core input callback"
	lastMousePressed = ofGetElapsedTimef();

	if( chromaticMode ){
		y = Tones::snap( y );
	}

	if( selectionMode ){
		selectionPolyLength = 0;
		selectionPoly[selectionPolyLength].x = x;
		selectionPoly[selectionPolyLength].y = y;
		selectionPolyLength++;
		return;
	}

	if( !selectionMode && selection.size() > 0 ){
		// Only clear recorders if no line is hovered, otherwise it's
		// perfectly fine to delete!
		if( hovering == NULL ){
			selection.clear();
		}
	}


	// handle the buttons...
	if( mouseX <= 30 ){
		// choosing beat
		for( int i = 0; i < 6; i++ ){
			if( this->inRect( mouseX*1.0, mouseY*1.0, 10, 10+i*30.0, 20.0, 20.0 ) ){
				beatMod = i;
				return;
			}
		}

		// choosing sound-shape
		for( int i = 0; i < 4; i++ ){
			if( this->inRect( mouseX*1.0, mouseY*1.0, 10, 220+i*30.0, 20.0, 20.0 ) ){
				soundShape = i;
				return;
			}
		}

		// change trigger-mode
		if( this->inRect( mouseX*1.0, mouseY*1.0, 10.0, 350.0, 20.0, 20.0 ) ){
			triggerAlwaysMode = !triggerAlwaysMode;
			return;
		}
	}


	if( hovering != NULL ){
		// usually we wanna abort here,
		// but if shift is pressed we generously
		// allow to continue for the parenting feature to work.
		if( ( glutModifiers & GLUT_ACTIVE_SHIFT ) == 0 ){
			return;
		}
	}

	// start recording!
	for( int i = 0; i < RECORDERS; i++ ){
		if( recorders[i].startTime == 0 ){
			recording = &recorders[i];
			recording->reset( this->beatMod );
			recording->soundShape = soundShape;
			recording->triggerAlways = triggerAlwaysMode;

			// we're someone's spawn?
			if( spawnFocusRecorder >= 0 ){
				pointRecorder * rec = &recorders[spawnFocusRecorder];
				rec->kids.push_back( recording );
				rec->kidPointNr.push_back( spawnFocusPoint );
				cout << "ADDED KID!!!!" << endl;
				ofPoint p = rec->pts[spawnFocusPoint].pos;
				recording->addPoint( p );
				recording->beatMod = -1; // this will never launch it's own players!
			}
			// parenting someone?
			else if( hovering ){
				recording->babysitting.push_back( hovering );
				recording->triggerAlways = false;
				hovering->babysitter = recording;
			}

			// no one's spawn (parenting or normal)
			// and on a classic beat
			if( spawnFocusRecorder < 0 && beatMod > 0 ){
				recording->startDelay = ofGetElapsedTimef() - bpmLastTriggered[beatMod];
				cout << recording->startDelay << endl;
			}

			return;
		}
	}


	// NONE???
	recording = NULL;
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
			if( pr->active() && pr->pts.size() > 0 && inPoly( selectionPoly, selectionPolyLength, pr->pts[0].pos ) ){
				selection.push_back( pr );
			}
		}

		// sort and shrink!
		sort( selection.begin(), selection.end() );
		unique( selection.begin(), selection.end() );

		selectionMode = false;

		return;
	}

	if( hovering != NULL && ofGetElapsedTimef() - lastMousePressed < 0.20 ){
		deleteRecorder( hovering );
		hovering = NULL;
	}



	if( recording != NULL ){
		if( recording->pts.size() >= 1 ){
			recording->addPoint( ofPoint(mouseX, mouseY,0) );
			recording->bAmRecording = false;
		}
		else{
			recording->reset( this->beatMod );

			// if this was anyone's babysitter then remove it!
			for( int i = 0; i < RECORDERS; i++ ){
				if( recorders[i].babysitter = recording ){
					recorders[i].babysitter = NULL;
				}
			}
		}

		timeCounter = 0;
		recording = NULL;
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
			if( players[i].pr->babysitting.size() > 0 ){
				vector<pointRecorder*> * vec = &players[i].pr->babysitting;
				ofPoint * targetPos = &players[i].currentPoint;
				for (vector<pointRecorder *>::iterator pr = vec->begin(); pr != vec->end(); ++pr ){
					this->moveRecorder( *pr, targetPos->x-(*pr)->pts[0].pos.x, targetPos->y-(*pr)->pts[0].pos.y, true );
				}

				for( int i =0; i < RECORDERS; i++ ){
					if( recorders[i].active() )
						recorders[i].applyOffset();
				}
			}
			else{
				players[i].audioRequested( output, bufferSize, nChannels, useEnvelope );
			}
		}
	}

	for( int i = 0; i < 256; i++ ){
		lAudio[i] = fmin( +2, fmax( -2, output[i*nChannels] ) );
		rAudio[i] = fmin( +2, fmax( -2, output[i*nChannels+1] ) );
	}

	if( toConsole ){
		cout << "-----------------------" << endl;
		for( int i = 0; i < 256; i++ ){
			cout << output[i*nChannels] << ":" << output[i*nChannels+1] << ", ";
		}
		cout << endl;
	}

}


// ------------------------
void testApp::deleteRecorder( pointRecorder * rec ){
	rec->startTime = 0;
	pointRecorder * pr;

	for( int i = 0; i < RECORDERS; i++ ){
		pr = &recorders[i];
		if( pr == rec ){
			// delete all the recorders (if there are any)
			// that were triggered by this recorder
			deleteRecordersKids( pr );
		}
		else{
			// remove the recorder from the other recorder it was triggered by
			// this doesn't have to be the case, it might be...
			for( int j = 0; j < pr->kids.size(); j++ ){
				if( pr->kids[j] == rec ){
					pr->kids.erase( pr->kids.begin()+j );
					pr->kidPointNr.erase( pr->kidPointNr.begin()+j );
					j--;
				}
			}
		}
	}

	hovering = NULL;
	return;
}

// ------------------------
void testApp::deleteRecordersKids( pointRecorder * rec ){
	for( int j = 0; j < rec->kids.size(); j++ ){
		rec->kids[j]->startTime = 0;
		deleteRecordersKids( rec->kids[j] );
	}
}

// ------------------------
void testApp::moveRecorder( pointRecorder * rec, int dx, int dy, bool moveKids ){
	rec->offsetX = dx;
	rec->offsetY = dy;

	if( moveKids ){
		for( int i = 0; i < rec->kids.size(); i++ ){
			moveRecorder( rec->kids[i], dx, dy, true );
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


// ------------------------
void testApp::save(){
	#ifdef __APPLE__ // Apple code only!
	short fRefNumOut;
	FSRef output_file;
	OSStatus err;

	NavDialogCreationOptions options;
	NavGetDefaultDialogCreationOptions( &options );
	options.modality = kWindowModalityAppModal;
	//CFPropertyListRef propRef = CFPreferencesCopyAppValue( CFSTR("currentDirectory"), CFSTR("org.sd.soundythingie") );
	//options.saveFileName = (CFStringRef) propRef;
	//cout << "STR: " << CFStringGetCStringPtr(options.saveFileName, kCFStringEncodingUnicode) << endl;

	NavDialogRef dialog;
	err = NavCreatePutFileDialog(&options, '?', '?', NULL, NULL, &dialog);
	err = NavDialogRun(dialog);

	NavUserAction action;
	action = NavDialogGetUserAction( dialog );
	if (action == kNavUserActionNone || action == kNavUserActionCancel) {
		cout << "user canceled" << endl;
		return;
		// return 2;
	}

	// get dialog reply
	NavReplyRecord reply;
	err = NavDialogGetReply(dialog, &reply);
	if ( err != noErr )
		return; // return 2;

	if ( reply.replacing )
	{
		printf("need to replace\n");
	}

	AEKeyword keyword;
	DescType actual_type;
	Size actual_size;
	FSRef output_dir;
	err = AEGetNthPtr(&(reply.selection), 1, typeFSRef, &keyword, &actual_type,
					  &output_dir, sizeof(output_file), &actual_size);

	//printf("AEGetNthPtr returned %i\n", err );


	CFURLRef cfUrl = CFURLCreateFromFSRef( kCFAllocatorDefault, &output_dir );
	CFStringRef cfString = NULL;
	if ( cfUrl != NULL )
	{
		cfString = CFURLCopyFileSystemPath( cfUrl, kCFURLPOSIXPathStyle );
		CFRelease( cfUrl );
	}

	// copy from a CFString into a local c string (http://www.carbondev.com/site/?page=CStrings+)
	const int kBufferSize = 255;

	char folderURL[kBufferSize];
	Boolean bool1 = CFStringGetCString(cfString,folderURL,kBufferSize,kCFStringEncodingMacRoman);

	char fileName[kBufferSize];
	Boolean bool2 = CFStringGetCString(reply.saveFileName,fileName,kBufferSize,kCFStringEncodingMacRoman);

	// append strings together

	string url1 = folderURL;
	string url2 = fileName;
	string finalURL = url1 + "/" + url2;

	printf("url %s\n", finalURL.c_str());

	// cleanup dialog
	NavDialogDispose(dialog);
	//CFStringRef ref = CFStringCreateWithCString(NULL, finalURL.c_str(), kCFStringEncodingUnicode );
	//cout << ref << endl;
	//CFPreferencesSetAppValue(CFSTR("currentDirectory"), ref, CFSTR("org.sd.soundythingie") );
	//CFPreferencesAppSynchronize( CFSTR("org.sd.soundythingie") );

	save( finalURL );
	#endif

	#ifdef __WIN32__
	char szFileName[MAX_PATH] = "";

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = 0;

	if(GetSaveFileName(&ofn)) {
	   save(string(szFileName));
	}
	#endif
}


// ------------------------
void testApp::load(){
	#ifdef __APPLE__ // Apple only!
	short fRefNumOut;
	FSRef output_file;
	OSStatus err;
	NavReplyRecord replyRecord;

	NavDialogCreationOptions options;
	NavGetDefaultDialogCreationOptions( &options );
	options.modality = kWindowModalityAppModal;
	//CFPropertyListRef propRef = CFPreferencesCopyAppValue( CFSTR("currentDirectory"), CFSTR("org.sd.soundythingie") );
	//options.saveFileName = (CFStringRef) propRef;
	//cout << "STR: " << CFStringGetCStringPtr(options.saveFileName, kCFStringEncodingUnicode) << endl;

	NavDialogRef dialog;

	err = NavCreateGetFileDialog(&options, NULL, NULL, NULL, NULL, NULL, &dialog);
	err = NavDialogRun(dialog);

	NavUserAction action;
	action = NavDialogGetUserAction( dialog );
	if (action == kNavUserActionNone || action == kNavUserActionCancel) {
		cout << "user canceled" << endl;
		return;
		// return 2;
	}



	// Get the reply
	err = NavDialogGetReply(dialog, &replyRecord);
	// If the user clicked "Cancel", just bail
	if ( err == userCanceledErr ){
		return;
	}

	// Get the file
	err = AEGetNthPtr(&(replyRecord.selection), 1, typeFSRef, NULL, NULL, &output_file, sizeof(FSRef), NULL);

	// Convert it to a CFURL
	CFURLRef cfUrl = CFURLCreateFromFSRef(NULL, &output_file);


	CFStringRef cfString = NULL;
	if ( cfUrl != NULL )
	{
		cfString = CFURLCopyFileSystemPath( cfUrl, kCFURLPOSIXPathStyle );
		CFRelease( cfUrl );
	}

	// copy from a CFString into a local c string (http://www.carbondev.com/site/?page=CStrings+)
	const int kBufferSize = 512;

	char folderURL[kBufferSize];
	Boolean bool1 = CFStringGetCString(cfString,folderURL,kBufferSize,kCFStringEncodingMacRoman);


	save( string( folderURL ) );

	#endif


	#ifdef __WIN32__
	char szFileName[MAX_PATH] = "";

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = 0;

	if(GetOpenFileName(&ofn)) {
	   load(string(szFileName));
	}
	#endif
}


// ------------------------
bool testApp::save( string filename ){

	ofstream out( filename.c_str(), ios::out );
	for( int i = 0; i < RECORDERS; i++ ){
		out << "recorder:" << i << endl;
		recorders[i].save( out );
	}

	for( int i = 0; i < 12; i++ ){
		out << "set:";
		for( int j = 0; j < sets[i].size(); j++ ){
			out << sets[i][j]->index << " ";
		}
		out << "-1" << endl;
		out << "setEnabled:" << setEnabled[i] << endl;
	}

	out.close();

	return false;
}


// ------------------------
bool testApp::load( string filename ){
	ifstream in( filename.c_str(), ios::in );

	// Reset EVERYTHING!
	for( int i = 0; i < RECORDERS; i++ ){
		recorders[i].reset( 0 );
	}
	for( int i = 0; i < 12; i++ ){
		sets[i].clear();
	}

	char cmd[64];

	int iRec = 0;
	int iSet = 0;
	int iSetEnabled = 0;
	while( !in.eof() ){
		Helpers::readCommand( cmd, in );
		if( 0 == strcmp( cmd, "recorder" ) ){
			recorders[iRec++].load( in, recorders, players );
		}
		if( 0 == strcmp( cmd, "set" ) ){
			int i = 0; in >> i;
			while( i != -1 && !in.eof() ){
				sets[iSet].push_back( &recorders[i] );
				in >> i;
			}
			iSet ++;
		}
		if( 0 == strcmp( cmd, "setEnabled" ) ){
			in >> setEnabled[iSetEnabled++];
		}
	}

	// great, now apply current set status!
	for( int i = 0; i < 12; i++ ){
		for (vector<pointRecorder *>::iterator pr = sets[i].begin(); pr != sets[i].end(); ++pr ){
			(*pr)->enabled = setEnabled[i];
		}
	}
	in.close();
}
