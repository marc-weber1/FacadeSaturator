#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "IPlugEditorDelegate.h"
#include "Smoothers.h"

enum PointType{
	VERTEX, SHAPE_POINT
};

struct CurvePoint{
	int index;
	PointType point_type;
	bool is_inverted=false;
	
	inline bool CurvePoint::exists() const{
		if(index<0) return false;
		else return true;
	}
};

const CurvePoint NULL_CURVE_POINT = {-1,VERTEX,false};

class IParamBezier{
public:
	IParamBezier(iplug::IEditorDelegate* tDelegate, int tInitCurvePoint, int tInitShapePoint, int tNumCurvePoints, int tMaxPoints=16) :
		mDelegate(tDelegate),
		kInitCurvePoint(tInitCurvePoint), kInitShapePoint(tInitShapePoint), kNumCurvePoints(tNumCurvePoints)
	{
		vertices.push_back(glm::vec2(-1.f,-1.f));
		vertices.push_back(glm::vec2(1.f,1.f));
		shape_points.push_back(DEFAULT_SHAPE_POINT);
		shape_points.push_back(DEFAULT_SHAPE_POINT);
		
		//Start from the init state
		bezier_lut_current_length = 1;
		bezier_lut_current[0] = { {-1.f,-0.5f,0.5f,1.f}, {-1.f,-0.5f,0.5f,1.f} };
	}
	
	//Alter curve from UI
	CurvePoint add_point_from_UI(glm::vec2);
	CurvePoint check_point(glm::vec2);
	bool remove_point_from_UI(CurvePoint);
	glm::vec2 move_point_from_UI(CurvePoint,glm::vec2);
	
	//Display curve on UI
	GLsizei get_stride();
	void* get_vertex_ptr();
	GLsizei get_num_vertices();
	void get_shape_point_buffer(std::vector<glm::vec2>&);
	void* get_hires_ptr();
	GLsizei get_num_hires();
	void update_hires_buffer();
	
	//Alter curve from host
	void update_param_from_host(int paramIdx);
	void update_params_from_host();
	
	//Get function value from host
	void waveshape(float* inputs, float* outputs, int nFrames, bool numChannels);
	
private:
	//Alter curve from host (private functions)
	void update_vertex_x_from_host(int starting_vertex);
	void update_shape_point_x_from_host(int starting_vertex);
	void add_vertices_from_host(int num_to_add);
	void remove_vertices_from_host(int num_to_remove);
	
	//Alter curve from UI (private functions)
	void set_parameter_from_UI(int paramIdx, double val);
	void update_vertices_from_UI(int starting_point);
	
	//Bezier help functions
	void update_smoothed_vertices();
	
	//For debug
	const bool DEBUG_ASSERTIONS = true;
	void check_assertions();

	// CONSTANTS
	
	iplug::IEditorDelegate* mDelegate;
	const int kInitCurvePoint, kInitShapePoint, kNumCurvePoints;
	
	const static int MAX_POINTS=16;
	const double POINT_CLICK_RADIUS = 0.025;
	const unsigned int CURVE_RESOLUTION = 20; //For the UI
	const GLfloat BEZIER_Y_SCALE = 2.f;
	const glm::vec2 DEFAULT_SHAPE_POINT = glm::vec2(0.1f,0.1f/BEZIER_Y_SCALE);
	const float AUDIO_SAMPLE_ERROR_RADIUS = 0.001f;
	const int MAX_ITERATIONS_PER_SAMPLE = 10; //Set this to -1 for no limit
	
	//Stored values for UI
	bool curve_updated_ui = true;
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> shape_points;
	std::vector<glm::vec2> hires_curve;
	
	//Stored values for DSP
	typedef struct alignas(16) bezier_segment{ //SSE-optimized
		float x[4];
		float y[4];
	} bezier_segment;
	bezier_segment bezier_lut_current[MAX_POINTS-1];
	unsigned int bezier_lut_current_length;
	bezier_segment bezier_lut_target[MAX_POINTS-1];
	unsigned int bezier_lut_target_length;
	//Both of these are buffers of constant-interval points on the curve, in groups of 4 to recover the cubic functions y=h(x)
	int sample_smooth_number = 48; //Update this when sample rate changes?? Set for 48000 rn, should be less than the smallest buffer size
	bool curve_updated_dsp = true;
	
	/* PRECONDITIONS:
	 * -  numCurvePoints >= 2
	 * -  vec2(-1,-1) <= curvePoint[i] <= vec2(1,1)
	 * -  vec2(0,-2) <= shapePoint[i] <= vec2(2,2)
	 */
	 
	/* POSTCONDITIONS (FOR EVERY FUNCTION):
	 * -  vertices.size() = shape_points.size() = numCurvePoints
	 * -  vertices[i].x = max(0<=j<=i) curvePoint[j].x   forall 0<=i<numCurvePoints
	 * -  shape_points[i].x = {
	 *							vertices[i+1].x-vertices[i].x   if shapePoint[i].x >= vertices[i+1].x-vertices[i].x
	 *							vertices[i].x-(vertices[i-1].x+shape_points[i-1].x)   if shapePoint[i].x >= vertices[i].x-(vertices[i-1].x+shape_points[i-1].x) and i>0
	 *							shapePoint[i].x   otherwise
	 *						  }
	 */
};
