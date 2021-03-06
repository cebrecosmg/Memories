#include "Morphing.h"
#include "opencv2/opencv.hpp"
#include "ofxKinect.h"

void Morphing::setup(){
    
    //CODE for Projection Mapping
    name = "Morphing FBO Source"; //Source name
	allocate(500, 500); // Allocate our FBO source, decide how big it should be
    
    //MY CODE:
    ofSetFrameRate(30); // set the frame rate fast enough to capture movement

    //KINECT
    ofSetLogLevel(OF_LOG_VERBOSE);

    // enable depth->video image calibration
    kinect.setRegistration(true);

    kinect.init();
    //kinect.init(true); // shows infrared instead of RGB video image
    //kinect.init(false, false); // disable video image (faster fps)

    kinect.open();        // opens first available kinect
    //kinect.open(1);    // open a kinect by id, starting with 0 (sorted by serial # lexicographically))
    //kinect.open("A00362A08602047A");    // open a kinect using it's unique serial #

    // print the intrinsic IR sensor values:
    if(kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }
    
    //Allocation Color image, gray image and thresholds in relation to the kinect:
    colorImg.allocate(kinect.width, kinect.height);
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);

    //Setting the slice where the interaction will take place
    //Values are between 0 and 255 as grayscale is used to settle the furthest and nearest pixel
    nearThreshold = 255; // closer to a white pixel
    farThreshold = 100;
    bThreshWithOpenCV = true;
    // brightness tracking
    threshold = 251; // hardcoded
    
    // zero the tilt on startup
    angle = 0;   // Starting angle of the kinect
    kinect.setCameraTiltAngle(angle);


//END KINECT
//______________________
//MORPHING IMAGE
    
    ofImage image1, image2; //declaring variables for each image
    
    //Load openFrameworks' images:
    image1.loadImage("CE1.jpg");  //h1
    image2.loadImage("CE2.jpg");  //h2 for nature amazon

    //Convert to ofxCv images
    color1.setFromPixels( image1 );
    color2.setFromPixels( image2 );

    float decimate = 0.3;              //Decimate images to 30%
    ofxCvColorImage imageDecimated1;
    imageDecimated1.allocate( color1.width * decimate,
                          color1.height * decimate );
    //High-quality resize
    imageDecimated1.scaleIntoMe( color1, CV_INTER_AREA );
    gray1 = imageDecimated1;

    ofxCvColorImage imageDecimated2;
    imageDecimated2.allocate( color2.width * decimate,
                              color2.height * decimate );
    //High-quality resize
    imageDecimated2.scaleIntoMe( color2, CV_INTER_AREA );
    gray2 = imageDecimated2;

    Mat img1 = cvarrToMat( gray1.getCvImage() );  //Create OpenCV images
    Mat img2 = cvarrToMat( gray2.getCvImage() );  //Create OpenCV images
    Mat flow;                        //Image for flow
    
    //Computing optical flow
      calcOpticalFlowFarneback( img1, img2, flow, 0.7, 3, 11, 5, 5, 1.1, 0 ); //if change values the effect on the image will be changed, this is a controled distorition.

    //Split flow into separate images
    vector<Mat> flowPlanes;
    split( flow, flowPlanes );
    
    //Copy float planes to ofxCv images flowX and flowY
    IplImage iplX( flowPlanes[0] );
    flowX = &iplX;
    IplImage iplY( flowPlanes[1] );
    flowY = &iplY;

    w = gray1.width;
    h = gray1.height;

    //Flow image
    planeX = flowX;
    planeY = flowY;

    //create idX, idy
    idX.allocate( w, h );
    idY.allocate( w, h );
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            idX.getPixelsAsFloats()[ x + w * y ] = x;
            idY.getPixelsAsFloats()[ x + w * y ] = y;
        }
    }

    //Load checkerboard image for testing the effect
    ofImage imageTest;
    imageTest.loadImage("checkerBoard.png");
    colorTest.setFromPixels( imageTest );

    //Make morphing at first time
    morphValue = 0;
    morphImageIndex = 1;
    updateMorph( morphValue, morphImageIndex);      //this will be updated as the location of the brightest point                                               will make the morphValue change
    
    //_______________________________
    //Sound
    jungle.load("Sultrasonic.mp3");
    
    jungle.setVolume(1);
    
    jungle.play();
    
    jungle.setLoop(true);
    


    
    
}


