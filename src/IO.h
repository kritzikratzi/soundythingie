#ifndef __IO__
#define __IO__

#include "testApp.h"

class testApp; 

class IO{

	public: 
	static bool save( testApp * app, string filename ); 
	static bool load( testApp * app, string filename ); 
};

#endif 
