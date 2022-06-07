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

#define CONTEXTBUTTONS   {/*"Open", "Open Folder", */"Metadata"}
#define CONTEXTFONT      12
#define CONTEXTWIDTH     128
#define CONTEXTBUTCOUNT  1
#define CONTEXTBUTHEIGHT 24

#define FILTERSBUTTONS       {"Tag", "Luminance", "Color", " Face\nCount", "     Edge\nDistribution", "Texture",                 "  Scene\nChanges",  "Object\n Match"}
#define FILTERSNAMES         {"Tag", "Luminance", "Color", "Face count",   "Edge distribution",       "Texture characteristics", "Scene changes",     "Object match"}
#define FILTERSFONT          12
#define FILTERSBUTTONSHEIGHT 48


//--------------------------------------------------------------
void ofApp::setup() {
	string filters[] = FILTERSBUTTONS;
	float ratio = appsize.x/DEFAULTAPPW;
	filtersheight = FILTERSBUTTONSHEIGHT*ratio;
	appsize.y = DEFAULTAPPH-filtersheight;
	filterscount = sizeof(filters)/sizeof(filters[0]);

	// Set up the haar finder
	hF.setup("haarcascade_frontalface_default.xml");

	// Initialize variables
	imagecount = 0;
	selectedImage = NULL;
	highlightedImage = NULL;
	vidplayer_alpha = 0;
	generatingmeta = false;
	contextopen = false;
	selectedfilter = NoFilter;
	filterfurther = -1;
	physicson = true;

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

	// Initialize the metadata menu
	gui_metadatamenu.setup();
	gui_metadatamenu.setSize(256, 256);
	gui_metadatamenu.add(gui_metadatamenu_tags.setup("Tags: ", ""));
	gui_metadatamenu.add(gui_metadatamenu_luminance.setup("Average Luminance: ", ""));
	gui_metadatamenu.add(gui_metadatamenu_color.setup("Average Color: ", ""));
	gui_metadatamenu.add(gui_metadatamenu_faces.setup("Face count: ", ""));
	gui_metadatamenu.add(gui_metadatamenu_edge.setup("Edge Distribution: ", ""));
	gui_metadatamenu.add(gui_metadatamenu_cuts.setup("Scene cuts: ", ""));
	gui_metadatamenu_tags.addListener(this, &ofApp::OnTagsChanged);

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

		// Allow dynamic image movement if physics is enabled
		if (physicson)
		{
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
					escape = {MAX(0, MIN(escape.x, appsize.x - img->GetSize().x)), MAX(0, MIN(escape.y, appsize.y - img->GetSize().y))};
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
			img->SetPos(MAX(0, MIN(img->GetPos().x, appsize.x - img->GetSize().x)), MAX(0, MIN(img->GetPos().y, appsize.y - img->GetSize().y)));
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
		ofTrueTypeFont font;
		string buttons[] = FILTERSBUTTONS;
		float width = appsize.x/filterscount;
		float ratiow = appsize.x/DEFAULTAPPW;
		float ratioh = appsize.y/DEFAULTAPPH;
		font.load(OF_TTF_SANS, FILTERSFONT*ratiow);
		for (int i=0; i<filterscount; i++)
		{
			ofSetColor(ofColor::black);
			ofDrawRectangle(width*i, ofGetWindowHeight()-filtersheight, width, filtersheight);
			if (selectedfilter == i)
				ofSetColor(192, 192, 192, 255);
			else
				ofSetColor(ofColor::white);
			ofDrawRectangle(width*i+1, ofGetWindowHeight()-filtersheight+1, width-2, filtersheight-2);
			ofSetColor(ofColor::black);
			font.drawString(buttons[i], width*i+1+width/2 - font.stringWidth(buttons[i])/2, ofGetWindowHeight()-filtersheight/4 - font.stringHeight(buttons[i])/2);
		}
		if (selectedfilter != NoFilter)
		{
			float padding = 64*ratiow;
			const string filternames[] = FILTERSNAMES;
			ofSetColor(ofColor::white);
			switch (selectedfilter)
			{
				case Luminance:
					font.drawString("Current filter: "+filternames[selectedfilter], padding, 64*ratioh);
					for (int i=0; i<5; i++)
						font.drawString("\n"+to_string((int)(((float)i)*(255.0f/4.0f))), padding+((appsize.x-padding*2)/4)*i, 64*ratioh);
					break;
				case Edge:
					font.drawString("Current filter: "+filternames[selectedfilter], padding, 64*ratioh);
					for (int i=0; i<5; i++)
						font.drawString("\n"+to_string((int)(((float)i)*(359.9f/4.0f))), padding+((appsize.x-padding*2)/4)*i, 64*ratioh);
					break;
				case Faces:
				case Scenes:
					font.drawString("Current filter: "+filternames[selectedfilter], padding, 64*ratioh);
					if (filterslargest == 0)
						font.drawString("\n0", appsize.x/2 + font.stringHeight("0"), 64*ratioh);
					else
						for (int i=0; i<5; i++)
							font.drawString("\n"+to_string(((float)i)*(filterslargest/4.0f)), padding+((appsize.x-padding*2)/4)*i, 64*ratioh);
					break;
			}
		}
		ofSetColor(ofColor::white);
		if (!physicson)
			font.drawString("Physics disabled", appsize.x/2-font.stringWidth("Physics disabled")/2, 32*ratioh);
	}

	// Context menu
	if (contextopen)
	{
		ofTrueTypeFont font;
		const string buttons[] = CONTEXTBUTTONS;
		ofSetColor(ofColor::gray);
		ofDrawRectangle(contextpos.x, contextpos.y, CONTEXTWIDTH, CONTEXTBUTHEIGHT*CONTEXTBUTCOUNT);
		ofSetColor(192, 192, 192, 255);
		ofDrawRectangle(contextpos.x+1, contextpos.y+1, CONTEXTWIDTH-2, CONTEXTBUTHEIGHT*CONTEXTBUTCOUNT-2);
		font.load(OF_TTF_SANS, CONTEXTFONT);
		for (int i=0; i<CONTEXTBUTCOUNT; i++)
		{
			ofSetColor(ofColor::gray);
			ofDrawRectangle(contextpos.x, contextpos.y+CONTEXTBUTHEIGHT*i, CONTEXTWIDTH, CONTEXTBUTHEIGHT);
			if (ofGetMouseX() >= contextpos.x && ofGetMouseY() >= contextpos.y+CONTEXTBUTHEIGHT*i && ofGetMouseX() <= contextpos.x+CONTEXTWIDTH && ofGetMouseY() <= contextpos.y+CONTEXTBUTHEIGHT*i+CONTEXTBUTHEIGHT)
				ofSetColor(ofColor::white);
			else
				ofSetColor(192, 192, 192, 255);
			ofDrawRectangle(contextpos.x+1, contextpos.y+1+CONTEXTBUTHEIGHT*i, CONTEXTWIDTH-2, CONTEXTBUTHEIGHT-2);
			ofSetColor(ofColor::black);
			font.drawString(buttons[i], contextpos.x+8, contextpos.y+18+CONTEXTBUTHEIGHT*i);
		}
	}

	// Metadata menu
	if (metadataopen)
		gui_metadatamenu.draw();
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
		string metapath = directory+".artwall/"+dir.getName(i)+"_metadata.xml";
		ofxXmlSettings metadata;
		ThumbObject* img = new ThumbObject(dir.getPath(i), std::rand()%ofGetWindowWidth(), std::rand()%((int)appsize.y));
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
		metadata.loadFile(metapath);
		this->GenerateMetadata(&metadata, img);
		if (generatingmeta)
		{
			metadata.saveFile(metapath);
			generatingmeta = false;
		}
		img->LoadMetadata(&metadata, metapath);
		images[imagecount++] = img;
	}
	printf("Loaded %d images.\n\n", imagecount);
}