//--------------------------------------------------------------
void Morphing::multiplyByScalar( ofxCvFloatImage &floatImage, float value )
{
    int w = floatImage.width;
    int h = floatImage.height;
    float *floatPixels = floatImage.getPixelsAsFloats();
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            floatPixels[ x + w * y ] *= value;
        }
    }
    floatImage.flagImageChanged();

}

//--------------------------------------------------------------
void Morphing::update(){
    
    kinect.update();
    
    // there is a new frame and we are connected
    if(kinect.isFrameNew()) {
        
        // load grayscale depth image from the kinect source
        grayImage.setFromPixels(kinect.getDepthPixels());
        
        count = 0;  // how we do average
        closestColorX = 0; // resets to zero each loop, then goes and does the averaging
        closestColorY = 0;

        for (int y=0; y<grayImage.height; y++) {
            for (int x=0; x<grayImage.width; x++) {

                // Extract the color components of the pixel at (x,y)
                // from grayImage (or some other image source)
                ofColor colorAtXY = grayImage.getPixels().getColor(x, y);
                float rAtXY = colorAtXY.r;
                float gAtXY = colorAtXY.g;
                float bAtXY = colorAtXY.b;

                //if color similar then add coordinates to sum
                if( rAtXY > threshold){  // used rAtXY because it's all the same values
                    closestColorX+= x;
                    closestColorY+= y;
                    count++;
                }
            }
        }

        //calculate the average coordinate
        if (count>0) {
            closestColorX = closestColorX / count;      //average in the x axis
            closestColorY = closestColorY / count;      //average in the y axis
            
            location(closestColorX, closestColorY);      // this will get the coordinates of the closest point =                                            brightest point
        }

   //-------------end of brightness tracking ------------------
        
        // we do two thresholds - one for the far plane and one for the near plane
        if(bThreshWithOpenCV) {
            grayThreshNear = grayImage;
            grayThreshFar = grayImage;
            grayThreshNear.threshold(nearThreshold, true);
            grayThreshFar.threshold(farThreshold);
            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL); //cvAnd to get the pixels which are a union of the two thresholds
        } else {
            
            // Working with grayscale pixels and pixels between the thresholds
            ofPixels & pix = grayImage.getPixels();
            int numPixels = pix.size();
            for(int i = 0; i < numPixels; i++) {
                if(pix[i] < nearThreshold && pix[i] > farThreshold) {
                    pix[i] = 255;
                } else {
                    pix[i] = 0;
                }
            }
        }
        
        // update the cv images
        grayImage.flagImageChanged();
        
        contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false); //finds the contours of the images detected inside the thresholds
    }
    
    //____________
    //SOUND
    float soundSpeed;
    soundSpeed = ofMap(closestColorX, 0, ofGetWidth(), 1, -10);
    jungle.setSpeed(soundSpeed);
    
}

//--------------------------------------------------------------

void Morphing::draw(){
    
    ofClear(0); //clear the buffer
    ofBackground( 0, 0, 0);    //Set the background to black
    
    //KINECT CODE
    
        grayImage.draw(0, 0, kinect.width, kinect.height);              //draw from live kinect in grayscale
        contourFinder.draw(0, 0, kinect.width, kinect.height);          //draw Contours images
        
        //For testing. The following two can be comment afterwards:
        ofSetColor(255, 0, 0);
        //ofDrawCircle(closestColorX, closestColorY, 20);                  // draw red dot to test, debug
        
    // draw instructions
    ofSetColor(255, 255, 255);
    stringstream reportStream;
        
    if(kinect.hasAccelControl()) {
        reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
        << ofToString(kinect.getMksAccel().y, 2) << " / "
        << ofToString(kinect.getMksAccel().z, 2) << endl;
    } else {
        reportStream << "Note: this is a newer Xbox Kinect or Kinect For Windows device," << endl
        << "motor / led / accel controls are not currently supported" << endl << endl;
    }
    
    reportStream << "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
    << "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
    << "set near threshold " << nearThreshold << " (press: + -)" << endl
    << "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
    << ", fps: " << ofGetFrameRate() << endl
    << "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl;

    if(kinect.hasCamTiltControl()) {
        reportStream << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
        << "press 1-5 & 0 to change the led mode" << endl;
    }
    
    ofDrawBitmapString(reportStream.str(), 20, 652);
    
    //___________________END KINECT
    
    //MORPH IMAGE CODE:
    
    int w = gray1.width;
    int h = gray1.height;

    //Input images + optical flow:
    morph.draw( 0, 0 );         //This draws the final Morphed IMAGE

    //Output text with current morhping value:
    ofSetColor( 0, 0, 0 );
    ofDrawBitmapString( "Morphing value: " + ofToString( morphValue * 100, 1 ) + "%", morph.getWidth() + 20, ofGetHeight() - 20 ); // prints out the Morphing value
}

