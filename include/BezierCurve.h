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
	BezierCurve(unsigned int t_max_vertices);
	
	CurvePoint add_point(glm::vec2); //Can't add it off the grid
	CurvePoint check_point(glm::vec2);
	glm::vec2 get_position(CurvePoint);
	glm::vec2 move_point(CurvePoint,glm::vec2); //Can't move it off the grid
	float move_point(CurvePoint,float,bool); //For moving 1 dimension at a time, sets shape points directly
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
	const GLfloat BEZIER_Y_SCALE = 2.f;
	const glm::vec2 DEFAULT_SHAPE_POINT = glm::vec2(0.1f,0.1f/BEZIER_Y_SCALE);
	
	unsigned int max_vertices;
	
	//CONSTANT: these two arrays should have the same number of points
	//CONSTANT: both arrays should have a first and a last point at all times,
	//			with respective x coordinates of the vertex element 0 and 1
	//CONSTANT: vertices[i-1].x <= vertices[i].x forall 0<x<vertices.size()
	//CONSTANT: 0<shape_points[i].x<1, -2<shape_points[i].y<2 forall 0<=i<vertices.size()
	//CONSTANT: vertices[i-1].x+shape_points[i-1].x <= vertices[i]-shape_points[i].x
	//			forall 0<x<vertices.size()
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> shape_points; //In coordinates relative to each respective vertex
	//TO FIX:
	// - for i from i=1 to vertices.size()-1, if vertices[i].x < vertices[i-1].x set vertices[i].x = vertices[i-1].x
	// - for i from i=0 to vertices.size()-2, if vertices[i].x+shape_points[i].x > vertices[i+1].x set shape_points[i].x = vertices[i+1].x-vertices[i].x
	// - for i from i=1 to vertices.size()-1, if vertices[i-1].x+shape_points[i-1].x > vertices[i].x-shape_points[i].x set shape_points[i].x = vertices[i].x-shape_points[i-1].x
};
