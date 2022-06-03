#include "thumbobj.h"

ThumbObject::ThumbObject(string path, float x, float y)
{
	ofImage* img = new ofImage();
	img->load(path);
	this->m_img = img;
	this->m_imgpath = path;
	this->m_imgrealsize = {img->getWidth(), img->getHeight()};
	this->m_imgsize = {1.0f, 1.0f};
	this->m_imgpos = {x, y};
	this->m_imgrot = 0.0f;
	this->m_grabbedpos = {0.0f, 0.0f};
}

ThumbObject::~ThumbObject()
{

}

ofImage* ThumbObject::GetImage()
{
	return this->m_img;
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