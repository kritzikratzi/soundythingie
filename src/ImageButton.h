/*
 *  Button.h
 *  osx10.4
 *
 *  Created by hansi on 09.03.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofMain.h"
#include "Helpers.h"

#ifndef _B_U_TT_O_N_
#define _B_U_TT_O_N_

class ImageButton{
public: 
	ofImage * image;
	
	bool hover; 
	
	bool selected; 
	bool visible; 
	bool enabled; 
	float forceAlpha; 
	
	// these below shouldn't be modified from outside! 
	float alphaIs; 
	float alphaTarget; 
	
	float overlayAlphaIs;
	float overlayAlphaTarget;
	
	float x, y, w, h; 

	void init( int x, int y, const char * filename ); 
	void update( float mouseX, float mouseY ); 
	void draw(); 
	void flash(); 
	void activated();	
	
};

#endif
