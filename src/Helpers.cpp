/*
 *  Helpers.cpp
 *  openFrameworks
 *
 *  Created by hansi on 07.03.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Helpers.h"

/** 
 * Will read a maximum of 64 characters! 
 */
void Helpers::readCommand( char * cmd, ifstream& in ){
	int i = 0;
	char value; 

	in.get( value ); 
	
	while( value != ':' && !in.eof() ){
		if( i >= 63 ){
			// do nothing anymore! 
		}
		else if( ( value >= 'A' && value <= 'Z' ) || ( value >= 'a' && value <= 'z' ) ){
			cmd[i] = value; 
			i++; 
		}
		
		in.get( value ); 
	}
	
	// terminator! 
	cmd[i] = '\0';
}



/**
 * Check if a point is inside a rectangle
 */
bool Helpers::inRect( float pX, float pY, float x, float y, float width, float height ){
	return pX >= x && pX <= x + width && pY >= y && pY <= y + height;
}


/**
 * Check if a point is inside a polygon
 */
bool Helpers::inPoly(ofPoint *polygon,int N, ofPoint p ){
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


