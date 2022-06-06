#include <algorithm>

#include "ofApp.h"

#ifndef CODE_ANALYSIS
#include "ofxCv.h"
#include "ofxOpenCv.h"
#endif

#define TESTING true

#define ESCAPESPEED 100
#define SCALESPEED 1

#define VIDBARALPHA     128
#define VIDBARFADESPEED 8
#define VIDBUTTONSIZE   64
#define VIDBUTTONSPACE  8
#define VIDBUTTONCOUNT  4
#define VIDBUTTONRATIO  (VIDBUTTONSIZE*VIDBUTTONCOUNT + VIDBUTTONSPACE*(VIDBUTTONCOUNT+1))

#define CONTEXTFONT      12
#define CONTEXTWIDTH     128
#define CONTEXTBUTCOUNT  4
#define CONTEXTBUTHEIGHT 24


//--------------------------------------------------------------
void ofApp::setup() {
	// Set up the haar finder
	hF = ofxCvHaarFinder();
	hF.setup("haarcascade_frontalface_default.xml");

	// Initialize variables
	imagecount = 0;
	selectedImage = NULL;
	highlightedImage = NULL;
	vidplayer_alpha = 0;
	generatingmeta = false;
	contextopen = false;

	// Initialize the video player button icons
	vidplayer_playbutton.load("Play.png");
	vidplayer_pausebutton.load("Pause.png");
	vidplayer_forwardbutton.load("Forward.png");
	vidplayer_backwardbutton.load("Back.png");
	vidplayer_soundbutton.load("Sound.png");
	vidplayer_mutebutton.load("Mute.png");

	// Initialize the rest of the app
	ofSetVerticalSync(true);
	ofBackground(ofColor::black);

	// Load the test directory
	if (TESTING)
		this->loadDirectory("images/of_logos/");
}

//--------------------------------------------------------------
void ofApp::update() {

	// Iterate through all loaded thumbs
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
						largest->SetSize(largest->GetSize().x - SCALESPEED, largest->GetSize().y - SCALESPEED/largest->GetSizeRatio());
				}
			}
		}
		
		// Enlarge the image if it is not colliding with anything
		if ((colcount == 0 || img == highlightedImage) && img->GetSize() < img->GetMaxSize())
			img->SetSize(img->GetSize().x + SCALESPEED, img->GetSize().y + SCALESPEED/img->GetSizeRatio());

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
	}

	// Show how to open a directory if none is open, otherwise show the filter buttons
	if (imagecount == 0)
	{
		ofTrueTypeFont font;
		string str = "Press CTRL + O to open a directory with images/video";
		if (generatingmeta)
			str = "Generating metadata";

		ofSetColor(ofColor::white);
		font.load(OF_TTF_SANS, 24);
		font.drawString(str, ofGetWindowWidth()/2 - font.stringWidth(str)/2, ofGetWindowHeight()/2);
	}
	else
	{
		// Filter buttons
	}
}