//__________________________
//Function to make the morphing image to constanly update by the morphValue)

void Morphing::updateMorph( float morphValue, int morphImageIndex ){
    
    mapX.allocate( w, h );            //w and h is size of gray1 image
    mapY.allocate( w, h );

    //Get pointers to pixels data
    float *flowXPixels = flowX.getPixelsAsFloats();
    float *flowYPixels = flowY.getPixelsAsFloats();
    float *mapXPixels = mapX.getPixelsAsFloats();
    float *mapYPixels = mapY.getPixelsAsFloats();
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            int i = x + w * y;        //pixels' index
            mapXPixels[ i ] = x + flowXPixels[ i ] * morphValue;
            mapYPixels[ i ] = y + flowYPixels[ i ] * morphValue;
        }
    }
    mapX.flagImageChanged();        //Notify that pixels values were changed
    mapY.flagImageChanged();

    inverseMapping( mapX, mapY );

    //Creating big maps out of the images:
    int W = color1.width;
    int H = color1.height;
    if ( !bigMapX.bAllocated ) {
        bigMapX.allocate( W, H );
        bigMapY.allocate( W, H );
    }
    //bigMapX and bigMapY have type ofxCvFloatImage
    bigMapX.scaleIntoMe( mapX, CV_INTER_LINEAR );
    bigMapY.scaleIntoMe( mapY, CV_INTER_LINEAR );
    multiplyByScalar( bigMapX, 1.0 * W / w );
    multiplyByScalar( bigMapY, 1.0 * H / h );

    //Select image to morph
    if ( morphImageIndex == 1 ) {
        morph = color1;        //First input image
    }
    else {
        morph = colorTest;    //Checkerboard image
    }
    morph.remap( bigMapX.getCvImage(), bigMapY.getCvImage() );
    
}

//--------------------------------------------------------------
//Inverting the mapping (mapX, mapY), with antialiasing.

