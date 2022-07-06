#include "CustomSource1.h"

void CustomSource1::setup(){
	// Give our source a decent name
    name = "CustomSource1 FBO Source";
    rectColor = ofColor(255);
	// Allocate our FBO source, decide how big it should be
    allocate(500, 500);
}

void CustomSource1::setName(string _name){
    name = _name;
}

void CustomSource1::setColor(ofColor c){
    rectColor = c;
}

// Don't do any drawing here
void CustomSource1::update(){
}

// No need to take care of fbo.begin() and fbo.end() here.
// All within draw() is being rendered into fbo;
void CustomSource1::draw(){
    ofClear(0); //clear the buffer
    ofSetColor(255, 1, 20);
    ofDrawCircle(250, 250, 100);

    //do it with static values
    //since the buffer is 500x500, then...
    //drawMovingRect(250, 250, 500, 500);

    //or pass it dynamic values (ie. ask the fbo how big it is)
}

    

