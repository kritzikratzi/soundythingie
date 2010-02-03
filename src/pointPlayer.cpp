/*
 *  pointPlayer.cpp
 *  openFrameworks
 *
 *  Created by hansi on 12.12.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "pointPlayer.h"
#include "ofMain.h"

int pointPlayer::idCount = 0;


pointPlayer::pointPlayer(){
	pr = NULL; 
	id = idCount; 
	idCount++; 
	
	suicide = true; 
}

void pointPlayer::setup( pointRecorder * pr ){
	this->pr = pr; 
	//volume				= 0.1f;
	volume = pr->volume; 
	pan					= 0.5f;
	sampleRate 			= 44100;
	timeCounter			= 0; 
	suicide				= false; 
	timeOfLastFrame		= ofGetElapsedTimef(); 
	
	// calculate attack and release time (50ms)
	attackTime = fmin( 0.1, this->pr->getDuration()/10 ); // max 0.1s or 10% attack time! 
	releaseTime = this->pr->getDuration() - attackTime; 
	doCrazyMath( true ); 
	envelopeScale = 0; 
}


void pointPlayer::update(){
	if( suicide ) return; 
	
	diffTime		= ofGetElapsedTimef() - timeOfLastFrame;
	timeOfLastFrame		= ofGetElapsedTimef(); 
	timeCounter			+= diffTime;
	samplesSinceUpdate = 0; 
	
	//cout << this->pr->bAmRecording << "--" << this->pr->pts.size() << "--" << timeCounter << "--" << this->pr->getDuration() << endl; 
	if ( this->pr != NULL && this->pr->bAmRecording == false && this->pr->pts.size() > 1){
		if( timeCounter >= this->pr->getDuration() ){
			// kill myself? 
			if( this->pr->beatMod == 0 && this->pr->startTime != 0 ){
				// reboot, beatless shit! 
				timeCounter = 0; 
				doCrazyMath( true ); 
				// get updated volume! 
				this->volume = this->pr->volume; 
			}
			else{
				// DIE BECAUSE OF AGE! 
				suicide = true; 
			}
		}
		else{
			doCrazyMath( false ); 
		}
	}
	else{
		// DIE BECAUSE OF UNEMPLOYMENT!
		suicide = true; 
		volume = 0;
		amplitude = 0; 
		targetFrequency = 100;
		phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
		timeCounter = 0; 
	}
}

void pointPlayer::draw(){
	if( suicide ) return; 
	
	ofSetColor(255,255,255);
	ofFill();
	ofSetRectMode(OF_RECTMODE_CENTER);
	// if we are not recording, get the point for a given time: 
	if (pr->bAmRecording == false && pr->pts.size() > 1){
		
		ofPoint vel = pr->getVelocityForTime(timeCounter);
		ofPoint pt = pr->getPointForTime(timeCounter);
		
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


void pointPlayer::audioRequested(float * output, int bufferSize, int nChannels, bool useEnvelope ){
	// note: buffer size is 256 (0.005sec)
	if( suicide ) return; 
	
	float leftScale, rightScale; 
	
	
	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	/*while (phase > TWO_PI){
		phase -= TWO_PI;
	}*/
	
	float envelopeScaleTarget = 1; 
	//amplitude = 0.1; 
	
	
	float (*shapeFunc)(float);
	if( pr->soundShape == 0 ) shapeFunc = &shapeFlat; 
	if( pr->soundShape == 1 ) shapeFunc = &shapeSinus; 
	if( pr->soundShape == 2 ) shapeFunc = &shapeSawtooth;
	if( pr->soundShape == 3 ) shapeFunc = &shapeRectangle;
	
	//pan = 0.95f * pan + 0.05f * panTarget;
	int cycle = 0; 
	for (int i = 0; i < bufferSize; i++){
		     if( fabs( pan-panTarget) < 0.0001 ) ; //nothing happens! 
		else if( pan < panTarget ) pan += 0.0001; 
		else if( pan > panTarget ) pan -= 0.0001; 
		
		/*     if( fabs( phaseAdder - phaseAdderTarget ) < 0.00001 ) ; 
		else if( phaseAdder < phaseAdderTarget ) phaseAdder += 0.00001; 
		else if( phaseAdder > phaseAdderTarget ) phaseAdder -= 0.00001; */
		phaseAdder = phaseAdderTarget*0.001f + phaseAdder*0.999f; 
		
		leftScale = 1-pan; 
		rightScale = pan; 
		
		//phaseAdder = 0.99f * phaseAdder + 0.01f * phaseAdderTarget;
		if( useEnvelope ){
			float actualTime = timeCounter + samplesSinceUpdate*bufferSize/44100.0 + i/44100.0;
			
			if( actualTime < attackTime ){
				cycle = 1; 
				envelopeScaleTarget = fmin( 1, 1 - ( attackTime - actualTime ) / attackTime ); 
			}
			else if( actualTime > releaseTime ){
				cycle = 2; 
				envelopeScaleTarget = fmax( 0, 1 - ( actualTime - releaseTime) / ( this->pr->getDuration() - releaseTime ) );
			}
			else{
				envelopeScaleTarget = 1; 
			}
		}
		
		phase += phaseAdder;
		phase = fmod( phase, TWO_PI ); 

		/*if( fabs( amplitude - amplitudeTarget ) < 0.00001 ) ; //nothing happens! 
		else if( amplitude < amplitudeTarget ) amplitude += 0.00001; 
		else if( amplitude > amplitudeTarget ) amplitude -= 0.00001; */
		amplitude = amplitudeTarget*0.001f + amplitude*0.999f; 
		
		if( fabs( envelopeScale - envelopeScaleTarget) < 0.001 ) ; //nothing happens! 
		else if( envelopeScale < envelopeScaleTarget ) envelopeScale += 0.001; 
		else if( envelopeScale > envelopeScaleTarget ) envelopeScale -= 0.001; 
		
		
		float sample = shapeFunc(phase);
		output[i*nChannels    ] += sample * amplitude * (volume*10) * leftScale  * envelopeScale;
		output[i*nChannels + 1] += sample * amplitude * (volume*10) * rightScale * envelopeScale;
	}
	
	
	//cout << env << "," << (cycle==0?"F":cycle==1?"A":"R") << endl; 
	
	this->samplesSinceUpdate ++; 
}

