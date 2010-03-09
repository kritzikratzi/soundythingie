/*
 *  Button.cpp
 *  osx10.4
 *
 *  Created by hansi on 09.03.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ImageButton.h"


void ImageButton::init( int x, int y, const char * filename ){
	this->x = x; 
	this->y = y; 
	image = new ofImage; 
	image->loadImage( string( filename ) ); 
	w = image->getWidth(); 
	h = image->getHeight(); 
	visible = 1; 
	enabled = 1; 
	forceAlpha = -1; 
}

void ImageButton::update( float mouseX, float mouseY ){
	hover = visible && enabled && Helpers::inRect( mouseX, mouseY, x, y, w, h );
	
	if( forceAlpha != -1 ) alphaTarget = forceAlpha; 
	else if( !visible ) alphaTarget = 0; 
	else if( !hover ) alphaTarget = enabled?0.5:0.3;
	else alphaTarget = enabled?1.0:0.5; 
	
	
	alphaIs += ( alphaTarget - alphaIs )/10;
	overlayAlphaIs -= overlayAlphaIs/17; 
}

void ImageButton::draw(){
	
	ofEnableAlphaBlending();
	ofSetColor( 0xCC*alphaIs, 0xCC*alphaIs, 0xCC*alphaIs ); 
	image->draw( x, y );
	
	ofFill();
	ofSetColor( 255, 255, 255, overlayAlphaIs*alphaIs*255 );
	ofRect( x + 0.5, y + 0.5, w, h );
	
	ofNoFill();
	ofSetColor( (((hover||selected)?0xFFFFFF:0x787878)&0x00FFFFFF) | (((int)alphaIs*255)<<24) ); 
	ofRect( x + 0.5, y + 0.5, w, h );
	
	ofDisableAlphaBlending();
}

void ImageButton::flash(){
	overlayAlphaIs = 1; 
}


void ImageButton::activated(){
	alphaIs = 1; 
}