//--------------------------------------------------------------
void ofApp::loadDirectory(string directory)
{

	printf("Reading directory '");
	cout << directory;
	printf("'\n");

	// Clean up memory used by any old images
	if (imagecount != 0)
	{
		for (size_t i=0; i<imagecount; i++)
		{
			if (images[i]->GetThumbType() == Image)
				delete images[i]->GetImage();
			else if (images[i]->GetThumbType() == Video)
				delete images[i]->GetVideo();
			else
				delete images[i]->GetGIF();
			delete images[i];
			images[i] = NULL;
		}
		dir.close();
	}

	// Reset variables
	imagecount = 0;
	selectedImage = NULL;
	highlightedImage = NULL;
	vidplayer_alpha = 0;

	// Get the images from the given directory
	dir.listDir(directory);
	dir.sort(); // In Linux, the file system doesn't return file lists ordered in alphabetical order

	// If there are no images, stop.
	if (dir.size() == 0)
		return;

	// Allocate the vector to have as many ThumbObject as files
	images.resize(dir.size());

	// Check if the metadata folder exists, and if not, create it
	if (!dir.doesDirectoryExist(directory+".artwall"))
		dir.createDirectory(directory+".artwall");

	// you can now iterate through the files and load them into the ofImage vector
	for (size_t i=0; i<images.size(); i++) 
	{
		ofxXmlSettings metadata;
		ThumbObject* img = new ThumbObject(dir.getPath(i), std::rand()%ofGetWindowWidth(), std::rand()%ofGetWindowHeight());
		if (img->GetThumbType() == None)
		{
			delete img;
			continue;
		}
		
		// Set the max size of the thumb
		if (appsize.x > appsize.y)
			img->SetMaxSize({appsize.y/2, (appsize.y/2)/img->GetSizeRatio()});
		else
			img->SetMaxSize({appsize.x/2, (appsize.x/2)/img->GetSizeRatio()});

		// Load the thumb's metadata
		metadata.loadFile(directory+".artwall/"+dir.getName(i)+"_metadata.xml");
		this->GenerateMetadata(&metadata, img);
		metadata.saveFile(directory+".artwall/"+dir.getName(i)+"_metadata.xml");
		img->LoadMetadata(&metadata);

		if (img->GetThumbType() == Video)
		{
			ofVideoPlayer* vid = img->GetVideo();
			//double vidCut = vidDetectCut(vid);
		}
		images[imagecount++] = img;
	}
	printf("Loaded %d images.\n\n", imagecount);
}

