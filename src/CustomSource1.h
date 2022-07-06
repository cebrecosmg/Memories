#pragma once

#include "ofMain.h"
#include "FboSource.h"

class CustomSource1 : public ofx::piMapper::FboSource {
	public:
        void setup();
		void update();
		void draw();
        void setName(string _name);
        void setColor(ofColor);
        ofColor rectColor;
};
