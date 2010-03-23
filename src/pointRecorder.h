#ifndef POINT_RECORDER_H
#define POINT_RECORDER_H

#include "ofMain.h"
#include "pointPlayer.h"
#include "Helpers.h"
#include <fstream>


typedef struct {
	ofPoint		pos;
	float		time; // from the start, what time does this pt occur at	
} timePt;

class pointPlayer;


class pointRecorder {

	public:
	
        pointRecorder();
	
		void addPoint(ofPoint pt);
		void draw();
		void reset( int beatMod );
		float getDuration();
		void applyOffset(); 
		bool active(); 
	
		ofPoint getPointForTime(float time);
		ofPoint getVelocityForTime(float time);
		
		void save( ofstream& out ); 
		void load( ifstream& in, pointRecorder recorders[], pointPlayer players[] );
		
		long				startTime;
		int					maxNumPts;
		vector <timePt>		pts;
	
		bool				bAmRecording;			// we don't playback when we are recording...
		
		int beatMod; 
		float volume;
		
		// A line might have an arbitrary number of kids... 
		vector <pointRecorder*> kids;
		vector <int> kidPointNr; // at which point of the line do we attach? 
	
		// A line might be parenting another line... 
		vector <pointRecorder*> babysitting; // If this line is the parent of another line we store that other line here (who's being babysitted?)
		pointRecorder * babysitter; // If a line is being parented by another line we store that other line here (who's the babysitter?)
		
		// if being babysitted it is interresting to remember how far away from 
		// the original line we are! 
		float babysitterX, babysitterY; 
		
		int offsetX; 
		int offsetY; 
		int soundShape; 
	
		bool enabled; 
		bool triggerAlways; 
		int index;
};

#endif // POINT_RECORDER_H