#include <algorithm>

#include "ofApp.h"

#ifndef CODE_ANALYSIS
#include "ofxCv.h"
#include "ofxOpenCv.h"
#endif

using cv::Mat;

#define M_PI 3.14159265358979323846
#define ESCAPESPEED 100
#define SCALESPEED 1
#define VIDBARALPHA     128
#define VIDBARFADESPEED 8
#define VIDBUTTONSIZE   64
#define VIDBUTTONSPACE  8
#define VIDBUTTONCOUNT  4
#define VIDBUTTONRATIO  (VIDBUTTONSIZE*VIDBUTTONCOUNT + VIDBUTTONSPACE*(VIDBUTTONCOUNT+1))

const double kDistanceCoef = 4.0;
const int kMaxMatchingSize = 50;
const int kMinMatchingSize = 3;

//--------------------------------------------------------------
void ofApp::setup() {
	//set up the haar finder
	ofxCvHaarFinder hF = ofxCvHaarFinder();
	hF.setup("haarcascade_frontalface_default.xml");
	ofSetVerticalSync(true);

	// Initialize the music player button icons
	vidplayer_playbutton.load("Play.png");
	vidplayer_pausebutton.load("Pause.png");
	vidplayer_forwardbutton.load("Forward.png");
	vidplayer_backwardbutton.load("Back.png");
	vidplayer_soundbutton.load("Sound.png");
	vidplayer_mutebutton.load("Mute.png");

	// Get the images from the given directory
	dir.listDir("images/of_logos/");
	dir.allowExt("jpg"); 
	dir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

	// Allocate the vector to have as many ThumbObject as files
	if (dir.size()) {
		images.resize(dir.size());
	}

	// you can now iterate through the files and load them into the ofImage vector
	for (size_t i=0; i<images.size(); i++) 
	{
		ThumbObject* img = new ThumbObject(dir.getPath(i), std::rand()%ofGetWindowWidth(), std::rand()%ofGetWindowHeight());
		if (img->GetThumbType() == None)
		{
			delete img;
			continue;
		}
		if (img->GetMaxSize().x > appsize.x/2 || img->GetMaxSize().y > appsize.y/2)
		{
			float ratio = img->GetMaxSize().x / img->GetMaxSize().y;
			img->SetMaxSize({appsize.x/2, appsize.y/2*ratio});
		}
		if (img->GetThumbType() == Video)
		{
			ofVideoPlayer* vid = img->GetVideo();
			double vidCut = vidDetectCut(vid);
		}
		images[imagecount++] = img;
	}
	selectedImage = NULL;
	highlightedImage = NULL;
	vidplayer_alpha = 0;

	// Draw the background
	ofBackground(ofColor::black);
}

