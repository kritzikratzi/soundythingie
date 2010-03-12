/*
 *  Tones.cpp
 *  openFrameworks
 *
 *  Created by hansi on 05.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Tones.h"

float Tones::pHeight = 0; 
float Tones::yValues[NUM_TONES]; 
//float Tones::*fValues = NULL; 

void Tones::checkInit(){
	if( pHeight != ofGetHeight() || fValues == NULL ){
		pHeight = ofGetHeight(); 
		if( fValues != NULL ) delete [] fValues; 
		fValues = new float[(int)pHeight+1];
		
		// this code is as primitive as possible! 
		int nrSemiTones = 0; 
		for( int i = 0; i < NUM_TONES; i++ ){
			int nr = i%12; 
			bool semiTone = nr==1 || nr==3 || nr==6 || nr==8 || nr==10; 
			//  || (nr == 6) || (nr == 8) || (nr == 10);
			if( semiTone ) nrSemiTones ++; 
		}
		
		int nrWholeTones = NUM_TONES - nrSemiTones; 
		int nrWholeTone = 0; 
		for( int i = 0; i < NUM_TONES; i++ ){
			// yValues[i] = yValue( toneFrequencies[i] ); 
			int nr = i%12; 
			bool semiTone = nr==1 || nr==3 || nr==6 || nr==8 || nr==10; 
			if( semiTone ){
				yValues[i] = ofGetHeight() - ofGetHeight()*(nrWholeTone-0.5)/(nrWholeTones-1); 
			}
			else{
				yValues[i] = ofGetHeight() - ofGetHeight()*nrWholeTone/(nrWholeTones-1); 
				nrWholeTone ++; 
			}
			
			// cout << "y(" << i << ")=" << yValues[i] << endl; 
		}
		
		int currentTone = 0; 
		float percent; 
		float y; 
		for( int i = pHeight; i >= 0; i-- ){
			// have we reached the next note already? 
			if( yValues[currentTone+1] - i >= 0 ){
				// can we step up to the next tone? 
				if( currentTone+2 < NUM_TONES )
					currentTone ++; 
			}
			   
			percent = (i-yValues[currentTone]) / (yValues[currentTone+1]-yValues[currentTone]);
			fValues[i] = toneFrequencies[currentTone] + percent*(toneFrequencies[currentTone+1]-toneFrequencies[currentTone]);
			// cout << i << "/" << currentTone << ":" << fValues[i] << endl; 
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
		//y = (int)( yValue(toneFrequencies[i]) );
		ofLine( 0, (int)yValues[i], w, (int)yValues[i] ); 
	}
};

float Tones::snap( float y ){
	float y1, y2, y3; 
	float result = y; 
	for( int i = 1; i < NUM_TONES-1; i++ ){
		/*y1 = (int) yValue(toneFrequencies[i-1]);
		y2 = (int) yValue(toneFrequencies[i]);
		y3 = (int) yValue(toneFrequencies[i+1]);*/
		y1=yValues[i-1];
		y2=yValues[i-0];
		y3=yValues[i+1];
		
		y1 = (y1+y2)/2; 
		y3 = (y2+y3)/2; 
		
		if( y <= y1 && y >= y3 ){
			result = y2; 
			break; 
		}
	}
	
	if( result < 0 ) result = 0; 
	if( result > ofGetHeight() ){
		result = ofGetHeight(); 
	}
	
	return result; 
}

/**
 * Scaling function for the tones... 
 * y: 0...height
 * Output will be 4.13....10000
 */
float Tones::fValue( float y ){
	y = fmin( fmax( 0, y ), pHeight ); 
	int a = (int)floorf( y );
	int b = (int)ceilf( y );
	if( a == b ){
		if( a == 0 ) b = 1; 
		else a = b - 1; 
	}
	float percent = (y-a)/(b-a);
	// cout << "RESULT=" << y << "==" << a << "," << b << "--" << percent <<"/" << (fValues[a] + percent*(fValues[b]-fValues[a])) << endl; 
	return fValues[a] + percent*(fValues[b]-fValues[a]);
	/*static float temp; 
	temp = (pHeight-y)/pHeight; 
	return -1884.84*logf( 1.005-temp ) + 13.5307; */
	
}

/**
 * Inverse function of f 
 */
float Tones::yValue( float f ){
	cout << "OBSOLETE-WHO USES ME ANYMORE?" << endl; 
	f = fmin( fmax( 4.13, f ), 10000 ); 
	return pHeight-pHeight* ( -expf( -0.00053055 * f ) * ( 1.0072 - 1.005*expf( 0.00053055 * f ) )); 
}