//--------------------------------------------------------------
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
	if (!metadata->tagExists("edgeangle"))
	{
		ofImage* frame;
		double edge[4] = {};
		double finalang = 0;
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
		finalang = edge[0]*90 + edge[1]*0 + edge[2]*45 + edge[3]*135 /MAX(1, edge[0]+edge[1]+edge[2]+edge[3]);
		metadata->addValue("edgeangle", ((int)finalang)%360);
	}

	// Add the thumbnail positions tag
	if (!metadata->tagExists("cuts"))
	{
		const double threshold = 100;
		std::vector<double>* cuts;
		generatingmeta = true;

		printf("Detecting video cuts\n");

		metadata->addTag("cuts");
		metadata->pushTag("cuts");
		switch (img->GetThumbType())
		{
			case Video:
				cuts = vidDetectCut((ofVideoPlayer*)img->GetVideo(), threshold);
				for (int i = 0; i < cuts->size(); i++)
					metadata->addValue("value", (double)cuts->at(i));
				delete cuts;
				break;
			case GIF:
				break;
			case Image:
				break;
		}
		metadata->popTag();
	}

	// Add the thumbnail positions tag
	if (!metadata->tagExists("textures"))
	{
		ofImage* frame;
		double* textures = new double[8];
		generatingmeta = true;
		printf("Detecting textures\n");

		metadata->addTag("textures");
		metadata->pushTag("textures");
		switch (img->GetThumbType())
		{
			case Video:
				frame = new ofImage();
				((ofVideoPlayer*)img->GetVideo())->setPosition(0);
				frame->setFromPixels(((ofVideoPlayer*)img->GetVideo())->getPixels());
				textures = calculateGabor(*frame, textures);
				delete frame;
				break;
			case GIF:
				textures = calculateGabor(*img->GetGIF(), textures);
				break;
			case Image:
				textures = calculateGabor(*img->GetImage(), textures);
				break;
		}
		for (int i=0; i<8; i++)
			metadata->addValue("value", textures[i]);
		delete textures;
		metadata->popTag();
	}

	// Stop the video if it was running because of cut detection
	if (img->GetThumbType() == Video)
	{
		((ofVideoPlayer*)img->GetVideo())->setPosition(0);
		((ofVideoPlayer*)img->GetVideo())->setPaused(true);
	}
	metadata->popTag();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (metadataopen)
		return;
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
	else
	{
		switch (key)
		{
			case ' ':
				physicson = !physicson;
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
			x = MAX(0, MIN(x-gpos.x, appsize.x - size.x));
			y = MAX(0, MIN(y-gpos.y, appsize.y - size.y));
			selectedImage->SetPos(x, y);
		}
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// Handle the metadata menu first
	if (metadataopen && gui_metadatamenu.getShape().inside(x,y))
		return;
	else if (metadataopen)
	{
		metadataopen = false;
		return;
	}

	if (button == OF_MOUSE_BUTTON_LEFT)
	{
		bool selected = false;

		// Handle context menu next
		if (contextopen)
		{
			if (x >= contextpos.x && y >= contextpos.y && x <= contextpos.x + CONTEXTWIDTH && y <= contextpos.y + CONTEXTBUTCOUNT*CONTEXTBUTHEIGHT)
			{
				for (int i=0; i<CONTEXTBUTCOUNT; i++)
				{
					if (ofGetMouseX() >= contextpos.x && ofGetMouseY() >= contextpos.y+CONTEXTBUTHEIGHT*i && ofGetMouseX() <= contextpos.x+CONTEXTWIDTH && ofGetMouseY() <= contextpos.y+CONTEXTBUTHEIGHT*i+CONTEXTBUTHEIGHT)
					{
						Meta* data = contextobject->GetMetadata();
						string tagsstring = "";
						switch (i)
						{
							case 0:
								for (int j=0; j<data->tags.size(); j++)
								{
									if (tagsstring == "")
										tagsstring = data->tags[j];
									else
										tagsstring += " " + data->tags[j];
								}
								gui_metadatamenu_tags = tagsstring;
								gui_metadatamenu_luminance = to_string(data->averageluminance);
								gui_metadatamenu_color = "["+to_string((int)data->averagecolor.red)+","+to_string((int)data->averagecolor.green)+","+to_string((int)data->averagecolor.blue)+"]";
								gui_metadatamenu_faces = to_string(data->facecount);
								gui_metadatamenu_edge = to_string(data->edgeangle);
								gui_metadatamenu_cuts = to_string(data->cuts.size());
								//2*90 + 3*0 + 2*45 + 2*135 /(2+3+2+2)
								metadataopen = true;
								break;
						}
						contextopen = false;
					}
				}
			}
			else
				contextopen = false;
			return;
		}

		// Handle filters next
		if (y >= appsize.y)
		{
			float width = appsize.x/filterscount;
			for (int i=0; i<filterscount; i++)
			{
				if (x >= width*i && x <= width*(i+1))
				{
					float ratio = appsize.x/DEFAULTAPPW;
					float padding = 64*ratio;
					if (highlightedImage != NULL && highlightedImage->GetThumbType() == Video)
						highlightedImage->SetVideoPlaying(false);
					highlightedImage = NULL;
					switch (i)
					{
						case Luminance:
							if (selectedfilter == Luminance) { selectedfilter = NoFilter; break; } else selectedfilter = Luminance;
							for (int j=0; j<imagecount; j++)
							{
								ThumbObject* img = images[j];
								Vector2D size = img->GetMinSize();
								Meta* meta = img->GetMetadata();
								img->SetSize(img->GetMinSize());
								img->SetPos(padding + MAX(0, MIN(1, (meta->averageluminance/255)))*(appsize.x-padding*2)-size.x/2, appsize.y/2-size.y/2);
								img->SetPos(img->GetPos().x + std::rand()%2-5, img->GetPos().y + std::rand()%2-5);
							}
							break;
						case Edge:
							if (selectedfilter == Edge) { selectedfilter = NoFilter; break; } else selectedfilter = Edge;
							for (int j=0; j<imagecount; j++)
							{
								ThumbObject* img = images[j];
								Vector2D size = img->GetMinSize();
								Meta* meta = img->GetMetadata();
								img->SetSize(img->GetMinSize());
								img->SetPos(padding + MAX(0, MIN(1, ((float)meta->edgeangle/360)))*(appsize.x-padding*2)-size.x/2, appsize.y/2-size.y/2);
								img->SetPos(img->GetPos().x + std::rand()%2-5, img->GetPos().y + std::rand()%2-5);
							}
							break;
						case Scenes:
							if (selectedfilter == Scenes) { selectedfilter = NoFilter; break; } else selectedfilter = Scenes;
							filterslargest = 0;
							for (int j=0; j<imagecount; j++)
								if (filterslargest < images[j]->GetMetadata()->cuts.size())
									filterslargest = images[j]->GetMetadata()->cuts.size();
							for (int j=0; j<imagecount; j++)
							{
								ThumbObject* img = images[j];
								Vector2D size = img->GetMinSize();
								Meta* meta = img->GetMetadata();
								img->SetSize(img->GetMinSize());
								img->SetPos(padding + MAX(0, MIN(1, ((float)meta->cuts.size()/filterslargest)))*(appsize.x-padding*2)-size.x/2, appsize.y/2-size.y/2);
								img->SetPos(img->GetPos().x + std::rand()%2-5, img->GetPos().y + std::rand()%2-5);
							}
							break;
						case Faces:
							if (selectedfilter == Faces) { selectedfilter = NoFilter; break; } else selectedfilter = Faces;
							filterslargest = 0;
							for (int j=0; j<imagecount; j++)
								if (filterslargest < (float)images[j]->GetMetadata()->facecount)
									filterslargest = (float)images[j]->GetMetadata()->facecount;
							for (int j=0; j<imagecount; j++)
							{
								ThumbObject* img = images[j];
								Vector2D size = img->GetMinSize();
								Meta* meta = img->GetMetadata();
								img->SetSize(img->GetMinSize());
								img->SetPos(padding + MAX(0, MIN(1, ((float)meta->facecount/filterslargest)))*(appsize.x-padding*2)-size.x/2, appsize.y/2-size.y/2);
								img->SetPos(img->GetPos().x + std::rand()%2-5, img->GetPos().y + std::rand()%2-5);
							}
							break;
					}
				}
			}
			return;
		}

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
		bool selected = false;

		// Check which image we pressed
		for (int i=0; i<imagecount; i++)
		{
			ThumbObject* img = images[i];
			if (img->IsOverlapping(x, y))
			{
				contextpos = {(float)x, (float)y};
				contextopen = true;
				contextobject = img;
				selected = true;
				break;
			}
		}

		// If nothing was pressed, close any open context menu
		if (!selected)
			contextopen = false;
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
	float ratio = appsize.x/DEFAULTAPPW;
	filtersheight = FILTERSBUTTONSHEIGHT*ratio;
	appsize.y -= filtersheight;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

//--------------------------------------------------------------
void ofApp::OnTagsChanged(string & text)
{
	ofxXmlSettings metadata;
	std::vector<string> tags; 
	ThumbObject* obj = contextobject;
	string temp = "";

	// Split the string by spaces
	for(int i=0; i<text.length(); ++i)
	{
		if(text[i] == ' ')
		{
			tags.push_back(temp);
			temp = "";
		}
		else
			temp.push_back(text[i]);
	}
	tags.push_back(temp);

	// Save the metadata
	contextobject->GetMetadata()->tags = tags;
	metadata.loadFile(contextobject->GetMetadata()->path);
	metadata.pushTag("metadata");
	metadata.clearTagContents("tags");
	metadata.pushTag("tags");
	for(int i=0; i<tags.size(); i++)
		metadata.addValue("tag", tags[i]);
	metadata.popTag();
	metadata.popTag();
	metadata.saveFile(contextobject->GetMetadata()->path);
	cout << "Tags changed successfully on '"+contextobject->GetMetadata()->path+"'\n";
}