//--------------------------------------------------------------
void ofApp::update() {
	for (int i=0; i<imagecount; i++) {

		// Initialize a bunch of helper variables
		ThumbObject* img = images[i];
		Vector2D size = img->GetSize();
		vector<ThumbObject*> collisions;
		int colcount = 0;
		collisions.resize(imagecount);

		// Check for overlaps
		for (int j=0; j<imagecount; j++) {

			// Skip ourselves
			if (i == j)
				continue;

			// If an image overlaps, add it to our list of collisions
			if (img->IsOverlapping(images[j]))
				collisions[colcount++] = images[j];
		}

		// If we have collisions
		if (colcount > 0)
		{
			// Iterate through all collided objects
			for (int j=0; j<colcount; j++)
			{
				// Skip the current selected image
				if (highlightedImage == collisions[j])
					continue;

				// Push the object we're colliding with away
				Vector2D center1 = img->GetPos() + img->GetSize()/2;
				Vector2D center2 = collisions[j]->GetPos() + collisions[j]->GetSize()/2;
				Vector2D escape = center2 - center1;
				float distance = sqrtf((center2.x-center1.x)*(center2.x-center1.x) + (center2.y-center1.y)*(center2.y-center1.y));
				escape = escape / sqrtf(escape.x*escape.x + escape.y*escape.y);
				escape = escape*ESCAPESPEED*(1/MAX(1, distance));
				escape = collisions[j]->GetPos() + escape;
				escape = {MAX(0, MIN(escape.x, ofGetWindowWidth() - img->GetSize().x)), MAX(0, MIN(escape.y, ofGetWindowHeight() - img->GetSize().y))};
				collisions[j]->SetPos(escape);

				// Scale down the largest image if we're still overlapping
				if (img->IsOverlapping(collisions[j]))
				{
					ThumbObject* largest = img;
					if (img->IsOverlapping(collisions[j]) && largest->GetSize() < images[j]->GetSize())
						largest = images[j];
					if (highlightedImage != largest && largest->GetSize() > largest->GetMinSize())
						largest->SetSize(largest->GetSize() - SCALESPEED);
				}
			}
		}
		
		// Enlarge the image if it is not colliding with anything
		if ((colcount == 0 || img == highlightedImage) && img->GetSize() < img->GetMaxSize())
			img->SetSize(img->GetSize() + SCALESPEED);

		// Don't allow the image to go out of bounds
		img->SetPos(MAX(0, MIN(img->GetPos().x, ofGetWindowWidth() - img->GetSize().x)), MAX(0, MIN(img->GetPos().y, ofGetWindowHeight() - img->GetSize().y)));

		// Clear the memory used by the vector
		collisions.clear();
		
		// Call the update function on the image (to play video)
		img->update();
	}

	// Handle video player buttons alpha
	if (highlightedImage != NULL && highlightedImage->GetThumbType() == Video)
	{
		if (highlightedImage->IsOverlapping(ofGetMouseX(), ofGetMouseY()))
		{
			if (vidplayer_alpha < VIDBARALPHA)
				vidplayer_alpha += VIDBARFADESPEED;
		}
		else
		{
			if (vidplayer_alpha > 0)
				vidplayer_alpha -= VIDBARFADESPEED;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	ofEnableAlphaBlending();
	if (imagecount > 0) {
		ofSetColor(ofColor::white);

		// Draw all the images
		for (int i=imagecount-1; i>=0; i--) {
			ThumbObject* img = images[i];
			Vector2D pos = img->GetPos();
			Vector2D size = img->GetSize();
			switch (img->GetThumbType())
			{
				case Image:
					img->GetImage()->draw(pos.x, pos.y, size.x, size.y);
					break;
				case GIF:
					img->GetGIF()->draw(pos.x, pos.y, size.x, size.y);
					break;
				case Video:
					img->GetVideo()->draw(pos.x, pos.y, size.x, size.y);
					break;
			}
		}

		// Draw the video player buttons
		if (highlightedImage != NULL && highlightedImage->GetThumbType() == Video)
		{
			float ratio = (highlightedImage->GetSize().x/VIDBUTTONRATIO);
			float butsize = VIDBUTTONSIZE*ratio;
			float butbufsize = (VIDBUTTONSIZE+VIDBUTTONSPACE)*ratio;
			float buffer = VIDBUTTONSPACE*ratio;
			float bufferh = VIDBUTTONSPACE*ratio/2;
			Vector2D pos = highlightedImage->GetPos();
			Vector2D size = highlightedImage->GetSize();
			ofImage* buttons[VIDBUTTONCOUNT];
			float buty = pos.y+size.y-butbufsize;

			// Select the buttons to show
			buttons[0] = &vidplayer_backwardbutton;
			buttons[1] = highlightedImage->GetVideoPlaying() ? &vidplayer_pausebutton : &vidplayer_playbutton;
			buttons[2] = &vidplayer_forwardbutton;
			buttons[3] = highlightedImage->GetVideoMuted() ? &vidplayer_mutebutton : &vidplayer_soundbutton;

			// Draw the button tray
			ofSetColor(0, 0, 0, vidplayer_alpha);
			ofDrawRectangle(pos.x, pos.y+size.y-butbufsize, size.x, butbufsize);

			// Draw the buttons
			for (int j=0; j<VIDBUTTONCOUNT; j++)
			{
				float butx = pos.x+(butbufsize)*j+buffer;
				if (ofGetMouseY() >= buty && ofGetMouseY() <= buty+butbufsize && ofGetMouseX() >= butx && ofGetMouseX() <= butx+butsize)
					ofSetColor(255, 255, 255, 255);
				else
					ofSetColor(255, 255, 255, vidplayer_alpha);
				buttons[j]->draw(butx, buty+bufferh, butsize, butsize);
			}

			// Reset drawing values
			ofSetColor(ofColor::white);
		}
		ofDisableAlphaBlending();

		/*
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
		*/
	}

	/*
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
	*/
}

float luminanceFunction(ofColor color) {
	return (0.2126 * (float)color.r + 0.7152 * (float)color.g + 0.0722 * (float)color.b);
}

float ofApp::calculateLuminance(int image) {
	ofImage* currentImage = images[image]->GetImage();
	ofPixels imagePixels = currentImage->getPixels();
	float sumLuminance = 0;
	size_t vectorSize = imagePixels.getWidth() * imagePixels.getHeight();
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


	ofImage* currentImage = images[image]->GetImage();
	ofPixels imagePixels = currentImage->getPixels();
	m_col sumColor = { 0, 0, 0 };
	float vectorSize = imagePixels.getWidth() * imagePixels.getHeight();
	if (vectorSize > 0) {
		for (uint32_t y = 0; y < imagePixels.getHeight(); y++) {
			for (uint32_t x = 0; x < imagePixels.getWidth(); x++) {
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
	ofImage currentImage = *images[image]->GetImage();
	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	currentImage.setImageType(OF_IMAGE_GRAYSCALE);
	//the image's pixels
	ofPixels imagePixels = currentImage.getPixels();
	//the image we're going to use as the basis for the output images throughout the cycle
	ofImage outputImage = currentImage;

	//the input image matrix
	ofxCvGrayscaleImage img;
	img.setFromPixels(imagePixels);
	cv::Mat m = ofxCv::toCv(img.getPixels());
	cv::InputArray inputArr (m);
	
	//the output images maxtices
	ofxCvGrayscaleImage img2;
	img2.setFromPixels(outputImage.getPixels());

	cv::Mat m2 = ofxCv::toCv(img2.getPixels());
	cv::OutputArray outputArr (m2);

	//at� 360, 8 vezes, 360/8 = 45
	int k = 22.5;
	
	//8 times, each 45 degrees, we apply the gaborfilter to the image, and place the result on the output matrix
	for(int i = 0; i < 8; i++){
		double rads = (i * k) / 180.0 * M_PI;
	cv::filter2D(inputArr, outputArr,-1,cv::getGaborKernel(cv::Size(32,32),20.00,rads,20.00,0.50,0.00));	//when i = 0, angle is 0, when i = 1, angle is 45, etc etc
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
			//somar o valor � soma total
			sum += m3.at<unsigned char>(j, k);

			//converter em imagem para podermos guardar
			
		}
	}
	//aplicar a m�dia
	sum = sum / matSize;

	ofxCv::toOf(m3, outputImage);
	string direc = dir.getPath(image) + "angle-" + to_string(i);

	//outputImage.save("test"+ to_string(i)+".jpg");

	//guardar a m�dia na posi��o adequada do array
	
	avgArray[i] = sum;
		
	}

	//return the array
	return avgArray;
	

}

double* ofApp::calculateEdges(int image, double* avgArray) {
	//the image we want to fetch from
	ofImage currentImage = *images[image]->GetImage();
	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	currentImage.setImageType(OF_IMAGE_GRAYSCALE);
	//the image's pixels
	ofPixels imagePixels = currentImage.getPixels();
	//the image we're going to use as the basis for the output images throughout the cycle
	ofImage outputImage = currentImage;

	//the input image matrix
	ofxCvGrayscaleImage img;
	img.setFromPixels(imagePixels);
	cv::Mat m = ofxCv::toCv(img.getPixels());
	cv::InputArray inputArr (m);

	//the edge detection matrices
	float data1[4] = { -1, -1, 1, 1};
	cv::Mat mE1 = cv::Mat(2, 2, CV_32F, data1);
	cv::InputArray inputArrEdge1 (mE1);

	//the edge detection matrices
	float data2[4] = { -1, 1, -1, 1 };
	cv::Mat mE2 = cv::Mat(2, 2, CV_32F, data2);
	cv::InputArray inputArrEdge2 (mE2);

	//the edge detection matrices
	float data3[4] = { 0, 1, -1, 0 };
	cv::Mat mE3 = cv::Mat(2, 2, CV_32F, data3);
	cv::InputArray inputArrEdge3 (mE3);

	//the edge detection matrices
	float data4[4] = { -1, 0, 0, 1 };
	cv::Mat mE4 = cv::Mat(2, 2, CV_32F, data4);
	cv::InputArray inputArrEdge4 (mE4);

	//the output images matrices
	ofxCvGrayscaleImage img2;
	img2.setFromPixels(outputImage.getPixels());
	cv::Mat m2 = ofxCv::toCv(img2.getPixels());
	cv::OutputArray outputArr (m2);

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
				//somar o valor � soma total
				sum += m3.at<unsigned char>(j, k);


			}
		}
		//aplicar a m�dia
		sum = sum / matSize;

		//converter em imagem para podermos guardar
		ofxCv::toOf(m3, outputImage);
		outputImage.save("toast" + to_string(i) + ".jpg");

		//guardar a m�dia na posi��o adequada do array

		avgArray[i] = sum;

	}
	//return the array
	return avgArray;


}