void ofApp::GenerateMetadata(ofxXmlSettings* metadata, ThumbObject* img)
{
	if (!metadata->tagExists("metadata"))
	{
		generatingmeta = true;
		metadata->addTag("metadata");
		printf("Generating metadata\n");
	}
	metadata->pushTag("metadata");

	// Initialize the tags as empty
	if (!metadata->tagExists("tags"))
	{
		generatingmeta = true;
		metadata->addTag("tags");
		//metadata.addValue("tag", "value");
		metadata->pushTag("tags");
		metadata->popTag();
	}

	// Add the luminance tag
	if (!metadata->tagExists("luminance"))
	{
		float luminance = 0;
		generatingmeta = true;
		printf("Calculating luminance\n");
		switch (img->GetThumbType())
		{
			case Video:
				luminance = calculateLuminance(&((ofVideoPlayer*)img->GetVideo())->getPixels());
				break;
			case GIF:
				luminance = calculateLuminance(&((ofImage*)img->GetGIF())->getPixels());
				break;
			case Image:
				luminance = calculateLuminance(&((ofImage*)img->GetImage())->getPixels());
				break;
		}
		metadata->addValue("luminance", luminance);
	}

	// Add the color tag
	if (!metadata->tagExists("color"))
	{
		m_col color = {0, 0, 0};
		generatingmeta = true;
		printf("Calculating color\n");
		switch (img->GetThumbType())
		{
			case Video:
				color = calculateColor(&((ofVideoPlayer*)img->GetVideo())->getPixels());
				break;
			case GIF:
				color = calculateColor(&((ofImage*)img->GetGIF())->getPixels());
				break;
			case Image:
				color = calculateColor(&((ofImage*)img->GetImage())->getPixels());
				break;
		}
		metadata->addTag("color");
		metadata->pushTag("color");
		metadata->addValue("red", color.red);
		metadata->addValue("green", color.green);
		metadata->addValue("blue", color.blue);
		metadata->popTag();
	}

	// Add the number of faces tag
	if (!metadata->tagExists("faces"))
	{
		ofImage* frame;
		int faces = 0;
		generatingmeta = true;
		printf("Calculating faces\n");
		switch (img->GetThumbType())
		{
			case Video:
				frame = new ofImage();
				frame->setFromPixels(((ofVideoPlayer*)img->GetVideo())->getPixels());
				faces = haarFaces(*frame, &hF);
				delete frame;
				break;
			case GIF:
				faces = haarFaces(*img->GetGIF(), &hF);
				break;
			case Image:
				faces = haarFaces(*img->GetImage(), &hF);
				break;
		}
		metadata->addValue("faces", faces);
	}

	// Add the intensity of each edge image tag
	if (!metadata->tagExists("edges"))
	{
		ofImage* frame;
		double edge[4] = {};
		generatingmeta = true;
		printf("Detecting edges\n");
		switch (img->GetThumbType())
		{
		case Video:
			frame = new ofImage();
			frame->setFromPixels(((ofVideoPlayer*)img->GetVideo())->getPixels());
			calculateEdges(*frame, edge);
			delete frame;
			break;
		case GIF:
			calculateEdges(*img->GetGIF(), edge);
			break;
		case Image:
			calculateEdges(*img->GetImage(), edge);
			break;
		}
		metadata->addTag("edges");
		metadata->pushTag("edges");
		for (int i = 0; i < 4; i++)
			metadata->addValue("value", edge[i]);
		metadata->popTag();
	}

	// Add the thumbnail positions tag
	if (!metadata->tagExists("cuts"))
	{
		double threshold = 0.5;
		std::vector<double>* cuts;

		printf("Detecting video cuts\n");

		metadata->addTag("cuts");
		metadata->pushTag("cuts");
		switch (img->GetThumbType())
		{
		case Video:
			cuts = vidDetectCut((ofVideoPlayer*)img->GetVideo(), threshold);
			for (int i = 0; i < cuts->size(); i++)
				metadata->addValue("value", cuts->at(i));
			delete cuts;
			break;
		case GIF:
			metadata->addValue("value", 0);
			break;
		case Image:
			metadata->addValue("value", 0);
			break;
		}
		metadata->popTag();
	}

	generatingmeta = false;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (ofGetKeyPressed(OF_KEY_CONTROL))
	{
		switch (key)
		{
			case 'O':
			case 'o':
			case 0x0F:
				ofFileDialogResult result = ofSystemLoadDialog("Load folder", true);
				if (result.bSuccess)
					this->loadDirectory(result.getPath()+"/");
				break;
		}
	}
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

		// Handle context menu first
		if (contextopen && x >= contextpos.x && y >= contextpos.y && x <= contextpos.x + CONTEXTWIDTH && y <= contextpos.y + CONTEXTBUTCOUNT*CONTEXTBUTHEIGHT)
		{

		}
		else
			contextopen = false;

		// Handle the highlighted picture next
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

			// No need to look into the other images
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
				if (highlightedImage != NULL && highlightedImage->GetThumbType() == Video)
					highlightedImage->SetVideoPlaying(false);
				highlightedImage = img;
				if (highlightedImage->GetThumbType() == Video)
					((ofVideoPlayer*)highlightedImage->GetVideo())->setPosition(0);
				images[i] = images[0];
				images[0] = img;
				selected = true;
				vidplayer_alpha = 0;
				break;
			}
		}

		// If nothing was selected, remove the highlighted image
		if (!selected)
		{
			if (highlightedImage != NULL && highlightedImage->GetThumbType() == Video)
				highlightedImage->SetVideoPlaying(false);
			highlightedImage = NULL;
		}
	}

	if (button == OF_MOUSE_BUTTON_RIGHT)
	{
		// Check which image we pressed
		for (int i=0; i<imagecount; i++)
		{
			ThumbObject* img = images[i];
			if (img->IsOverlapping(x, y))
			{
				contextpos = {(float)x, (float)y};
				contextopen = true;
				contextobject = img;
				break;
			}
		}
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
		if (appsize.x > appsize.y)
			images[i]->SetMaxSize({appsize.y/2, (appsize.y/2)/images[i]->GetSizeRatio()});
		else
			images[i]->SetMaxSize({appsize.x/2, (appsize.x/2)/images[i]->GetSizeRatio()});
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
