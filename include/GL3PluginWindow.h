#pragma once

#include <stdio.h>
#include <memory>

#define IGRAPHICS_GL
//#if defined OS_WIN
//	#include <glad/glad.h>
//#elif defined OS_MAC
//	#include <OpenGL/gl3.h>
//#endif

#include <glad/glad.h>

extern "C" {
	#include "pugl/gl.h"
	#include "pugl/pugl.h"
	#include "pugl/pugl_gl.h"
}

#include "GL3PluginUI.h"


class GL3PluginWindow{
public:
	typedef struct {
		int  samples;
		int  doubleBuffer;
		int  sync;
		bool continuous; //unused, for now
		bool help; //unused
		bool ignoreKeyRepeat;
		bool resizable;
		bool verbose; //unused
		bool errorChecking;
	} PuglOptions;


	GL3PluginWindow(void* pParent, unsigned w, unsigned h, unsigned fps, float scale, const PuglOptions opts,std::unique_ptr<GL3PluginUI>& ui_t);
	
	void Resize(int w, int h, float scale);
	void* OpenWindow(void* pParent);
	void CloseWindow();
	PuglStatus handleEvent(const PuglEvent* event);
	
	void ForceCheckEvents(); //DEBUG
	
	
	const static uintptr_t REDRAW_TIMER_ID = 0;
	unsigned framesDrawn;
	
private:

	const char* WINDOW_NAME = "FacadeSaturator";
	const PuglOptions m_opts;

	unsigned m_width, m_height, m_fps;
	float m_scale;
	
	PuglWorld* world;
	PuglView* view;
	
	std::unique_ptr<GL3PluginUI>& ui;
};