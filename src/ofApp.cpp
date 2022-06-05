#include <algorithm>

#include "ofApp.h"

#ifndef CODE_ANALYSIS
#include "ofxCv.h"
#include "ofxOpenCv.h"
#endif

#define TESTING true

#define ESCAPESPEED     10.0f
#define SCALESPEED      1.0f
#define SCALESPEEDHIGH  SCALESPEED*4
#define VIDBARALPHA     128
#define VIDBARFADESPEED 8
#define VIDBUTTONSIZE   64
#define VIDBUTTONSPACE  8
#define VIDBUTTONCOUNT  4
#define VIDBUTTONRATIO  (VIDBUTTONSIZE*VIDBUTTONCOUNT + VIDBUTTONSPACE*(VIDBUTTONCOUNT+1))


//--------------------------------------------------------------
void ofApp::setup() {
	// Set up the haar finder
	ofxCvHaarFinder hF = ofxCvHaarFinder();
	hF.setup("haarcascade_frontalface_default.xml");

	// Initialize variables
	imagecount = 0;
	selectedImage = NULL;
	highlightedImage = NULL;
	vidplayer_alpha = 0;

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
		Vector2D pos = img->GetPos();
		Vector2D size = img->GetSize();
		Vector2D sizehalf = img->GetSizeHalf();
		vector<ThumbObject*> collisions;
		int colcount = 0;
		collisions.resize(imagecount);

		// Push the image if it's out of bounds
		if (img != selectedImage)
		{
			Vector2D escape = {0, 0};
			Vector2D magnitude = {0, 0};
			if (pos.x-sizehalf.x < 0)
			{
				escape.x += 1.0f;
				magnitude.x -= pos.x-sizehalf.x;
			}
			if (pos.x+sizehalf.x > appsize.x)
			{
				escape.x -= 1.0f;
				magnitude.x += pos.x+sizehalf.x - appsize.x;
			}
			if (pos.y-sizehalf.y < 0)
			{
				escape.y += 1.0f;
				magnitude.y -= pos.y-sizehalf.y;
			}
			if (pos.y+sizehalf.y > appsize.y)
			{
				escape.y -= 1.0f;
				magnitude.y += pos.y+sizehalf.y - appsize.y;
			}

			// If we have an escape vector, then push the thumb back into bounds
			if (escape.x != 0 || escape.y != 0)
			{
				escape = escape/sqrtf(escape.x*escape.x + escape.y*escape.y);
				escape = escape*ESCAPESPEED*(magnitude/sizehalf);
				img->SetPos(img->GetPos()+escape);
			}
		}

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
				Vector2D center1 = img->GetPos();
				Vector2D center2 = collisions[j]->GetPos();
				Vector2D escape = center2 - center1;
				Vector2D wiggle = {std::rand()%2-1, std::rand()%2-1};

				// Calculate the distance
				Vector2D maxdist = img->GetSizeHalf() + collisions[j]->GetSizeHalf();
				float distance = sqrtf((center2.x-center1.x)*(center2.x-center1.x));
				maxdist = {1-distance/maxdist.x, 1-distance/maxdist.y};

				// Normalize and push
				escape = escape / sqrtf(escape.x*escape.x + escape.y*escape.y);
				escape = escape*ESCAPESPEED*maxdist;
				collisions[j]->SetPos(collisions[j]->GetPos() + escape + wiggle);

				// Scale down the largest image
				ThumbObject* largest = img;
				if (largest == highlightedImage || largest->GetSize() < collisions[j]->GetSize())
				    largest = collisions[j];
				if (largest->GetSize() > largest->GetMinSize())
				    largest->SetSize(largest->GetSize().x - SCALESPEED, largest->GetSize().y - SCALESPEED/largest->GetSizeRatio());
			}
		}

		// Enlarge the image if it is not colliding with anything or it's highlighted
		if (img->GetSize() < img->GetMaxSize())
		{
			if (img == highlightedImage)
				img->SetSize(img->GetSize().x + SCALESPEEDHIGH, img->GetSize().y + SCALESPEEDHIGH/img->GetSizeRatio());
			else if (colcount == 0)
				img->SetSize(img->GetSize().x + SCALESPEED, img->GetSize().y + SCALESPEED/img->GetSizeRatio());
		}

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
			Vector2D sizehalf = img->GetSizeHalf();
			switch (img->GetThumbType())
			{
				case Image:
					img->GetImage()->draw(pos.x-sizehalf.x, pos.y-sizehalf.y, size.x, size.y);
					break;
				case GIF:
					img->GetGIF()->draw(pos.x-sizehalf.x, pos.y-sizehalf.y, size.x, size.y);
					break;
				case Video:
					img->GetVideo()->draw(pos.x-sizehalf.x, pos.y-sizehalf.y, size.x, size.y);
					break;
			}
			ofDrawCircle(pos.x, pos.y, 3);
		}

		// Draw the video player buttons
		if (highlightedImage != NULL && highlightedImage->GetThumbType() == Video)
		{
			float ratio = (highlightedImage->GetSize().x/VIDBUTTONRATIO);
			float butsize = VIDBUTTONSIZE*ratio;
			float butbufsize = (VIDBUTTONSIZE+VIDBUTTONSPACE)*ratio;
			float buffer = VIDBUTTONSPACE*ratio;
			float bufferh = VIDBUTTONSPACE*ratio/2;
			Vector2D pos = highlightedImage->GetPos() - highlightedImage->GetSizeHalf();
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

	// Show how to open a directory if none is open
	if (imagecount == 0)
	{
		ofTrueTypeFont font;
		string str = "Press CTRL + O to open a directory with images/video";

		ofSetColor(ofColor::white);
		font.load(OF_TTF_SANS, 24);
		font.drawString(str, ofGetWindowWidth()/2 - font.stringWidth(str)/2, ofGetWindowHeight()/2);
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

	if (dir.size() == 0)
		return;

	// Allocate the vector to have as many ThumbObject as files
	images.resize(dir.size());

	// you can now iterate through the files and load them into the ofImage vector
	for (size_t i=0; i<images.size(); i++) 
	{
		ThumbObject* img = new ThumbObject(dir.getPath(i), std::rand()%ofGetWindowWidth(), std::rand()%ofGetWindowHeight());
		if (img->GetThumbType() == None)
		{
			delete img;
			continue;
		}
		if (appsize.x > appsize.y)
			img->SetMaxSize({appsize.y/2, (appsize.y/2)/img->GetSizeRatio()});
		else
			img->SetMaxSize({appsize.x/2, (appsize.x/2)/img->GetSizeRatio()});
		if (img->GetThumbType() == Video)
		{
			ofVideoPlayer* vid = img->GetVideo();
			//double vidCut = vidDetectCut(vid);
		}
		images[imagecount++] = img;
	}
	printf("Loaded %d images.\n\n", imagecount);
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
			Vector2D sizehalf = selectedImage->GetSizeHalf();
			Vector2D gpos = selectedImage->GetGrabbedPosition();
			x = x-gpos.x;
			y = y-gpos.y;
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
				Vector2D pos = highlightedImage->GetPos() - highlightedImage->GetSizeHalf();
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