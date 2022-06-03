/* Refer to the README.md in the example's root folder for more information on usage */

#pragma once

#ifndef CODE_ANALYSIS
#include "ofMain.h"
#include "ofxOpenCv.h"
#endif



struct m_col {
	float red;
	float green;
	float blue;
};

class ofApp : public ofBaseApp {

public:

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	float calculateLuminance(int image);
	m_col calculateColor(int image);
	double* calculateGabor(int image, double* avgArray);
	double* calculateEdges(int image, double* avgArray);
	int haarFaces(int image, ofxCvHaarFinder hF);
	bool ofApp::detectMatchingFeatures(int image1, int image2);
	void ofApp::match(cv::Mat& desc1, cv::Mat& desc2, vector<cv::DMatch>& matches);
	void ofApp::detectAndCompute(cv::Mat& img, vector<cv::KeyPoint>& kpts, cv::Mat& desc);
	int ofApp::detectCut(int image1, int image2);

	// we will have a dynamic number of images, based on the content of a directory:
	ofDirectory dir;
	vector<ofImage> images;

	int currentImage;

};