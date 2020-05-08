#pragma once

#include "GL3PluginUI.h"

#include <vector>
#include "IPlugEditorDelegate.h"
#include "IPlugParameter.h"
#include "BezierCurve.h"
#include "ringbuf.h"
#include "memtools.h"


class TestUI : public GL3PluginUI{
public:

	TestUI(iplug::IEditorDelegate* mDelegate_t, int kNumCurvePoints_t, int kInitCurvePoint_t, int kInitShapePoint_t, int kNumPointsEnabled_t, WDL_TypedRingBuf<float>* oscilloscopeBuffer_t):
		  mDelegate(mDelegate_t), kNumCurvePoints(kNumCurvePoints_t), kInitCurvePoint(kInitCurvePoint_t), kInitShapePoint(kInitShapePoint_t), kNumPointsEnabled(kNumPointsEnabled_t),
		  oscilloscopeBuffer(oscilloscopeBuffer_t), curve(kNumCurvePoints) {
		oscilloscopeBuffer->SetSize(OSCILLOSCOPE_BUFFER_SIZE);
		oscilloscopeBuffer->Fill(0.f);
		generate_ramp(oscillator_x_coords,OSCILLOSCOPE_BUFFER_SIZE);
	}
	
	//vv These functions will be called with the correct context selected
	bool initGLContext() override;
	void destroyGLContext() override;
	void drawFrame(PuglView*) override;
	
	// UI Manipulation
	void mouseDown(uint32_t button,double x,double y) override;
	void mouseUp(uint32_t button,double x,double y) override;
	void mouseMove(double x, double y) override;
	
	// Parameters
	void setParameterFromUI(int paramIdx, double val);
	void setParameterFromUI(int paramIdx, int val);
	void changeUIOnParamChange(int paramIdx);
	
private:
	GLuint single_color_shader;
	GLuint circle_shader;
	GLuint image_shader;
	
	GLint line_color_uniform;
	GLuint bg_texture_uniform;
	
	GLuint vao;
	GLuint vbo;
	

	iplug::IEditorDelegate* mDelegate;
	int kNumCurvePoints, kInitCurvePoint, kInitShapePoint, kNumPointsEnabled;
	
	const static int OSCILLOSCOPE_BUFFER_SIZE = 1024;
	WDL_TypedRingBuf<float>* oscilloscopeBuffer;
	float oscillator_x_coords[OSCILLOSCOPE_BUFFER_SIZE];

	BezierCurve curve;
	std::vector<glm::vec2> hires_curve; //This should only be updated on a frame where the curve has been updated
	bool curve_updated;
	
	
	CurvePoint selected_point = NULL_CURVE_POINT;
	enum MOUSE_BUTTON{
		LEFT_MOUSE_BTN = 1,
		RIGHT_MOUSE_BTN = 3,
		NUMBER_MOUSE_BUTTONS
	};
	bool mouse_down[NUMBER_MOUSE_BUTTONS] = {false};
	
	const glm::vec3 color_tangeants = {0.f,1.f,0.f};
	const glm::vec3 color_shape_circles = {0.5f,0.f,1.f};
	const glm::vec3 color_circles = {1.f,0.f,1.f};
	const glm::vec3 color_lines = {1.f,1.f,1.f};
	const glm::vec3 color_curve = {0.f,1.f,1.f};
	
	const GLfloat ENTIRE_SCREEN[12] = { -1.f,1.f, -1.f,-1.f, 1.f,-1.f,
										-1.f,1.f, 1.f,-1.f,  1.f,1.f };
};