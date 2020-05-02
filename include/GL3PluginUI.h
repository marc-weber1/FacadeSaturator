#pragma once

extern "C" {
	#include "pugl/pugl.h"
}

class GL3PluginUI{
public:
	virtual bool initGLContext() = 0;
	virtual void destroyGLContext() = 0;
	virtual void drawFrame(PuglView*) = 0;
	virtual void mouseDown(uint32_t button,double x,double y) = 0;
	virtual void mouseUp(uint32_t button,double x,double y) = 0;
	virtual void mouseMove(double x, double y) = 0;
	virtual void setParameterFromUI(int paramIdx, double val) =0;
	virtual void changeUIOnParamChange(int paramIdx, double val) =0;
};