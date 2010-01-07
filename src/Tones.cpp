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

float Tones::pHeight = 0; 
float Tones::yValues[135]; 

void Tones::checkInit(){
	if( pHeight != ofGetHeight() ){
		pHeight = ofGetHeight(); 
		for( int i = 0; i < NUM_TONES; i++ ){
			yValues[i] = yValue( toneFrequencies[i] ); 
		}
	}
};

void Tones::draw(){
	
	float w = ofGetWidth(); 
	float y; 
	float h = ofGetHeight(); 
	ofSetLineWidth( 1 );
	ofSetColor( 45, 45, 45 ); 

	for( int i = 0; i < NUM_TONES; i++ ){
		y = (int)( yValue(toneFrequencies[i]) );
		ofLine( 0, y, w, y ); 
	}
};

float Tones::snap( float y ){
	float y1, y2, y3; 
	for( int i = 2; i < NUM_TONES-1; i++ ){
		y1 = (int) yValue(toneFrequencies[i-1]);
		y2 = (int) yValue(toneFrequencies[i]);
		y3 = (int) yValue(toneFrequencies[i+1]);
		
		y1 = (y1+y2)/2; 
		y3 = (y2+y3)/2; 
		
		if( y <= y1 && y >= y3 ){
			return y2; 
		}
	}
}

/**
 * Scaling function for the tones... 
 * y: 0...height
 * Output will be 4.13....10000
 */
float Tones::fValue( float y ){
	y = fmin( fmax( 0, y ), pHeight ); 
	static float temp; 
	temp = (pHeight-y)/pHeight; 
	return -1884.84*logf( 1.005-temp ) + 13.5307; 
}

/**
 * Inverse function of f 
 */
float Tones::yValue( float f ){
	f = fmin( fmax( 4.13, f ), 10000 ); 
	return pHeight-pHeight* ( -expf( -0.00053055 * f ) * ( 1.0072 - 1.005*expf( 0.00053055 * f ) )); 
}
