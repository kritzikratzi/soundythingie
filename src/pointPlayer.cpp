/*
 *  pointPlayer.cpp
 *  openFrameworks
 *
 *  Created by hansi on 12.12.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "pointPlayer.h"

int pointPlayer::idCount = 0;


pointPlayer::pointPlayer(){
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
	float attackLen = fmin( 50, this->pr->getDuration()/10 ); // max 50s or 10% attack time! 
	attackTime = 0.05; 
	releaseTime = this->pr->getDuration() - 0.05; 
}


void pointPlayer::update(){
	if( suicide ) return; 
	
	diffTime		= ofGetElapsedTimef() - timeOfLastFrame;
	timeOfLastFrame		= ofGetElapsedTimef(); 
	timeCounter			+= diffTime;
	samplesSinceUpdate = 0; 
	
	//cout << this->pr->bAmRecording << "--" << this->pr->pts.size() << "--" << timeCounter << "--" << this->pr->getDuration() << endl; 
	if (this->pr->bAmRecording == false && this->pr->pts.size() > 1){
		if( timeCounter >= this->pr->getDuration() ){
			// kill myself? 
			if( this->pr->beatMod == 0 && this->pr->startTime != 0 ){
				// reboot, beatless shit! 
				timeCounter = 0; 
				// get updated volume! 
				this->volume = this->pr->volume; 
			}
			else{
				// DIE BECAUSE OF AGE! 
				suicide = true; 
			}
		}
		else{
			ofPoint vel = this->pr->getVelocityForTime(timeCounter);
			ofPoint pt = this->pr->getPointForTime(timeCounter);
			
			float lengthOfVel = sqrt(vel.x * vel.x + vel.y * vel.y);
			if (lengthOfVel > 40) lengthOfVel = 40;
			amplitude = 0.94f * amplitude + 0.06f *( powf((lengthOfVel / 40.0f),2.0) * 0.5f);
			int width = ofGetWidth();
			pan = 0.95f * pan + 0.05f * ((float)pt.x / (float)width);
			float height = (float)ofGetHeight();
			float heightPct = ((height-pt.y) / height);
			targetFrequency = 1000.0f * heightPct;
			phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
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
	
	ofPoint vel = this->pr->getVelocityForTime(timeCounter);
	ofPoint pt = this->pr->getPointForTime(timeCounter);
	float height = (float)ofGetHeight();
	float heightPct = ((height-pt.y) / height);
	targetFrequency = 1000.0f * heightPct;
	cout << (int)targetFrequency << endl; 
	static float fx = 0;
	static float fy = 0; 
	fx += ( vel.x - fx ) / 50.0; 
	fy += ( vel.y - fy ) / 50.0; 
	ofLine( 100, 100, 100+fx*10, 100+fy*10 ); 
	ofRect( 100, 200, targetFrequency/10, 10 ); 
	ofRect( 100, 300, volume*100, 10 ); 
}


void pointPlayer::audioRequested(float * output, int bufferSize, int nChannels, bool useEnvelope ){
	if( suicide ) return; 
	
	float leftScale = 1 - pan;
	float rightScale = pan;
	
	
	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
	
	phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
	
	float envelopeScale = 1; 

	if( useEnvelope ){
		float actualTime = timeCounter + samplesSinceUpdate*bufferSize/44100.0;
		// buffer size is 256 (0.005sec), seems viable to calculate envelopescale only once! 
		if( actualTime < attackTime ){
			envelopeScale = fmin( 1, 1 - ( attackTime - actualTime ) / attackTime ); 
		}
		else if( actualTime > releaseTime ){
			envelopeScale = fmax( 0, 1 - ( actualTime - releaseTime) / ( this->pr->getDuration() - releaseTime ) );
		}
		else{
			envelopeScale = 1; 
		}
	}
	
	
	for (int i = 0; i < bufferSize; i++){
		// we run at 44100 samples/second. 
		// so one iteration equals 1/44100 seconds. 
		phase += phaseAdder;
		float sample = sin(phase);
		output[i*nChannels    ] += sample * amplitude * (volume*10) * leftScale * envelopeScale;
		output[i*nChannels + 1] += sample * amplitude * (volume*10) * volume * rightScale * envelopeScale;
	}
	
	this->samplesSinceUpdate ++; 
}