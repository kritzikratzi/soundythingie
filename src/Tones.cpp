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
	
	// we use the equation 
	// f(x) = a*ln( x + const ) + b 
	// and fixated f(0) = 4.13, 
	// and f(1) = 10 000
};

void Tones::draw(){
	float w = ofGetWidth(); 
	float y; 
	float h = ofGetHeight(); 
	ofSetLineWidth( 1 );
	ofSetColor( 45, 45, 45 ); 

	for( int i = 0; i < 181; i++ ){
		y = (int)( yValue(toneFrequencies[i]) );
		ofLine( 0, y, w, y ); 
	}	
};

float Tones::snap( float y ){
	float y1, y2, y3; 
	float h = ofGetHeight(); 
	for( int i = 2; i < 180; i++ ){
		y1 = (int) yValue(toneFrequencies[i-1]);
		y2 = (int) yValue(toneFrequencies[i]);
		y3 = (int) yValue(toneFrequencies[i+1]);
		
		cout << y1 << " :: " << y3 << endl; 
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
	static float temp; 
	temp = (ofGetHeight()-y)/ofGetHeight(); 
	return -1884.84*logf( 1.005-temp ) + 13.5307; 
}

/**
 * Inverse function of f 
 */
float Tones::yValue( float f ){
	return ofGetHeight()-ofGetHeight()* ( -expf( -0.00053055 * f ) * ( 1.0072 - 1.005*expf( 0.00053055 * f ) )); 
}
