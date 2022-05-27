#pragma once

#ifndef CODE_ANALYSIS
#include "ofMain.h"
#endif

typedef struct {
	int x;
	int y;
} Vector2D;

class ThumbObject
{
	private:
		ofImage  m_img;
		string   m_imgpath;
		Vector2D m_imgrealsize;
		Vector2D m_imgpos;
		Vector2D m_imgsize;
		float    m_imgrot;

	public:
		ThumbObject(ofImage image, string path);
		~ThumbObject();
};