#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "IPlugEditorDelegate.h"

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
	IParamBezier(iplug::IEditorDelegate* tDelegate, int tRemoveDCOffset, int tInitCurvePoint, int tInitShapePoint, int tNumCurvePoints, int tMaxPoints=16) :
		mDelegate(tDelegate), kRemoveDCOffset(tRemoveDCOffset),
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
	const int kRemoveDCOffset, kInitCurvePoint, kInitShapePoint, kNumCurvePoints;
	const static int MAX_POINTS=16;
	const double POINT_CLICK_RADIUS = 0.03;
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
	int sample_smooth_number = 240; //Update this when sample rate changes?? Set for 48000 rn, should be less than the smallest buffer size
	float dc_offset_current = 0.f;
	float dc_offset_target = 0.f;
	bool curve_updated_dsp = true;
	
	//Waveshaping
	float waveshape(float input,bezier_segment* curve,unsigned curve_length);
	
	float sample_smooth_weights[240] = { 1.000000f, 0.999948f, 0.999793f, 0.999535f, 0.999176f, 0.998716f, 0.998156f, 0.997498f, 0.996741f, 0.995887f, 0.994936f, 0.993890f, 0.992750f, 0.991516f, 0.990189f, 0.988770f, 0.987259f, 0.985659f, 0.983969f, 0.982190f, 0.980324f, 0.978371f, 0.976332f, 0.974208f, 0.972000f, 0.969708f, 0.967334f, 0.964879f, 0.962343f, 0.959726f, 0.957031f, 0.954258f, 0.951407f, 0.948480f, 0.945478f, 0.942401f, 0.939250f, 0.936026f, 0.932730f, 0.929363f, 0.925926f, 0.922419f, 0.918844f, 0.915201f, 0.911491f, 0.907715f, 0.903874f, 0.899969f, 0.896000f, 0.891969f, 0.887876f, 0.883723f, 0.879509f, 0.875237f, 0.870906f, 0.866518f, 0.862074f, 0.857574f, 0.853020f, 0.848411f, 0.843750f, 0.839037f, 0.834272f, 0.829457f, 0.824593f, 0.819680f, 0.814719f, 0.809711f, 0.804657f, 0.799559f, 0.794416f, 0.789229f, 0.784000f, 0.778729f, 0.773418f, 0.768066f, 0.762676f, 0.757247f, 0.751781f, 0.746279f, 0.740741f, 0.735168f, 0.729561f, 0.723922f, 0.718250f, 0.712547f, 0.706814f, 0.701051f, 0.695259f, 0.689440f, 0.683594f, 0.677721f, 0.671824f, 0.665902f, 0.659957f, 0.653989f, 0.648000f, 0.641990f, 0.635959f, 0.629910f, 0.623843f, 0.617758f, 0.611656f, 0.605539f, 0.599407f, 0.593262f, 0.587103f, 0.580932f, 0.574750f, 0.568557f, 0.562355f, 0.556145f, 0.549926f, 0.543700f, 0.537469f, 0.531232f, 0.524991f, 0.518746f, 0.512499f, 0.506250f, 0.500000f, 0.493750f, 0.487501f, 0.481254f, 0.475009f, 0.468768f, 0.462531f, 0.456300f, 0.450074f, 0.443855f, 0.437645f, 0.431443f, 0.425250f, 0.419068f, 0.412897f, 0.406738f, 0.400593f, 0.394461f, 0.388344f, 0.382242f, 0.376157f, 0.370090f, 0.364041f, 0.358010f, 0.352000f, 0.346011f, 0.340043f, 0.334098f, 0.328176f, 0.322279f, 0.316406f, 0.310560f, 0.304741f, 0.298949f, 0.293186f, 0.287453f, 0.281750f, 0.276078f, 0.270439f, 0.264832f, 0.259259f, 0.253721f, 0.248219f, 0.242753f, 0.237324f, 0.231934f, 0.226582f, 0.221271f, 0.216000f, 0.210771f, 0.205584f, 0.200441f, 0.195343f, 0.190289f, 0.185281f, 0.180320f, 0.175407f, 0.170543f, 0.165728f, 0.160963f, 0.156250f, 0.151589f, 0.146980f, 0.142426f, 0.137926f, 0.133482f, 0.129094f, 0.124763f, 0.120491f, 0.116277f, 0.112124f, 0.108031f, 0.104000f, 0.100031f, 0.096126f, 0.092285f, 0.088509f, 0.084799f, 0.081156f, 0.077581f, 0.074074f, 0.070637f, 0.067270f, 0.063974f, 0.060750f, 0.057599f, 0.054522f, 0.051520f, 0.048593f, 0.045742f, 0.042969f, 0.040274f, 0.037657f, 0.035121f, 0.032666f, 0.030292f, 0.028000f, 0.025792f, 0.023668f, 0.021629f, 0.019676f, 0.017810f, 0.016031f, 0.014341f, 0.012741f, 0.011230f, 0.009811f, 0.008484f, 0.007250f, 0.006110f, 0.005064f, 0.004113f, 0.003259f, 0.002502f, 0.001844f, 0.001284f, 0.000824f, 0.000465f, 0.000207f, 0.000052f }; //TESTING, SHOULD BE THE SAME SIZE AS sample_smooth_number
	
	/* PRECONDITIONS:
	 * -  numCurvePoints >= 2
	 * -  vec2(-1,-1) <= curvePoint[i] <= vec2(1,1)
	 * -  vec2(0,-2) <= shapePoint[i] <= vec2(2,2)
	*/
	 
	/* POSTCONDITIONS (FOR EVERY FUNCTION):
	 * -  vertices.size() = shape_points.size() = numCurvePoints
	 * -  vertices[i].x = max(0<=j<=i) curvePoint[j].x   forall 0<=i<numCurvePoints
	 * -  shape_points[i].x = {
	 *							vertices[i+1].x-vertices[i].x   if shapePoint[i].x >= vertices[i+1].x-vertices[i].x and i+1<vertices.size()
	 *							vertices[i].x-(vertices[i-1].x+shape_points[i-1].x)   if shapePoint[i].x >= vertices[i].x-(vertices[i-1].x+shape_points[i-1].x) and i>0
	 *							shapePoint[i].x   otherwise
	 *						  }
	*/
};
