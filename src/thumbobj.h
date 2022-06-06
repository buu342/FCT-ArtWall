#pragma once

#ifndef CODE_ANALYSIS
#include "ofMain.h"
#include "ofxXmlSettings.h"
#endif

#include "structs.h"

class ThumbObject
{
	private:
		void*     m_thumb;
		ThumbType m_thumbtype;
		string    m_thumbpath;
		Vector2D  m_thumbsizereal;
		Vector2D  m_thumbpos;
		Vector2D  m_thumbsize;
		Vector2D  m_thumbsizemin;
		Vector2D  m_thumbsizemax;
		float     m_thumbsizeratio;
		Vector2D  m_grabbedpos;
		bool      m_videoplaying;
		bool      m_videomuted;
		uint64_t  m_videocuttimer;
		Meta      m_metadata;

	public:
		ThumbObject(string path, float x, float y);
		~ThumbObject();
		void update();
		void LoadMetadata(ofxXmlSettings* metadata, string path);

		ofImage*       GetImage();
		ofImage*       GetGIF();
		ofVideoPlayer* GetVideo();
		ThumbType      GetThumbType();
		Vector2D       GetPos();
		Vector2D       GetRealSize();
		Vector2D       GetSize();
		Vector2D       GetMinSize();
		Vector2D       GetMaxSize();
		float          GetSizeRatio();
		Vector2D       GetGrabbedPosition();
		bool           GetVideoPlaying();
		bool           GetVideoMuted();
		Meta*          GetMetadata();

		void SetPos(float x, float y);
		void SetPos(Vector2D pos);
		void SetSize(float x, float y);
		void SetSize(Vector2D size);
		void SetMinSize(Vector2D size);
		void SetMaxSize(Vector2D size);
		void SetGrabbedPosition(float x, float y);
		void SetVideoPlaying(bool play);
		void SetVideoMuted(bool muted);

		void AdvanceVideo(int frames);
		bool IsOverlapping(float x, float y);
		bool IsOverlapping(Vector2D coord);
		bool IsOverlapping(ThumbObject* other);
};