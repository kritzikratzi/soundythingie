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
}


void pointPlayer::update(){
	if( suicide ) return; 
	
	float diffTime		= ofGetElapsedTimef() - timeOfLastFrame;
	timeOfLastFrame		= ofGetElapsedTimef(); 
	timeCounter			+= diffTime;
	
	//cout << this->pr->bAmRecording << "--" << this->pr->pts.size() << "--" << timeCounter << "--" << this->pr->getDuration() << endl; 
	if (this->pr->bAmRecording == false && this->pr->pts.size() > 1){
		if( timeCounter >= this->pr->getDuration() ){
			// kill myself? 
			if( this->pr->beatMod == 0 && this->pr->startTime != 0 ){
				// reboot, beatless shit! 
				timeCounter = 0; 
			}
			else{
				suicide = true; 
				cout << id << ": DIE BECAUSE OF AGE! [" << timeCounter << "]" << endl; 
			}
		}
		else{
			ofPoint vel = this->pr->getVelocityForTime(timeCounter);
			ofPoint pt = this->pr->getPointForTime(timeCounter);
			
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
	}
	else{
		cout << id << ": DIE BECAUSE OF UNEMPLOYMENT!" << endl; 
		suicide = true; 
		volume = 0;
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


void pointPlayer::audioRequested(float * output, int bufferSize, int nChannels){
	if( suicide ) return; 
	
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
		output[i*nChannels    ] += sample * volume * leftScale;
		output[i*nChannels + 1] += sample * volume * rightScale;
	}	
}