double ofApp::vidDetectCut(ofVideoPlayer * vid) {
	double cutTotal = 0;
	int fCounter = 0;

	vid->firstFrame();

	while (fCounter < vid->getTotalNumFrames()) {
		ofPixels currFrame = vid->getPixels();

		fCounter+=5;
		vid->nextFrame();
		vid->nextFrame();
		vid->nextFrame();
		vid->nextFrame();
		vid->nextFrame();

		ofPixels nextFrame = currFrame;
		if (fCounter < vid->getTotalNumFrames()) {

			nextFrame = vid->getPixels();
		}

		double val = detectCut(currFrame, nextFrame);
		cutTotal += val;

	}
	return (cutTotal) / (fCounter * vid->getHeight() * vid->getWidth());
}

//dividir pelo n�mero de frames e pelo tamanho de cada frame
double ofApp::detectCut(ofPixels image1Of, ofPixels image2Of) {


	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	image1Of.setImageType(OF_IMAGE_GRAYSCALE);

	//the input image matrix
	ofxCvGrayscaleImage img1;
	img1.setFromPixels(image1Of);
	cv::Mat m1 = ofxCv::toCv(img1.getPixels());


	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	image2Of.setImageType(OF_IMAGE_GRAYSCALE);

	//the input image matrix
	ofxCvGrayscaleImage img2;
	img2.setFromPixels(image2Of);
	cv::Mat m2 = ofxCv::toCv(img2.getPixels());


	cv::MatND hist1;
	cv::MatND hist2;

	int channels[] = { 0 };
	int lbins = 256;
	int histSize[] = { lbins };
	float lranges[] = { 0, 256 };
	const float* ranges[] = { lranges };
	cv::calcHist(&m1, 1, channels, cv::Mat(), // do not use mask
		hist1, 1, histSize, ranges,
		true, // the histogram is uniform
		false);
	double maxVal1 = 0;
	cv::minMaxLoc(hist1, 0, &maxVal1, 0, 0);

	cv::calcHist(&m2, 1, channels, cv::Mat(), // do not use mask
		hist2, 1, histSize, ranges,
		true, // the histogram is uniform
		false);
	double maxVal2 = 0;
	cv::minMaxLoc(hist2, 0, &maxVal2, 0, 0);

	//calcular as difs entre os dois histogramas com cv::compareHist(), em que o method � a constante "HISTCMP_CHISQR "
	double comparison = cv::compareHist(hist1, hist2, cv::HISTCMP_CHISQR);
	// Needs to return something to compile
	return comparison;
}

