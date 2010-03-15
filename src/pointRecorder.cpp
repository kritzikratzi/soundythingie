#include "pointRecorder.h"


//------------------------------------------------------------------
pointRecorder::pointRecorder(){
	maxNumPts = 5000;  // try 50, for example....
	startTime = 0;
	offsetX = 0; 
	offsetY = 0; 
}

//------------------------------------------------------------------
void pointRecorder::draw() {
	
	//---------------------------
	// (a) create a small report 
	ofSetColor(200,200,200);
	// get the number of points and the time of the last point:
	int nPts = pts.size();
	float totalDuration = getDuration();
	//string report = "nPts = " + ofToString(nPts) + "\ntotal time = " + ofToString(totalDuration, 3);
	//ofDrawBitmapString(report, 10, 10);
	
	//---------------------------
	// (b) draw the line: 
	if (bAmRecording)		ofSetColor(255,255,0);
	else					ofSetColor(80,80,80);
	
	if( babysitting.size() > 0 ){
		glEnable(GL_LINE_STIPPLE);
		glLineStipple( 1, 0x2222 ); 
	}		
	ofPushMatrix();
	ofTranslate( offsetX, offsetY, 0 ); 
	ofNoFill();
	ofBeginShape();
	for (int i = 0; i < pts.size(); i++){
		ofVertex(pts[i].pos.x, pts[i].pos.y);
	}
	
	ofEndShape(false);
	glDisable(GL_LINE_STIPPLE);
	
	if( pts.size() > 1 ){
		ofFill(); 
		float radius = 2+2*volume/0.1;
		ofCircle( pts[0].pos.x, pts[0].pos.y, radius ); 
	}
	ofPopMatrix(); 
	
	
}

//------------------------------------------------------------------
void pointRecorder::addPoint(ofPoint pt) {
	
	
	// on the first point, grab the start time
	if (pts.size() == 0){
		//startTime = ofGetSystemTime();
	} 
	
	
	// combine the position and the time here: 
	timePt	myPoint;
	myPoint.pos			= pt;
	myPoint.time		= (ofGetSystemTime() - startTime)/1000.0;
	
	pts.push_back(myPoint);
	if (pts.size() > maxNumPts){
		pts.erase(pts.begin());
	}
}

//------------------------------------------------------------------
void pointRecorder::reset( int beatMod ) {
	this->beatMod = beatMod; 

	offsetX = 0; 
	offsetY = 0; 
	bAmRecording = true;
	volume = 0.1f;
	startDelay = 0; 
	startTime = 0; 
	enabled = true; 
	
	pts.clear();
	kids.clear(); 
	kidPointNr.clear(); 
	
	babysitter = NULL; 
	babysitting.clear(); 
}



//------------------------------------------------------------------
float pointRecorder::getDuration() {
	float totalDuration = 0;
	if (pts.size() > 0){
		totalDuration = pts[pts.size() - 1].time;
	}
	return totalDuration;
}



//------------------------------------------------------------------
void pointRecorder::applyOffset(){
	for( int i = 0; i < pts.size(); i++ ){
		pts[i].pos.x += offsetX; 
		pts[i].pos.y += offsetY; 
	}
	
	offsetX = 0; 
	offsetY = 0; 
}



//------------------------------------------------------------------
ofPoint pointRecorder::getPointForTime(float time){
	
	// ok here's how to do it.
	// (a) if we have no points, or one point, or are recording return nothing...
	
	if (pts.size() <= 1 /*|| bAmRecording*/) return ofPoint(0,0,0); 
	
	// (b) else, find out where we are for a given time 
	// do something like a % operator with the time, 
	// easier to see it in integer math:
	// for example, if our total time is 3, and we look for time 0, we are at time 0
	// if we look at 1, we are at time 1  (1 % 3)
	// if we look at 2, we are at time 2  (2 % 3)
	// if we look at 3, we are at time 0  (3 % 3)
	// if we look at 4, we are at time 1  (4 % 3)
	// (ie, we use mod to wrap over)
	// for example, if are at time 11, we do 11 % 3, which equals 2, so we are at time 2....
	
	// now, we do that with floats, like this:
	//float totalTime = getDuration();
	//float timeInRange = time;
	//while (timeInRange > totalTime){
	//	timeInRange -= totalTime;
	//}
	//
	// ^^^ actually... we don't need to do this anymore at all. 
	// this is the new code: 
	float timeInRange = time;
	if( time >= getDuration() ){
		cout << "fake pos!" << endl; 
		return pts[pts.size()-1].pos; 
	}
	
	// ok we are looking for a point whose time is greater then timeInRange...
	int whatPointAmINear = 0;
	for (int i = 0; i < pts.size(); i++){
		if (pts[i].time > timeInRange){
			whatPointAmINear = i;
			break; // leave the for loop :)
		}
	}
	
	// now, if that point is greater then 0, we will linearly interpolate
	// else return pt0
	
	if (whatPointAmINear > 0){
		float timea = pts[whatPointAmINear - 1].time;
		ofPoint pta = pts[whatPointAmINear - 1].pos;
		float timeb = pts[whatPointAmINear    ].time;
		ofPoint ptb = pts[whatPointAmINear    ].pos;
		float totalDurationBetweenThesePts	= timeb - timea;
		float myPositionBetweenThesePts		= timeInRange - timea;
		float pct = myPositionBetweenThesePts / totalDurationBetweenThesePts; 
		ofPoint mix(0,0,0);
		mix.x = (1-pct) * pta.x + (pct) * ptb.x;
		mix.y = (1-pct) * pta.y + (pct) * ptb.y;
		return mix;
	} else {
		return pts[whatPointAmINear].pos;
	}
}



