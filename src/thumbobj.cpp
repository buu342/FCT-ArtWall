#include "thumbobj.h"

ThumbObject::ThumbObject(string path, float x, float y)
{
	printf("Loaded file '%s'\n", path.c_str());
	std::string ext = path;
	ext = ext.substr(ext.find_last_of(".") + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "mkv" || ext == "wmv" || ext == "flv" || ext == "mpeg")
	{
		ofVideoPlayer* img = new ofVideoPlayer();
		if (!img->load(path))
		{
			this->m_imgtype = None;
			delete img;
			return;
		}
		img->setLoopState(OF_LOOP_NORMAL);
		img->play();
		this->m_img = img;
		this->m_imgtype = Video;
		this->m_imgrealsize = {img->getWidth(), img->getHeight()};
	}
	else
	{
		ofImage* img = new ofImage();
		if (!img->load(path))
		{
			this->m_imgtype = None;
			delete img;
			return;
		}
		this->m_img = img;
		if (ext == "gif")
			this->m_imgtype = GIF;
		else
			this->m_imgtype = Image;
		this->m_imgrealsize = {img->getWidth(), img->getHeight()};
	}
	this->m_imgpath = path;
	this->m_imgsize = {1.0f, 1.0f};
	this->m_imgpos = {x, y};
	this->m_imgrot = 0.0f;
	this->m_grabbedpos = {0.0f, 0.0f};
}

ThumbObject::~ThumbObject()
{

}

void ThumbObject::update()
{
	if (this->m_imgtype == Video)
	    ((ofVideoPlayer*)this->m_img)->update();
}

ofImage* ThumbObject::GetImage()
{
	return (ofImage*)this->m_img;
}

ofImage* ThumbObject::GetGIF()
{
	return (ofImage*)this->m_img;
}

ofVideoPlayer* ThumbObject::GetVideo()
{
	return (ofVideoPlayer*)this->m_img;
}

ThumbType ThumbObject::GetThumbType()
{
	return this->m_imgtype;
}

Vector2D ThumbObject::GetPos()
{
	return this->m_imgpos;
}

float ThumbObject::GetRotation()
{
	return this->m_imgrot;
}

Vector2D ThumbObject::GetRealSize()
{
	return this->m_imgrealsize;
}

Vector2D ThumbObject::GetSize()
{
	return this->m_imgsize;
}

Vector2D ThumbObject::GetGrabbedPosition()
{
	return this->m_grabbedpos;
}

void ThumbObject::SetPos(float x, float y)
{
	this->m_imgpos.x = x;
	this->m_imgpos.y = y;
}

void ThumbObject::SetRotation(float ang)
{
	this->m_imgrot = ang;
}

void ThumbObject::SetSize(float x, float y)
{
	this->m_imgsize.x = x;
	this->m_imgsize.y = y;
}

void ThumbObject::SetGrabbedPosition(float x, float y)
{
	this->m_grabbedpos.x = x;
	this->m_grabbedpos.y = y;
}

bool ThumbObject::IsOverlapping(ThumbObject* other)
{
	return (this->m_imgpos.x < other->GetPos().x + other->GetSize().x &&
		this->m_imgpos.x + this->m_imgsize.x > other->GetPos().x &&
		this->m_imgpos.y < other->GetPos().y + other->GetSize().y &&
		this->m_imgsize.y + this->m_imgpos.y > other->GetPos().y
	);
}

bool ThumbObject::WouldOverlap(ThumbObject* other, float scale)
{
	return (this->m_imgpos.x < other->GetPos().x + other->GetSize().x &&
		this->m_imgpos.x + this->m_imgsize.x + scale > other->GetPos().x &&
		this->m_imgpos.y < other->GetPos().y + other->GetSize().y &&
		this->m_imgpos.y + this->m_imgsize.y + scale > other->GetPos().y
	);
}