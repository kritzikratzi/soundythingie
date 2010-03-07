/*
 *  IO.cpp
 *  openFrameworks
 *
 *  Created by hansi on 07.03.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "IO.h"

bool IO::save( testApp * app, string filename ){
	
	ofstream out( filename.c_str(), ios::out );
	for( int i = 0; i < RECORDERS; i++ ){
		out << "RECORDER:" << i << endl; 
		app->recorders[i].save( out ); 
	}
	
	out.close();
	
	return false; 
}


bool IO::load( testApp * app, string filename ){
	ifstream in( filename.c_str(), ios::in ); 
	
	// Reset EVERYTHING! 
	for( int i = 0; i < RECORDERS; i++ ){
		app->recorders[i].reset( 0 ); 
	}
	
	char cmd[64]; 
	int iRec = 0; 
	while( !in.eof() ){
		Helpers::readCommand( cmd, in ); 
		if( strcmp( cmd, "RECORDER" ) == 0 ){
			app->recorders[iRec++].load( in ); 
		}
	}
	
	in.close(); 
}
