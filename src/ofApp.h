#pragma once
/*
This code creates video morphing, through the use of ulploaded images, ofxOpenCv addon, OpenCV, ofXPiMapper and ofxKinect.
 It is a live, project mapped installation that works with a Kinect camara v1, depht camara.
The code inside the class "Morphing" computes optical flow between two images, and use them to morphing some other image. A morphing value which has direct relation with the pixels detected by the kinect camara generate the distortion of the image.
Images in the kinect are been translated to pixels and the closest point is calculated by and average of the brightest pixels capture. The code gets the location of it (x,y) and uses the x-position to set morphing values.
--------------------------------------
This code is partially based in the the example 05-VideoMorphing from the book
"Mastering openFrameworks: Creative Coding Demystified",
Chapter 9 - Computer Vision with OpenCV
*/


#include "ofMain.h"
#include "ofxPiMapper.h"
#include "VideoSource.h"
#include "Morphing.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);

    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);

    ofxPiMapper piMapper;   //declaring a variable for projection mapping
    Morphing morphing;      //declaring an object for the class morphing
    
};
