/*
 *  Tones.cpp
 *  openFrameworks
 *
 *  Created by hansi on 05.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Tones.h"
#include "ofMain.h"

void Tones::init(){
	//toneYValues[181];
	
};

void Tones::draw(){
	float w = ofGetWidth(); 
	float y; 
	float h = ofGetHeight(); 
	ofSetLineWidth( 1 );
	ofSetColor( 45, 45, 45 ); 

	for( int i = 0; i < 181; i++ ){
		//freq = 1000*(ofGetHeight()-y)/ofGetHeight(); 
		y = (int)( h-toneFrequencies[i]*h/5000.0f );
		ofLine( 0, y, w, y ); 
	}	
};

float Tones::snap( float y ){
	float y1, y2, y3; 
	float h = ofGetHeight(); 
	for( int i = 2; i < 180; i++ ){
		y1 = (int)( h-toneFrequencies[i-1]*h/5000.0f );
		y2 = (int)( h-toneFrequencies[i]*h/5000.0f );
		y3 = (int)( h-toneFrequencies[i+1]*h/5000.0f );
		
		cout << y1 << " :: " << y3 << endl; 
		y1 = (y1+y2)/2; 
		y3 = (y2+y3)/2; 
		
		if( y <= y1 && y >= y3 ){
			return y2; 
		}
		
	}
}