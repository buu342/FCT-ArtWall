#include "ofApp.h"
#include "thumbobj.h"

#define VIDEOCUTTIME 1000

ThumbObject::ThumbObject(string path, float x, float y)
{
	std::string ext = path;
	ext = ext.substr(ext.find_last_of(".") + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "mkv" || ext == "wmv" || ext == "flv" || ext == "mpeg")
	{
		ofVideoPlayer* img = new ofVideoPlayer();
		if (!img->load(path))
		{
			this->m_thumbtype = None;
			delete img;
			return;
		}
		img->setLoopState(OF_LOOP_NORMAL);
		img->setVolume(0);
		img->play();

		this->m_thumb = img;
		this->m_thumbtype = Video;
		this->m_thumbsizereal = {img->getWidth(), img->getHeight()};
		printf("Loaded video '%s'\n", path.c_str());
	}
	else
	{
		ofImage* img = new ofImage();
		if (!img->load(path))
		{
			this->m_thumbtype = None;
			delete img;
			return;
		}
		this->m_thumb = img;
		if (ext == "gif")
		{
			this->m_thumbtype = GIF;
			printf("Loaded GIF '%s'\n", path.c_str());
		}
		else
		{
			this->m_thumbtype = Image;
			printf("Loaded image '%s'\n", path.c_str());
		}
		this->m_thumbsizereal = {img->getWidth(), img->getHeight()};
	}

	this->m_thumbsizeratio = this->m_thumbsizereal.x/this->m_thumbsizereal.y;
	this->m_thumbpath = path;
	this->m_thumbsizemin = {64.0f, 64.0f/this->m_thumbsizeratio};
	this->m_thumbsizemax = this->m_thumbsizereal;
	this->m_thumbsize = this->m_thumbsizemin;
	this->m_thumbpos = {x, y};
	this->m_grabbedpos = {0.0f, 0.0f};
	this->m_videoplaying = false;
	this->m_videomuted = false;
	this->m_videocuttimer = ofGetElapsedTimeMillis() + VIDEOCUTTIME;
}

ThumbObject::~ThumbObject()
{
	this->m_metadata.tags.clear();
	this->m_metadata.cuts.clear();
}

void ThumbObject::update()
{
	if (this->m_thumbtype == Video)
	{
		ofVideoPlayer* vid = (ofVideoPlayer*)this->m_thumb;

		// If the timer has elapsed, advance the video thumbnail
		if (highlightedImage != this && this->m_videocuttimer < ofGetElapsedTimeMillis())
		{
			vid->setPosition(((float)(((int)(vid->getPosition()*10.0f + 1.0f))%10))/10.0f);
			this->m_videocuttimer = ofGetElapsedTimeMillis() + VIDEOCUTTIME;
		}

		// Update the video
	    vid->update();
	}
}

void ThumbObject::LoadMetadata(ofxXmlSettings* metadata)
{
	
}

ofImage* ThumbObject::GetImage()
{
	return (ofImage*)this->m_thumb;
}

ofImage* ThumbObject::GetGIF()
{
	return (ofImage*)this->m_thumb;
}

ofVideoPlayer* ThumbObject::GetVideo()
{
	return (ofVideoPlayer*)this->m_thumb;
}

ThumbType ThumbObject::GetThumbType()
{
	return this->m_thumbtype;
}

Vector2D ThumbObject::GetPos()
{
	return this->m_thumbpos;
}

Vector2D ThumbObject::GetRealSize()
{
	return this->m_thumbsizereal;
}

Vector2D ThumbObject::GetSize()
{
	return this->m_thumbsize;
}

Vector2D ThumbObject::GetMinSize()
{
	return this->m_thumbsizemin;
}

Vector2D ThumbObject::GetMaxSize()
{
	return this->m_thumbsizemax;
}

float ThumbObject::GetSizeRatio()
{
	return this->m_thumbsizeratio;
}

Vector2D ThumbObject::GetGrabbedPosition()
{
	return this->m_grabbedpos;
}

bool ThumbObject::GetVideoPlaying()
{
	return this->m_videoplaying;
}

bool ThumbObject::GetVideoMuted()
{
	return this->m_videomuted;
}

void ThumbObject::SetPos(float x, float y)
{
	this->m_thumbpos.x = x;
	this->m_thumbpos.y = y;
}

void ThumbObject::SetPos(Vector2D pos)
{
	this->m_thumbpos = pos;
}

void ThumbObject::SetSize(float x, float y)
{
	this->m_thumbsize.x = x;
	this->m_thumbsize.y = y;
}

void ThumbObject::SetSize(Vector2D size)
{
	this->m_thumbsize = size;
}

void ThumbObject::SetGrabbedPosition(float x, float y)
{
	this->m_grabbedpos.x = x;
	this->m_grabbedpos.y = y;
}

void ThumbObject::SetMinSize(Vector2D size)
{
	this->m_thumbsizemin = size;
}

void ThumbObject::SetMaxSize(Vector2D size)
{
	this->m_thumbsizemax = size;
}

void ThumbObject::SetVideoPlaying(bool play)
{
	this->m_videoplaying = play;
	if (this->m_videoplaying)
	{
		((ofVideoPlayer*)this->m_thumb)->setVolume(this->m_videomuted ? 0.0f : 1.0f);
		((ofVideoPlayer*)this->m_thumb)->play();
	}
	else
		((ofVideoPlayer*)this->m_thumb)->setPaused(true);
}

void ThumbObject::SetVideoMuted(bool muted)
{
	this->m_videomuted = muted;
	((ofVideoPlayer*)this->m_thumb)->setVolume(this->m_videomuted ? 0.0f : 1.0f);
}

void ThumbObject::AdvanceVideo(int frames)
{
	ofVideoPlayer* vid = (ofVideoPlayer*)this->m_thumb;
	this->SetVideoPlaying(false);

	// setFrame doesn't seem to work properly so I'm using this hack
	if (frames > 0)
	{
		for (int i=0; i<frames; i++)
			vid->nextFrame();
	}
	else
	{
		for (int i=frames; i<0; i++)
			vid->previousFrame();
	}
}

bool ThumbObject::IsOverlapping(float x, float y)
{
	return (
		x >= this->m_thumbpos.x && x <= this->m_thumbpos.x + this->m_thumbsize.x &&
		y >= this->m_thumbpos.y && y <= this->m_thumbpos.y + this->m_thumbsize.y
	);
}

bool ThumbObject::IsOverlapping(Vector2D coord)
{
	return this->IsOverlapping(coord.x, coord.y);
}

bool ThumbObject::IsOverlapping(ThumbObject* other)
{
	return (
		this->m_thumbpos.x < other->GetPos().x + other->GetSize().x &&
		this->m_thumbpos.x + this->m_thumbsize.x > other->GetPos().x &&
		this->m_thumbpos.y < other->GetPos().y + other->GetSize().y &&
		this->m_thumbsize.y + this->m_thumbpos.y > other->GetPos().y
	);
}