void Morphing::inverseMapping( ofxCvFloatImage &mapX, ofxCvFloatImage &mapY ){
    if ( !fx.bAllocated ) {
        fx.allocate( w, h );
        fy.allocate( w, h );
        weight.allocate( w, h );
    }
    fx.set( 0 );
    fy.set( 0 );
    weight.set( 0 );

    float *mapXPixels = mapX.getPixelsAsFloats();
    float *mapYPixels = mapY.getPixelsAsFloats();
    float *fxPixels = fx.getPixelsAsFloats();
    float *fyPixels = fy.getPixelsAsFloats();
    float *weightPixels = weight.getPixelsAsFloats();

    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            float MX = mapXPixels[ x + w * y ];
            float MY = mapYPixels[ x + w * y ];

            int mx0 = int( MX );
            int my0 = int( MY );
            int mx1 = mx0 + 1;
            int my1 = my0 + 1;
            float weightX = MX - mx0;
            float weightY = MY - my0;

           //Not all the pixels are modified, some are clamp
            mx0 = ofClamp( mx0, 0, w-1 );    //Bound
            my0 = ofClamp( my0, 0, h-1 );
            mx1 = ofClamp( mx1, 0, w-1 );
            my1 = ofClamp( my1, 0, h-1 );
            for (int b=0; b<2; b++) {
                for (int a=0; a<2; a++) {
                    int x1 = ( a == 0 ) ? mx0 : mx1;
                    int y1 = ( b == 0 ) ? my0 : my1;
                    int i1 = x1 + w * y1;
                    float wgh = ( ( a == 0 ) ? ( 1 - weightX ) : weightX )
                        * ( ( b == 0 ) ? ( 1 - weightY ) : weightY );
                    fxPixels[ i1 ] += x * wgh;
                    fyPixels[ i1 ] += y * wgh;
                    weightPixels[ i1 ] += wgh;
                }
            }
        }
    }

    //Compute map for non-zero weighted pixels
    int zeros = 0;        //Count of zeros pixels
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            int i = x + w * y;
            float X = fxPixels[ i ];
            float Y = fyPixels[ i ];
            float Weight = weightPixels[ i ];
            if ( Weight > 0 ) {
                X /= Weight;
                Y /= Weight;
            }
            else {
                X = x;
                Y = y;
                zeros++;
            }
            mapXPixels[ i ] = X;
            mapYPixels[ i ] = Y;
            weightPixels[ i ] = Weight;
        }
    }

    //Fill zero-weighted pixels by weighting of near non-zero weighted pixels
    const int rad = 2;
    const int diam = 2 * rad + 1;
    float filter[ diam * diam ];
    float sum = 0;
    for (int b=-rad; b<=rad; b++) {
        for (int a=-rad; a<=rad; a++) {
            float wgh = rad + 1 - max( abs( a ), abs( b ) );
            filter[ a+rad + diam * (b+rad) ] = wgh;
            sum += wgh;
        }
    }
    for (int i=0; i<diam*diam; i++) {
        filter[ i ] /= sum;
    }

    int zeros0 = -1;
    while ( zeros > 0 && (zeros0 == -1 || zeros0 > zeros) ) {
        zeros0 = zeros;
        zeros = 0;
        for (int y=0; y<h; y++) {
            for (int x=0; x<w; x++) {
                int i = x + w * y;
                if (weightPixels[ i ] < 0.0001 ) {
                    float mX = 0;
                    float mY = 0;
                    float mWeight = 0;
                    int x1, y1, i1;
                    float wgh;
                    for (int b = -rad; b<=rad; b++) {
                        for (int a = -rad; a<=rad; a++) {
                            x1 = x + a;
                            y1 = y + b;
                            if ( ofInRange( x1, 0, w-1 ) && ofInRange( y1, 0, h-1 ) ) {
                                i1 = x1 + w * y1;
                                if ( weightPixels[ i1 ] >= 0.0001 ) {
                                    wgh = filter[ a+rad + diam * (b+rad) ] * weightPixels[ i1 ];
                                    mX += mapXPixels[i1] * wgh;
                                    mY += mapYPixels[i1] * wgh;
                                    mWeight += wgh;
                                }
                            }
                        }
                    }
                    if ( mWeight > 0 ) {
                        mapXPixels[ i ] = mX / mWeight;
                        mapYPixels[ i ] = mY / mWeight;
                        weightPixels[ i ] = mWeight;
                    }
                    else {
                        zeros++;
                    }
                }
            }
        }
    }

    mapX.flagImageChanged();
    mapY.flagImageChanged();
}

//--------------------------------------------------------------
void Morphing::keyPressed(int key){
    if ( key == '1' ) {        //Select first image to morph
        morphImageIndex = 1;
        updateMorph( morphValue, morphImageIndex );
    }
    if ( key == '2' ) {        //Select checkerboard image to morph
        morphImageIndex = 2;
        updateMorph( morphValue, morphImageIndex );
    }
}

//--------------------------------------------------------------
void Morphing::location(int x, int y ){
    morphValue = ofMap(x, 0, kinect.width, 0, 15 ); // Last number will distort more the image, the bigger it is to 1. X now need to be the brightness
    // 10 is an interesting distortion
    std::cout << "value: " << x << endl;
    //0 o 60 a 560
    updateMorph( morphValue, morphImageIndex );
}

//--------------------------------------------------------------
void Morphing::exit() {
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();

}



