#include "ofApp.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"

#define M_PI 3.14159265358979323846
//--------------------------------------------------------------
void ofApp::setup() {

	dir.listDir("images/of_logos/");
	dir.allowExt("jpg");
	dir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

	//allocate the vector to have as many ofImages as files
	if (dir.size()) {
		images.assign(dir.size(), ofImage());
	}

	// you can now iterate through the files and load them into the ofImage vector
	for (int i = 0; i < (int)dir.size(); i++) {
		images[i].load(dir.getPath(i));
	}
	currentImage = 0;

	ofBackground(ofColor::black);
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	if (dir.size() > 0) {
		ofSetColor(ofColor::white);
		images[currentImage].draw(300, 50, 256, 256);

		ofSetColor(ofColor::gray);
		m_col colorValue = calculateColor(currentImage);
		string pathInfo = dir.getName(currentImage) + " " + dir.getPath(currentImage) + "\n\n" +
			"press any key to advance current image\n\n" +
			"many thanks to hikaru furuhashi for the OFs" +
			"\n Image Luminance: " + std::to_string(calculateLuminance(currentImage)) +
			"\n Image Color: " + std::to_string(colorValue.red) + "," + std::to_string(colorValue.green) + "," + std::to_string(colorValue.blue);
		ofDrawBitmapString(pathInfo, 300, 256 + 80);
	}

	ofSetColor(ofColor::gray);
	for (int i = 0; i < (int)dir.size(); i++) {
		if (i == currentImage) {
			ofSetColor(ofColor::red);
		}
		else {
			ofSetColor(ofColor::blueSteel);
		}
		string fileInfo = "file " + ofToString(i + 1) + " = " + dir.getName(i);
		ofDrawBitmapString(fileInfo, 50, i * 20 + 50);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {


	if (dir.size() > 0) {

		switch (key) {
		case (57356):
		case (57357):
			currentImage--;
			currentImage %= dir.size();
			break;
		case (57358):
		case (57359):
			currentImage++;
			currentImage %= dir.size();
			break;


		}

	}

}
float luminanceFunction(ofColor color) {
	return (0.2126 * (float)color.r + 0.7152 * (float)color.g + 0.0722 * (float)color.b);
}

float ofApp::calculateLuminance(int image) {
	ofImage currentImage = images[image];
	ofPixels imagePixels = currentImage.getPixels();
	float sumLuminance = 0;
	int vectorSize = imagePixels.getWidth() * imagePixels.getHeight();
	if (vectorSize > 0) {
		for (int i = 0; i < vectorSize; i++) {
			ofColor colorAtIndex = imagePixels.getColor(i);
			sumLuminance += luminanceFunction(colorAtIndex);
		}
		sumLuminance = sumLuminance / vectorSize;
	}
	return sumLuminance;
}




m_col ofApp::calculateColor(int image) {


	ofImage currentImage = images[image];
	ofPixels imagePixels = currentImage.getPixels();
	m_col sumColor = { 0, 0, 0 };
	float vectorSize = imagePixels.getWidth() * imagePixels.getHeight();
	printf("---------- Vector Size %ld\n", imagePixels.getWidth() * imagePixels.getHeight());
	if (vectorSize > 0) {
		for (int y = 0; y < imagePixels.getHeight(); y++) {
			for (int x = 0; x < imagePixels.getWidth(); x++) {
				ofColor colorAtIndex = imagePixels.getColor(x, y);
				sumColor.red += (float)colorAtIndex.r;
				sumColor.green += (float)colorAtIndex.g;
				sumColor.blue += (float)colorAtIndex.b;
			}
		}

		float div = 1 / vectorSize;
		sumColor.red *= div;
		sumColor.green *= div;
		sumColor.blue *= div;
	}
	//imagePixels.
	return sumColor;
}

double* ofApp :: calculateAngle(int image, double* avgArray) {
	ofImage currentImage = images[image];
	//TODO converter grayscale
	ofPixels imagePixels = currentImage.getPixels();
	ofImage outputImage = currentImage;

	ofxCvColorImage img;
	img.setFromPixels(imagePixels);
	cv::Mat m = ofxCv::toCv(img.getPixels());
	cv::InputArray inputArr = cv::InputArray(m);
	
	ofxCvColorImage img2;
	img2.setFromPixels(outputImage.getPixels());
	cv::Mat m2 = ofxCv::toCv(img2.getPixels());
	cv::OutputArray outputArr = cv::OutputArray(m2);
	
	double angleArray[8];

	int k = 0;
	//até 360
	for(int i = 0; i < 8; i++){
	cv::filter2D(inputArr, outputArr,-1,cv::getGaborKernel(cv::Size(32,32),20.00,k/180 * M_PI,40.00,0.50,0.00));
	cv::Mat m3 = outputArr.getMat();

	int sum = 0;
	int nRows = m3.rows;
	int nCols = m3.cols;
	int matSize = nCols * nRows;

	//para cada linha, para cada coluna (-1, porque nrows/ncols começa a 1, enquanto na matriz começam a zero)
	//somar o valor do elemento no i, j
	//m3.at<int>(i,j);


	//proabably guardar imagem para interesse?
	m3.at<int>(0,0);
	angleArray[i] = 
		//média de todos 
	}
	return angleArray;
	//fazer média e contagem

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
