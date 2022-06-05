#include "thumbobj.h"

ThumbObject::ThumbObject(string path, float x, float y)
{
	float ratio;
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
		this->m_imgsizereal = {img->getWidth(), img->getHeight()};
		printf("Loaded video '%s'\n", path.c_str());
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
		{
			this->m_imgtype = GIF;
			printf("Loaded GIF '%s'\n", path.c_str());
		}
		else
		{
			this->m_imgtype = Image;
			printf("Loaded image '%s'\n", path.c_str());
		}
		this->m_imgsizereal = {img->getWidth(), img->getHeight()};
	}

	ratio = this->m_imgsizereal.x/this->m_imgsizereal.y;
	this->m_imgpath = path;
	this->m_imgsizemin = {ratio*64.0f, 1.0f*64.0f};
	this->m_imgsizemax = this->m_imgsizereal;
	this->m_imgsize = this->m_imgsizemin;
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
	return this->m_imgsizereal;
}

Vector2D ThumbObject::GetSize()
{
	return this->m_imgsize;
}

Vector2D ThumbObject::GetGrabbedPosition()
{
	return this->m_grabbedpos;
}

Vector2D ThumbObject::GetMinSize()
{
	return this->m_imgsizemin;
}

Vector2D ThumbObject::GetMaxSize()
{
	return this->m_imgsizemax;
}

void ThumbObject::SetPos(float x, float y)
{
	this->m_imgpos.x = x;
	this->m_imgpos.y = y;
}

void ThumbObject::SetPos(Vector2D pos)
{
	this->m_imgpos = pos;
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

void ThumbObject::SetSize(Vector2D size)
{
	this->m_imgsize = size;
}

void ThumbObject::SetGrabbedPosition(float x, float y)
{
	this->m_grabbedpos.x = x;
	this->m_grabbedpos.y = y;
}

void ThumbObject::SetMinSize(Vector2D size)
{
	this->m_imgsizemin = size;
}

void ThumbObject::SetMaxSize(Vector2D size)
{
	this->m_imgsizemax = size;
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