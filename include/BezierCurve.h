#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glad/glad.h"

enum PointType{
	VERTEX, SHAPE_POINT
};

struct CurvePoint{
	int index;
	PointType point_type;
	bool is_inverted=false;
	
	bool exists();
};

const CurvePoint NULL_CURVE_POINT = {-1,VERTEX,false};

class BezierCurve{
public:
	BezierCurve();
	
	CurvePoint add_point(glm::vec2); //Can't add it off the grid
	CurvePoint check_point(glm::vec2);
	bool move_point(CurvePoint,glm::vec2); //Can't move it off the grid
	bool remove_point(CurvePoint); //Can't remove first or last point, or curve points
	
	GLsizei get_stride();
	
	void* get_vertex_ptr();
	GLsizei get_num_vertices();
	GLsizeiptr get_vertex_buffer_size();
	
	void get_shape_point_buffer(std::vector<glm::vec2>&);
	void get_curve_buffer(std::vector<glm::vec2>&);
	
private:
	const double POINT_CLICK_RADIUS = 0.025;
	const unsigned int CURVE_RESOLUTION = 20;
	const unsigned int MAX_VERTICES = 16;
	const GLfloat BEZIER_SCALE = 2.f;
	
	//CONSTANT: these two vertices should have the same number of points
	//CONSTANT: both arrays should have a first and a last point at all times,
	//			with respective x coordinates 0 and 1
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> shape_points; //In coordinates relative to each respective vertex
};