//using orb to check out if two images have similar descriptors
bool ofApp::detectMatchingFeatures(int image1, int image2) {
	//the first image we want to fetch from
	ofImage* image1Of = images[image1]->GetImage();
	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	image1Of->setImageType(OF_IMAGE_GRAYSCALE);
	//the image's pixels
	ofPixels image1Pixels = image1Of->getPixels();
	//the input image matrix
	ofxCvGrayscaleImage img1;
	img1.setFromPixels(image1Pixels);
	cv::Mat m1 = ofxCv::toCv(img1.getPixels());

	//the second image we want to fetch from
	ofImage* image2Of = images[image2]->GetImage();
	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	image2Of->setImageType(OF_IMAGE_GRAYSCALE);
	//the image's pixels
	ofPixels image2Pixels = image2Of->getPixels();
	//the input image matrix
	ofxCvGrayscaleImage img2;
	img2.setFromPixels(image2Pixels);
	cv::Mat m2 = ofxCv::toCv(img2.getPixels());

	vector<cv::KeyPoint> kpts1;
	vector<cv::KeyPoint> kpts2;

	Mat desc1;
	Mat desc2;

	vector<cv::DMatch> matches;

	//this will detect feautres in both images
	detectAndCompute(m1, kpts1, desc1);
	detectAndCompute(m2, kpts1, desc2);

	//now we are going to compare them to see if we find any matches
	match(desc1, desc2, matches);

	//now, we will apply a match mask
	vector<char> match_mask(matches.size(), 1);

	int counterObjects = 0;

	for (int i = 0; i < matches.size();i++) {
		if (matches[i].distance < kDistanceCoef) {
			counterObjects++;
		}
	}

	return (counterObjects >= kMinMatchingSize);
}

 void ofApp::detectAndCompute(cv::Mat& img, vector<cv::KeyPoint>& kpts, cv::Mat& desc) {

		cv::Ptr<cv::ORB> orb = cv::ORB::create();
		orb->detectAndCompute(img, Mat(), kpts, desc);
}

