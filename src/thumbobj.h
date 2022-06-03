#pragma once

#ifndef CODE_ANALYSIS
#include "ofMain.h"
#endif

typedef struct {
	float x;
	float y;
} Vector2D;

class ThumbObject
{
	private:
		ofImage* m_img;
		string   m_imgpath;
		Vector2D m_imgrealsize;
		Vector2D m_imgpos;
		Vector2D m_imgsize;
		float    m_imgrot;
		Vector2D m_grabbedpos;

	public:
		ThumbObject(string path, float x, float y);
		~ThumbObject();
		ofImage* GetImage();
		Vector2D GetPos();
		float GetRotation();
		Vector2D GetRealSize();
		Vector2D GetSize();
		Vector2D GetGrabbedPosition();
		void SetPos(float x, float y);
		void SetRotation(float ang);
		void SetSize(float x, float y);
		void SetGrabbedPosition(float x, float y);
		bool IsOverlapping(ThumbObject* other);
		bool WouldOverlap(ThumbObject* other, float scale);
};