#ifndef __HELP_ARS
#define __HELP_ARS

#include <fstream>
#include <iostream>
#include "ofMain.h"
using namespace std; 

class Helpers{
public:
	static void readCommand( char * cmd, ifstream& input ); 
	static bool inRect( float pX, float pY, float x, float y, float width, float height );
	static bool inPoly( ofPoint *polygon, int N, ofPoint p );
};


#endif
