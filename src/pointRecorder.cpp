#include "pointRecorder.h"


//------------------------------------------------------------------
pointRecorder::pointRecorder(){
	maxNumPts = 500;  // try 50, for example....
	startTime = 0;
}

//------------------------------------------------------------------
void pointRecorder::draw() {
	
	//---------------------------
	// (a) create a small report 
	ofSetColor(100,100,100);
	// get the number of points and the time of the last point:
	int nPts = pts.size();
	float totalDuration = getDuration();
	//string report = "nPts = " + ofToString(nPts) + "\ntotal time = " + ofToString(totalDuration, 3);
	//ofDrawBitmapString(report, 10, 10);
	
	//---------------------------
	// (b) draw the line: 
	if (bAmRecording)		ofSetColor(255,255,0);
	else					ofSetColor(80,80,80);
	
	ofNoFill();
	ofBeginShape();
	for (int i = 0; i < pts.size(); i++){
		ofVertex(pts[i].pos.x, pts[i].pos.y);
	}
	
	ofEndShape(false);
	
	if( pts.size() > 1 ){
		ofFill(); 
		float radius = 2+2*volume/0.1;
		ofCircle( pts[0].pos.x, pts[0].pos.y, radius ); 
	}
	
	
}

//------------------------------------------------------------------
void pointRecorder::addPoint(ofPoint pt) {
	
	
	// on the first point, grab the start time
	if (pts.size() == 0){
		startTime = ofGetElapsedTimef();
	} 
	
	
	// combine the position and the time here: 
	timePt	myPoint;
	myPoint.pos			= pt;
	myPoint.time		= ofGetElapsedTimef() - startTime;
	
	cout << "creating point " << pts.size() << " at time: " << myPoint.time << endl;

	pts.push_back(myPoint);
	if (pts.size() > maxNumPts){
		pts.erase(pts.begin());
	}
}

//------------------------------------------------------------------
void pointRecorder::reset( int beatMod ) {
	this->pts.clear();
	this->bAmRecording = true;
	this->beatMod = beatMod; 
	this->volume = 0.1f;
	this->startTime = 0; 
	
	this->kids.clear(); 
	this->kidPointNr.clear(); 
}

//------------------------------------------------------------------
float pointRecorder::getDuration() {
	float totalDuration = 0;
	if (pts.size() > 0){
		totalDuration = pts[pts.size() - 1].time;
	}
	return totalDuration;
}

ofPoint pointRecorder::getPointForTime(float time){
	
	// ok here's how to do it.
	// (a) if we have no points, or one point, or are recording return nothing...
	
	if (pts.size() <= 1 || bAmRecording) return ofPoint(0,0,0); 
	
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
	float totalTime = getDuration();
	float timeInRange = time;
	while (timeInRange > totalTime){
		timeInRange -= totalTime;
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
	
	
	if (pts.size() <= 1 || bAmRecording) return ofPoint(0,0,0); 
	
	float totalTime = getDuration();
	float timeInRange = time;
	while (timeInRange > totalTime){
		timeInRange -= totalTime;
	}
	
	float prevTime = MAX(0, timeInRange - 0.016666f); // time minus 1/60 of a second....
	//cout << timeInRange << " " << prevTime << endl;
	
	ofPoint curPoint	= getPointForTime(timeInRange);
	ofPoint prevPoint	= getPointForTime(prevTime);
	
	
	
	ofPoint velocity;
	
	velocity.x			= curPoint.x - prevPoint.x;
	velocity.y			= curPoint.y - prevPoint.y;
	
	return velocity;
}
