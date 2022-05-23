#include "ofApp.h"

#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include "ofxCv.h"
#include "ofxOpenCv.h"
#pragma warning( pop )

#define M_PI 3.14159265358979323846
//--------------------------------------------------------------
void ofApp::setup() {
	//set up the haar finder
	ofxCvHaarFinder hF = ofxCvHaarFinder();
	hF.setup("haarcascade_frontalface_default.xml");


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

		double gabor[8] = {};
		calculateGabor(currentImage, gabor);
		double edge[4] = {};
		calculateEdges(currentImage, edge);

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

double* ofApp :: calculateGabor(int image, double* avgArray) {
	//the image we want to fetch from
	ofImage currentImage = images[image];
	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	currentImage.setImageType(OF_IMAGE_GRAYSCALE);
	//the image's pixels
	ofPixels imagePixels = currentImage.getPixels();
	//the image we're going to use as the basis for the output images throughout the cycle
	ofImage outputImage = currentImage;

	//the input image matrix
	ofxCvColorImage img;
	img.setFromPixels(imagePixels);
	cv::Mat m = ofxCv::toCv(img.getPixels());
	cv::InputArray inputArr = cv::InputArray(m);
	
	//the output images maxtices
	ofxCvColorImage img2;
	img2.setFromPixels(outputImage.getPixels());
	cv::Mat m2 = ofxCv::toCv(img2.getPixels());
	cv::OutputArray outputArr = cv::OutputArray(m2);
	
	//this array contains all averages of the values of the image when applied a certain angle of the Gabor filter

	//até 360, 8 vezes, 360/8 = 45
	int k = 45;
	
	//8 times, each 45 degrees, we apply the gaborfilter to the image, and place the result on the output matrix
	for(int i = 0; i < 8; i++){
	cv::filter2D(inputArr, outputArr,-1,cv::getGaborKernel(cv::Size(32,32),20.00,(i)*k/180 * M_PI,40.00,0.50,0.00));	//when i = 0, angle is 0, when i = 1, angle is 45, etc etc
	cv::Mat m3 = outputArr.getMat();

	//the sum of all pixel values of the image
	int sum = 0;

	//number of rows and cols, to speed up access
	int nRows = m3.rows;
	int nCols = m3.cols;

	//the size of the matrix as w hole, so we can apply the average to it
	int matSize = nCols * nRows;

	//para cada linha, para cada coluna
	//somar o valor do elemento no i, j
	//m3.at<int>(i,j);
	for (int j = 0; j < nRows; j++) {
		for (int k = 0; k < nCols; k++) {
			//somar o valor à soma total
			sum += m3.at<int>(j, k);

			//converter em imagem para podermos guardar
			ofxCv::toOf(m3, outputImage);
			outputImage.save(dir.getPath(image) + "angle-" + to_string(i));
		}
	}
	//aplicar a média
	sum = sum / matSize;


	//guardar a média na posição adequada do array
	
	avgArray[i] = sum;
		
	}
	//return the array
	return avgArray;
	

}

double* ofApp::calculateEdges(int image, double* avgArray) {
	//the image we want to fetch from
	ofImage currentImage = images[image];
	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	currentImage.setImageType(OF_IMAGE_GRAYSCALE);
	//the image's pixels
	ofPixels imagePixels = currentImage.getPixels();
	//the image we're going to use as the basis for the output images throughout the cycle
	ofImage outputImage = currentImage;

	//the input image matrix
	ofxCvColorImage img;
	img.setFromPixels(imagePixels);
	cv::Mat m = ofxCv::toCv(img.getPixels());
	cv::InputArray inputArr = cv::InputArray(m);

	//the edge detection matrices
	float data1[4] = { -1, -1, 1, 1};
	cv::Mat mE1 = cv::Mat(2, 2, CV_32F, data1);
	cv::InputArray inputArrEdge1 = cv::InputArray(mE1);

	//the edge detection matrices
	float data2[4] = { -1, 1, -1, 1 };
	cv::Mat mE2 = cv::Mat(2, 2, CV_32F, data2);
	cv::InputArray inputArrEdge2 = cv::InputArray(mE2);

	//the edge detection matrices
	float data3[4] = { 0, 1, -1, 0 };
	cv::Mat mE3 = cv::Mat(2, 2, CV_32F, data3);
	cv::InputArray inputArrEdge3 = cv::InputArray(mE3);

	//the edge detection matrices
	float data4[4] = { -1, 0, 0, 1 };
	cv::Mat mE4 = cv::Mat(2, 2, CV_32F, data4);
	cv::InputArray inputArrEdge4 = cv::InputArray(mE4);

	//the output images matrices
	ofxCvColorImage img2;
	img2.setFromPixels(outputImage.getPixels());
	cv::Mat m2 = ofxCv::toCv(img2.getPixels());
	cv::OutputArray outputArr = cv::OutputArray(m2);

	//this array contains all averages of the values of the image when applied a certain angle of the Gabor filter


	//até 360, 8 vezes, 360/8 = 45
	int k = 45;

	//8 times, each 45 degrees, we apply the gaborfilter to the image, and place the result on the output matrix
	for (int i = 0; i < 4; i++) {

		switch (i) {
		case 0:
			cv::filter2D(inputArr, outputArr, -1, inputArrEdge1);	
			break;
		case 1:
			cv::filter2D(inputArr, outputArr, -1, inputArrEdge2);
			break;
		case 2:
			cv::filter2D(inputArr, outputArr, -1, inputArrEdge3);
			break;
		case 3:
			cv::filter2D(inputArr, outputArr, -1, inputArrEdge4);
			break;
		default:
			// code block
			break;
		}


				cv::Mat m3 = outputArr.getMat();

		//the sum of all pixel values of the image
		int sum = 0;

		//number of rows and cols, to speed up access
		int nRows = m3.rows;
		int nCols = m3.cols;

		//the size of the matrix as w hole, so we can apply the average to it
		int matSize = nCols * nRows;

		//para cada linha, para cada coluna
		//somar o valor do elemento no i, j
		//m3.at<int>(i,j);
		for (int j = 0; j < nRows; j++) {
			for (int k = 0; k < nCols; k++) {
				//somar o valor à soma total
				sum += m3.at<int>(j, k);

				//converter em imagem para podermos guardar
				ofxCv::toOf(m3, outputImage);
				outputImage.save(dir.getPath(image) + "angle-" + to_string(i));
			}
		}
		//aplicar a média
		sum = sum / matSize;


		//guardar a média na posição adequada do array

		avgArray[i] = sum;

	}
	//return the array
	return avgArray;


}

int ofApp::haarFaces(int image, ofxCvHaarFinder hF) {
	ofImage currentImage = images[image];

	hF.findHaarObjects(currentImage);

	//this will store it as blobs, now we need to count them
	int count = 0;
	for (int i = 0; i < hF.blobs.size(); i++) {
		count++;
	}
	return count;
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
