#ifndef CODE_ANALYSIS
#include "ofMain.h"
#endif

#include "ofApp.h"

Vector2D appsize = {1024, 768};
ThumbObject* selectedImage;
ThumbObject* highlightedImage;

ofxPanel gui_metadatamenu;
ofxInputField<string> gui_metadatamenu_tags;
ofxLabel gui_metadatamenu_luminance;
ofxLabel gui_metadatamenu_color;
ofxLabel gui_metadatamenu_faces;
ofxLabel gui_metadatamenu_edge;
ofxLabel gui_metadatamenu_cuts;

//========================================================================
int main() {

	ofSetupOpenGL(appsize.x, appsize.y, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
} 