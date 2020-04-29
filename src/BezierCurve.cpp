#include "BezierCurve.h"

#include <cfloat>

using namespace glm;

void clampOnGrid(vec2& location){
	if(location.x<-1.f) location.x = -1.f;
	else if(location.x>1.f) location.x = 1.f;
	if(location.y<-1.f) location.y = -1.f;
	else if(location.y>1.f) location.y = 1.f;
}

GLfloat clamp(GLfloat x, GLfloat lower, GLfloat higher){
	if(x<lower) return lower;
	else if(x>higher) return higher;
	else return x;
}

GLfloat minimum_distance_sq(vec2& line_start,vec2& line_end,vec2& point){
	const vec2 line_dir = line_end - line_start;
	const GLfloat l2 = dot(line_dir,line_dir);
	if(-0.001 < l2 && l2 < 0.001){
		const vec2 PQ = point-line_start;
		return dot(PQ,PQ);
	}
	
	const GLfloat t = clamp( dot(line_dir,point-line_start)/l2, -1.f, 1.f );
	const vec2 projection = line_start + line_dir*t;
	const vec2 MP = point-projection;
	return dot(MP,MP);
}


//CurvePoint functions

bool CurvePoint::exists(){
	if(index<0) return false;
	else return true;
}

BezierCurve::BezierCurve(){
	vertices.push_back(vec2(-1.f,-1.f));
	shape_points.push_back(vec2(0.1f,0.1f));
	vertices.push_back(vec2(1.f,1.f));
	shape_points.push_back(vec2(0.1f,0.1f));
}

CurvePoint BezierCurve::add_point(vec2 location){
	clampOnGrid(location);
	
	//Find out where to insert it by what edge the point is closest to, then insert into the vector
	double closest_distance_sq = DBL_MAX;
	int point_index = 0;
	const size_t edge_num = vertices.size()-1;
	for(int i=0;i<edge_num;i++){
		//vertices[i+1] guaranteed safe
		const double next_distance_sq = minimum_distance_sq(vertices[i],vertices[i+1],location);
		if(next_distance_sq<closest_distance_sq){
			closest_distance_sq = next_distance_sq;
			point_index = i+1;
		}
	}
	
	//The x coord can't preceed the previous or exceed the next
	if(location.x < vertices[point_index-1].x) location.x = vertices[point_index-1].x;
	else if(location.x > vertices[point_index].x) location.x = vertices[point_index].x;
	
	//Insert the point, keeping the sizes constant
	std::vector<vec2>::iterator it = vertices.begin();
	vertices.insert(it+point_index,location);
	it = shape_points.begin();
	shape_points.insert(it+point_index,vec2(0.1f,0.1f)); //Maybe change the tangeant direction
	
	return {point_index,VERTEX,false};
}

CurvePoint BezierCurve::check_point(vec2 click_point){
	//UNOPTIMIZED VERSION; TEMPORARY
	
	double closest_distance_sq = DBL_MAX;
	CurvePoint closest_point = NULL_CURVE_POINT;
	
	const size_t num_vertices = vertices.size();
	for(int i=0;i<num_vertices;i++){
		vec2 distance_vector = vertices[i]-click_point;
		double new_distance_sq = dot(distance_vector,distance_vector);
		if(new_distance_sq < closest_distance_sq){
			closest_distance_sq = new_distance_sq;
			
			closest_point = {i, VERTEX, false};
		}
	}
	
	for(int i=0;i<num_vertices;i++){
		vec2 distance_vector = vertices[i]+shape_points[i]-click_point;
		double new_distance_sq = dot(distance_vector,distance_vector);
		if(new_distance_sq < closest_distance_sq){
			closest_distance_sq = new_distance_sq;
			
			closest_point = {i, SHAPE_POINT, false};
		}
		distance_vector = vertices[i]-shape_points[i]-click_point;
		new_distance_sq = dot(distance_vector,distance_vector);
		if(new_distance_sq < closest_distance_sq){
			closest_distance_sq = new_distance_sq;
			
			closest_point = {i, SHAPE_POINT, true};
		}
	}
	
	if(closest_distance_sq <= POINT_CLICK_RADIUS*POINT_CLICK_RADIUS) return closest_point;
	else return NULL_CURVE_POINT;
}

bool BezierCurve::move_point(CurvePoint p,vec2 destination){
	
	if(p.index<0) return false;
	
	if(p.point_type==VERTEX){
		clampOnGrid(destination);
		if(p.index >= vertices.size()) return false;
		
		if(p.index==0) destination.x=0.f; //Lock the first control point to the left side
		else if(p.index == vertices.size()-1) destination.x=1.f; //And last to the right side
		else{
			if(destination.x < vertices[p.index-1].x) destination.x = vertices[p.index-1].x;
			if(destination.x > vertices[p.index+1].x) destination.x = vertices[p.index+1].x;
		}
		
		vertices[p.index] = destination;
	}
	else if(p.point_type==SHAPE_POINT){
		if(p.index >= shape_points.size()) return false;
		
		// vertices[p.index] + relative_destination = destination, so:
		vec2 relative_destination = destination - vertices[p.index];
		if(p.is_inverted){
			// vertices[p.index] - relative_destination = destination since it's inverted
			relative_destination = relative_destination*-1.f; //If it's inverted, reflect it across the vertex
		}
		if(relative_destination.x<0) relative_destination.x = 0; //Clamp it so that the graph's tangeant vector can't go backwards
		
		shape_points[p.index] = relative_destination;
	}
	
	return true;
}

bool BezierCurve::remove_point(CurvePoint p){
	if(p.point_type != VERTEX) return false;
	if(p.index<1 || p.index>vertices.size()-2) return false;
	
	vertices.erase(vertices.begin()+p.index);
	return true;
}



// Getters

GLsizei BezierCurve::get_stride(){
	return sizeof(vec2);
}

void* BezierCurve::get_vertex_ptr(){
	return vertices.data();
}

GLsizei BezierCurve::get_num_vertices(){
	return (GLsizei) vertices.size();
}

GLsizeiptr BezierCurve::get_vertex_buffer_size(){
	return get_stride()*get_num_vertices();
}

void BezierCurve::get_shape_point_buffer(std::vector<vec2>& points){
	//ASSERT: vertices.size() == shape_points.size()
	
	const size_t num_vertices = shape_points.size();
	for(int i=0;i<num_vertices;i++){
		points.push_back(vertices[i]-shape_points[i]);
		points.push_back(vertices[i]+shape_points[i]);
	}
}



// Bezier Curve Processing

void BezierCurve::get_curve_buffer(std::vector<vec2>& points){
	const size_t num_edges = vertices.size()-1;
	
	for(int i=0;i<num_edges;i++){
		vec2 p1 = vertices[i];
		vec2 p2 = vertices[i]+shape_points[i]*BEZIER_SCALE;
		vec2 p3 = vertices[i+1]-shape_points[i+1]*BEZIER_SCALE;
		vec2 p4 = vertices[i+1];
		
		for(unsigned int j=0;j<CURVE_RESOLUTION;j++){
			GLfloat t = 1.f*j/CURVE_RESOLUTION;
			
			vec2 h1 = p1+(p2-p1)*t;
			vec2 h2 = p2+(p3-p2)*t;
			vec2 h3 = p3+(p4-p3)*t;
			
			vec2 l1 = h1+(h2-h1)*t;
			vec2 l2 = h2+(h3-h2)*t;
			
			points.push_back(l1+(l2-l1)*t);
		}
	}
	
	points.push_back(vertices[num_edges]);
}