void ofApp::match(cv::Mat& desc1, cv::Mat& desc2, vector<cv::DMatch>& matches) {
	 matches.clear();
		 cv::BFMatcher desc_matcher(cv::NORM_L2, true);
		 desc_matcher.match(desc1, desc2, matches, Mat());
	 std::sort(matches.begin(), matches.end());
	 while (matches.front().distance * kDistanceCoef < matches.back().distance) {
		 matches.pop_back();
	 }
	 while (matches.size() > kMaxMatchingSize) {
		 matches.pop_back();
	 }
 }

//haar face detection
int ofApp::haarFaces(int image, ofxCvHaarFinder hF) {
	ofImage* currentImage = images[image]->GetImage();

	hF.findHaarObjects(*currentImage);

	//this will store it as blobs, now we need to count them
	int count = 0;
	for (uint32_t i = 0; i < hF.blobs.size(); i++) {
		count++;
	}
	return count;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	/*
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
	*/
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (button == OF_MOUSE_BUTTON_LEFT)
	{
		if (selectedImage != NULL)
		{
			Vector2D size = selectedImage->GetSize();
			Vector2D gpos = selectedImage->GetGrabbedPosition();
			x = MAX(0, MIN(x-gpos.x, ofGetWindowWidth() - size.x));
			y = MAX(0, MIN(y-gpos.y, ofGetWindowHeight() - size.y));
			selectedImage->SetPos(x, y);
		}
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	if (button == OF_MOUSE_BUTTON_LEFT)
	{
		bool selected = false;

		// Handle the highlighted picture first
		if (highlightedImage != NULL && highlightedImage->IsOverlapping(x, y))
		{
			selectedImage = highlightedImage;
			selectedImage->SetGrabbedPosition(x-highlightedImage->GetPos().x, y-highlightedImage->GetPos().y);

			// Handle video player controls
			if (highlightedImage->GetThumbType() == Video)
			{
				float ratio = (highlightedImage->GetSize().x/VIDBUTTONRATIO);
				float butsize = VIDBUTTONSIZE*ratio;
				float butbufsize = (VIDBUTTONSIZE+VIDBUTTONSPACE)*ratio;
				float buffer = VIDBUTTONSPACE*ratio;
				Vector2D pos = highlightedImage->GetPos();
				Vector2D size = highlightedImage->GetSize();
				float buty = pos.y+size.y-butbufsize;
				if (y >= buty && y <= buty+butbufsize)
				{
					for (int i=0; i<VIDBUTTONCOUNT; i++)
					{
						float butx = pos.x+(butbufsize)*i+buffer;
						if (x >= butx && x <= butx+butsize)
						{
							switch (i)
							{
								case 0:
									highlightedImage->AdvanceVideo(-1);
									break;
								case 1:
									highlightedImage->SetVideoPlaying(!highlightedImage->GetVideoPlaying());
									break;
								case 2:
									highlightedImage->AdvanceVideo(1);
									break;
								case 3:
									highlightedImage->SetVideoMuted(!highlightedImage->GetVideoMuted());
									break;
							}
						}
					}
				}
			}

			return;
		}

		// Otherwise, check which image we pressed
		for (int i=0; i<imagecount; i++)
		{
			ThumbObject* img = images[i];
			Vector2D pos = img->GetPos();
			if (img->IsOverlapping(x, y))
			{
				selectedImage = img;
				selectedImage->SetGrabbedPosition(x-pos.x, y-pos.y);
				highlightedImage = img;
				images[i] = images[0];
				images[0] = img;
				selected = true;
				vidplayer_alpha = 0;
				break;
			}
		}

		// If nothing was selected, remove the highlighted image
		if (!selected)
			highlightedImage = NULL;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	if (button == OF_MOUSE_BUTTON_LEFT)
		selectedImage = NULL;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	appsize = {(float)w, (float)h};
	for (int i=0; i<imagecount; i++)
	{
		float ratio = images[i]->GetMaxSize().x / images[i]->GetMaxSize().y;
		images[i]->SetMaxSize({appsize.x/2, appsize.y/2*ratio});
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
