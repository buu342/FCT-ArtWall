#ifndef CODE_ANALYSIS
#include "ofxCv.h"
#include "ofxOpenCv.h"
#endif

#include "ofApp.h"

using cv::Mat;

#define M_PI 3.14159265358979323846

const double kDistanceCoef = 4.0;
const int kMaxMatchingSize = 50;
const int kMinMatchingSize = 3;

inline float luminanceFunction(ofColor color) {
	return (0.2126 * (float)color.r + 0.7152 * (float)color.g + 0.0722 * (float)color.b);
}

float ofApp::calculateLuminance(ofPixels* imagePixels) {
	float sumLuminance = 0;
	size_t vectorSize = imagePixels->getWidth() * imagePixels->getHeight();
	if (vectorSize > 0) {
		for (int i = 0; i < vectorSize; i++) {
			ofColor colorAtIndex = imagePixels->getColor(i);
			sumLuminance += luminanceFunction(colorAtIndex);
		}
		sumLuminance = sumLuminance / vectorSize;
	}
	return sumLuminance;
}

m_col ofApp::calculateColor(ofPixels* imagePixels) {
	m_col sumColor = { 0, 0, 0 };
	float vectorSize = imagePixels->getWidth() * imagePixels->getHeight();
	if (vectorSize > 0) {
		for (uint32_t y = 0; y < imagePixels->getHeight(); y++) {
			for (uint32_t x = 0; x < imagePixels->getWidth(); x++) {
				ofColor colorAtIndex = imagePixels->getColor(x, y);
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
	img.allocate(currentImage.getWidth(), currentImage.getHeight());
	img.setFromPixels(imagePixels);
	cv::Mat m = ofxCv::toCv(img.getPixels());
	cv::InputArray inputArr (m);
	
	//the output images maxtices
	ofxCvGrayscaleImage img2;
	img2.allocate(outputImage.getWidth(), outputImage.getHeight());
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

double* ofApp::calculateEdges(ofImage currentImage, double* avgArray) {
	//the image we want to fetch from

	//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
	currentImage.setImageType(OF_IMAGE_GRAYSCALE);
	//the image's pixels
	ofPixels imagePixels = currentImage.getPixels();
	//the image we're going to use as the basis for the output images throughout the cycle
	ofImage outputImage = currentImage;

	//the input image matrix
	ofxCvGrayscaleImage img;
	img.allocate(currentImage.getWidth(), currentImage.getHeight());
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
	img2.allocate(outputImage.getWidth(), outputImage.getHeight());
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

void ofApp::vidThumb(ofVideoPlayer* vid, double *array) {
	int fCounter = 0;

	while (fCounter < 10) {

		array[fCounter] = (fCounter / 10.0);
		fCounter += 1;
	}
}

	std::vector<double>* ofApp::vidDetectCut(ofVideoPlayer * vid, double thresh) {

		int fCounter = 0;
	
		std::vector<double>* cuts = new std::vector<double>();
		cuts->push_back(0);

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

			double val = detectCut(currFrame, nextFrame, vid->getWidth(), vid->getHeight());
			//printf("%lf\n", val);
			if (val > thresh) {
				cuts->push_back(((double)fCounter) / ((double)vid->getTotalNumFrames()));
			}
		}

		return cuts;
	}

	//dividir pelo n�mero de frames e pelo tamanho de cada frame
	double ofApp::detectCut(ofPixels image1Of, ofPixels image2Of, int w, int h) {

		//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
		image1Of.setImageType(OF_IMAGE_GRAYSCALE);

		//the input image matrix
		ofxCvGrayscaleImage img1;
		img1.allocate(w, h);
		img1.setFromPixels(image1Of);
		cv::Mat m1 = ofxCv::toCv(img1.getPixels());


		//we convert it in grayscale so each pixel only has one value, important for performing the calculations further on
		image2Of.setImageType(OF_IMAGE_GRAYSCALE);

		//the input image matrix
		ofxCvGrayscaleImage img2;
		img2.allocate(w, h);
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
	img1.allocate(image1Of->getWidth(), image1Of->getHeight());
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
	img2.allocate(image2Of->getWidth(), image2Of->getHeight());
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
int ofApp::haarFaces(ofImage currentImage, ofxCvHaarFinder* hF) {

	currentImage.setImageType(OF_IMAGE_GRAYSCALE);
	hF->findHaarObjects(currentImage);

	// This will store it as blobs, now we need to count them
	int count = 0;
	for (size_t i = 0; i < hF->blobs.size(); i++)
		count++;
	return count;
}