#include "thumbobj.h"

ThumbObject::ThumbObject(ofImage image, string path)
{
	this->m_img = image;
	this->m_imgpath = path;
	this->m_imgrealsize = {(int)image.getWidth(), (int)image.getHeight()};
	this->m_imgsize = this->m_imgrealsize;
	this->m_imgpos = {0, 0};
	this->m_imgrot = 0.0f;
}

ThumbObject::~ThumbObject()
{

}