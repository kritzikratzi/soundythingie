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