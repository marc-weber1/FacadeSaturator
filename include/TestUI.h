#pragma once

#include "GL3PluginUI.h"

#include "IPlugEditorDelegate.h"
#include "IPlugParameter.h"
#include "ringbuf.h"
#include "memtools.h"

#include <iostream>
#include <fstream>
#include <vector>


class TestUI : public GL3PluginUI{
public:

	TestUI(iplug::IEditorDelegate* mDelegate_t, WDL_TypedRingBuf<float>* oscilloscopeBuffer_t, int numOscBuffers_t):
		  mDelegate(mDelegate_t),
		  oscilloscopeBuffer(oscilloscopeBuffer_t), numOscBuffers(numOscBuffers_t) {
		oscilloscopeBuffer[0].SetSize(OSCILLOSCOPE_BUFFER_SIZE);
		oscilloscopeBuffer[0].Fill(0.f);
		oscilloscopeBuffer[1].SetSize(OSCILLOSCOPE_BUFFER_SIZE);
		oscilloscopeBuffer[1].Fill(0.f);
		generate_duplicate_ramp(oscilloscope_x_coords,OSCILLOSCOPE_BUFFER_SIZE);
		
		//DEBUG
		//debug_file.open("C:/Users/facade/Documents/VSTs/facade-saturator-log.txt");
	}
	
	//vv These functions will be called with the correct context selected
	bool initGLContext() override;
	void destroyGLContext() override;
	void drawFrame(PuglView*) override;
	
	// UI Manipulation
	void mouseDown(uint32_t button,double x,double y) {};
	void mouseUp(uint32_t button,double x,double y) {};
	void mouseMove(double x, double y) {};
	
	// Parameters
	//void setParameterFromUI(int paramIdx, double val);
	//void changeUIOnParamChange(int paramIdx);
	//void updateParametersStartingFrom(int paramIdx);
	
private:
	std::ofstream debug_file; //for debug messages

	GLuint single_color_shader;
	GLuint circle_shader;
	GLuint image_shader;
	
	GLint line_color_uniform;
	GLuint bg_texture_uniform;
	
	GLuint vao;
	GLuint vbo;
	

	iplug::IEditorDelegate* mDelegate;
	
	const static int OSCILLOSCOPE_BUFFER_SIZE = 4096;
	int numOscBuffers;
	WDL_TypedRingBuf<float>* oscilloscopeBuffer;
	float oscilloscope_x_coords[OSCILLOSCOPE_BUFFER_SIZE*2];
	
	const glm::vec3 color_lines = {0.85f,0.85f,0.85f};
	const GLfloat image_brightness = 0.7f;
	
	const GLfloat ENTIRE_SCREEN[12] = { -1.f,1.f, -1.f,-1.f, 1.f,-1.f,
										-1.f,1.f, 1.f,-1.f,  1.f,1.f };
};