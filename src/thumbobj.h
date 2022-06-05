#pragma once

#ifndef CODE_ANALYSIS
#include "ofMain.h"
#endif

enum ThumbType {
	None,
	Image,
	GIF,
	Video
};

struct Vector2D{
	float x;
	float y;
	inline Vector2D operator+(Vector2D a) {return {x+a.x, y+a.y};};
	inline Vector2D operator-(Vector2D a) {return {x-a.x, y-a.y};};
	inline Vector2D operator*(Vector2D a) {return {x*a.x, y*a.y};};
	inline Vector2D operator/(Vector2D a) {return {x/a.x, y/a.y};};
	inline Vector2D operator+(float a) {return {x+a, y+a};};
	inline Vector2D operator-(float a) {return {x-a, y-a};};
	inline Vector2D operator*(float a) {return {x*a, y*a};};
	inline Vector2D operator/(float a) {return {x/a, y/a};};
	inline Vector2D operator-(void) {return {-x, -y};};
	inline bool operator==(Vector2D a) {return {x == a.x && y == a.y};};
	inline bool operator<(Vector2D a) {return {x < a.x && y < a.y};};
	inline bool operator>(Vector2D a) {return {x > a.x && y > a.y};};
	inline bool operator<=(Vector2D a) {return {x <= a.x && y <= a.y};};
	inline bool operator>=(Vector2D a) {return {x >= a.x && y >= a.y};};
};

class ThumbObject
{
	private:
		void*     m_img;
		ThumbType m_imgtype;
		string    m_imgpath;
		Vector2D  m_imgsizereal;
		Vector2D  m_imgpos;
		Vector2D  m_imgsize;
		Vector2D  m_imgsizemin;
		Vector2D  m_imgsizemax;
		float     m_imgrot;
		Vector2D  m_grabbedpos;

	public:
		ThumbObject(string path, float x, float y);
		~ThumbObject();
		void update();
		ofImage* GetImage();
		ofImage* GetGIF();
		ofVideoPlayer* GetVideo();
		ThumbType GetThumbType();
		Vector2D GetPos();
		float GetRotation();
		Vector2D GetRealSize();
		Vector2D GetSize();
		Vector2D GetMinSize();
		Vector2D GetMaxSize();
		Vector2D GetGrabbedPosition();
		void SetPos(float x, float y);
		void SetPos(Vector2D pos);
		void SetRotation(float ang);
		void SetSize(float x, float y);
		void SetSize(Vector2D size);
		void SetMinSize(Vector2D size);
		void SetMaxSize(Vector2D size);
		void SetGrabbedPosition(float x, float y);
		bool IsOverlapping(ThumbObject* other);
		bool WouldOverlap(ThumbObject* other, float scale);
};