/**
 * Recalculate some target-values, 
 * use the apply parameter to instantly 
 * set the values to their corresponding target values. 
 */
void pointPlayer::doCrazyMath( bool apply ){
	ofPoint vel = this->pr->getVelocityForTime(timeCounter);
	ofPoint pt = this->pr->getPointForTime(timeCounter);
	currentVelocity = vel; 
	currentPoint = pt; 
	
	
	float lengthOfVel = sqrt(vel.x * vel.x + vel.y * vel.y);
	if (lengthOfVel > 40) lengthOfVel = 40;
	//amplitude = 0.94f * amplitude + 0.06f *( powf((lengthOfVel / 40.0f),2.0) * 0.5f);
	amplitudeTarget = powf((lengthOfVel / 40.0f),2.0) * 0.5f; 
	panTarget = fmin( 1, fmax( 0, pt.x/(float)ofGetWidth() ) ); 
	float height = (float)ofGetHeight();
	//float heightPct = ((height-pt.y) / height);
	//targetFrequency = 5000.0f * heightPct;
	targetFrequency = Tones::fValue( pt.y ); 
	phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	
	
	if( apply ){
		phaseAdder = phaseAdderTarget; 
		pan = panTarget; 
	}
}

float shapeFlat( float t ){ return 0; }
float shapeSinus( float t ){ return sin( t ); }
float shapeSawtooth( float t ){ return 1-2*t/TWO_PI; }
float shapeRectangle( float t ){ return t < PI? 1 : -1; }