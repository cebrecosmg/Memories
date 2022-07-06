#pragma once

#include "ofMain.h"
#include "FboSource.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
using namespace cv;

class Morphing : public ofx::piMapper::FboSource {
	public:

    void setup();
    void update();
    void draw();

//Declaring variables for MORPHING IMAGE
    
    ofxCvColorImage color1, color2;      //First and second original images
    ofxCvGrayscaleImage gray1, gray2;    //Decimated grayscaled images
    ofxCvFloatImage flowX, flowY;        //Resulted optical flow in x and y axes
    ofxCvColorImage colorTest;           //Input image "checkerboard" for morphing

    int w, h;                            //Decimated size of input images

    ofxCvGrayscaleImage planeX, planeY;
    ofxCvFloatImage idX, idY;            //idX(x,y) = x, idY(x,y) = y
    ofxCvFloatImage mapX, mapY;
    ofxCvFloatImage bigMapX, bigMapY;
    ofxCvFloatImage fx, fy;
    ofxCvFloatImage weight;

    float morphValue;                    //[0, 1] or more to get it full distorted
    ofxCvColorImage morph;               //Resulted morphed image
    int morphImageIndex;                 //What to morph: 1 - first input image, 2 - checkerboard image
    
    //Inverting the mapping (mapX, mapY), with antialiasing.
    void inverseMapping( ofxCvFloatImage &mapX, ofxCvFloatImage &mapY );

    //Making image morphing
    void updateMorph( float morphValue, int morphImageIndex );
    void multiplyByScalar( ofxCvFloatImage &floatImage, float value);
    void keyPressed(int key);
    void location(int x, int y );       //to generate the interaction with the values closest values detected in                                        the kinect and change the morphic value
    
//_______________________________
//Declaring variables for KINECT
    void exit();

    ofxKinect kinect;
    
    ofxCvColorImage colorImg;           // colour image
    ofxCvGrayscaleImage grayImage;      // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar;  // the far thresholded image
    ofxCvContourFinder contourFinder;   // for generating countours
    
    bool bThreshWithOpenCV;
    
    int nearThreshold;                  //for the nearest point detected
    int farThreshold;                   //for the furthest point detected
    int angle;                          // angle of the kinect camera
    int threshold;
    float closestColorX, closestColorY; //declatering variables for the closest pixel coordinates
    int count;                          // to calculate an average
    
//_______________________________
//Declaring variables for sound
    
    ofSoundPlayer water;
    ofSoundPlayer jungle;
    
    
};