ofPoint pointRecorder::getVelocityForTime(float time){
	
	
	// ok the trick is to do the same as above (find the time in the gesture)
	// and to find a time slightly less... (prev position and current position).
	
	
	if (pts.size() <= 1 /*|| bAmRecording*/) return ofPoint(0,0,0); 
	
	/*float totalTime = getDuration();
	float timeInRange = time;
	while (timeInRange > totalTime){
		timeInRange -= totalTime;
	}*/
/*	if( time >= getDuration() ){
		ofPoint velocity; 
		velocity.x = 0; 
		velocity.y = 0; 

		ofPoint prevPoint	= pts[pts.size()-1].
		return velocity; 
	}*/
	
	float timeInRange = time; 
	
	float prevTime = MAX(0, timeInRange - 0.016666f); // time minus 1/60 of a second....
	//cout << timeInRange << " " << prevTime << endl;
	
	ofPoint curPoint	= getPointForTime(timeInRange);
	ofPoint prevPoint	= getPointForTime(prevTime);
	
	
	
	ofPoint velocity;
	
	velocity.x			= curPoint.x - prevPoint.x;
	velocity.y			= curPoint.y - prevPoint.y;
	
	return velocity;
}

bool pointRecorder::active(){
	return startTime != 0 && enabled != 0; 
}



void pointRecorder::save( ofstream& out ){
	
	if( startTime == 0 ){
		out << "END:" << endl; 
		return; 
	}
	
	float SCALE_X = 800.0/ofGetWidth();
	float SCALE_Y = 600.0/ofGetHeight(); 
	
	out << "startTime:" << startTime << endl; 
	out << "offsetX:" << offsetX << endl; 
	out << "offsetY:" << offsetY << endl; 
	out << "maxNumPts:" << maxNumPts << endl; 
	out << "beatMod:" << beatMod << endl; 
	out << "volume:" << volume << endl; 
	out << "soundShape:" << soundShape << endl; 
	out << "enabled:" << enabled << endl; 
	out << "triggerAlways:" << triggerAlways << endl;
	out << "startDelay:" << startDelay << endl; 
	out << "babysitter:" << (babysitter==NULL?-1:(babysitter->index)) << endl; 
	
	out << "timePt:"; 
	for( int i = 0; i < pts.size(); i++ ){
		out << pts[i].time << " " << (SCALE_X*pts[i].pos.x) << " " << (SCALE_Y*pts[i].pos.y) << " "; 
	}
	out << "-1" << endl;
	
	out << "kids:"; 
	for( int i = 0; i < kids.size(); i++ ){
		out << kids[i]->index << " "; 
	}
	out << "-1" << endl; 
	
	out << "kidPointNr:"; 
	for( int i = 0; i < kidPointNr.size(); i++ ){
		out << kidPointNr[i] << " "; 
	}
	out << "-1" << endl; 
	
	out << "babysitting:"; 
	for( int i = 0; i < babysitting.size(); i++ ){
		out << babysitting[i]->index << " "; 
	}
	out << "-1" << endl; 
	
	out << "END:" << endl;
}

void pointRecorder::load( ifstream& in, pointRecorder recorders[], pointPlayer players[] ){
	char cmd[64]; 
	
	// Manually set this to false! 
	bAmRecording = false; 

	float SCALE_X = ofGetWidth()/800.0;
	float SCALE_Y = ofGetHeight()/600.0; 
	
	
	while( !in.eof() ){
		Helpers::readCommand( cmd, in ); 
		
		if( 0 == strcmp( cmd, "startTime" ) ) in >> startTime; 
		if( 0 == strcmp( cmd, "offsetX" ) ) in >> offsetX; 
		if( 0 == strcmp( cmd, "offsetY" ) ) in >> offsetY; 
		if( 0 == strcmp( cmd, "maxNumPts" ) ) in >> maxNumPts; 
		if( 0 == strcmp( cmd, "beatMod" ) ) in >> beatMod; 
		if( 0 == strcmp( cmd, "volume" ) ) in >> volume; 
		if( 0 == strcmp( cmd, "soundShape" ) ) in >> soundShape; 
		if( 0 == strcmp( cmd, "enabled" ) ) in >> enabled; 
		if( 0 == strcmp( cmd, "triggerAlways" ) ) in >> triggerAlways; 
		if( 0 == strcmp( cmd, "startDelay" ) ) in >> startDelay; 
		if( 0 == strcmp( cmd, "babysitter" ) ){
			int i; in >> i; 
			if( i == -1 ){
				babysitter = NULL; 
			}
			else{
				babysitter = &recorders[i]; 
			}
		}
		
		if( 0 == strcmp( cmd, "timePt" ) ){
			float t; 
			in >> t; 
			
			while( t != -1 && !in.eof() ){
				timePt	point;
				point.pos = ofPoint(0,0);
				in >> point.pos.x; 
				in >> point.pos.y; 
				point.pos.x *= SCALE_X; 
				point.pos.y *= SCALE_Y; 
				point.time = t;
				pts.push_back( point ); 
				in >> t; 
			}
		}
		
		if( 0 == strcmp( cmd, "kids" ) ){
			int i; 
			in >> i; 
			while( i != -1 && !in.eof() ){
				kids.push_back( &recorders[i] ); 
				in >> i; 
			}
		}
		
		if( 0 == strcmp( cmd, "kidPointNr" ) ){
			int i; in >> i; 
			while( i != -1 && !in.eof() ){
				kidPointNr.push_back( i ); 
				in >> i; 
			}
		}
		
		if( 0 == strcmp( cmd, "babysitting" ) ){
			int i; in >> i; 
			while( i != -1 && !in.eof() ){
				babysitting.push_back( &recorders[i] ); 
				in >> i; 
			}
		}
		
		
		if( 0 == strcmp( cmd, "END" ) ) break; 
	}
}

