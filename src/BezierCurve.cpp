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

BezierCurve::BezierCurve(unsigned int t_max_vertices): max_vertices(t_max_vertices){
	vertices.push_back(vec2(-1.f,-1.f));
	shape_points.push_back(vec2(0.1f,0.1f));
	vertices.push_back(vec2(1.f,1.f));
	shape_points.push_back(vec2(0.1f,0.1f));
}

CurvePoint BezierCurve::add_point(vec2 location){
	clampOnGrid(location);
	
	//Where to put the point?
	int point_index=0;
	for(; point_index<vertices.size()-1; point_index++){
		if( location.x <= vertices[point_index].x ) break;
	}
	
	//Insert the point
	std::vector<vec2>::iterator it = vertices.begin();
	vertices.insert(it+point_index,location);
	it = shape_points.begin();
	shape_points.insert(it+point_index,DEFAULT_SHAPE_POINT);
	
	CurvePoint p = {point_index,VERTEX,false};
	
	//Move the last shape point back if necessary
	if(p.index > 0){
		if( vertices[p.index].x < vertices[p.index-1].x+shape_points[p.index-1].x ) shape_points[p.index-1].x = vertices[p.index].x-vertices[p.index-1].x;
	}
	
	//Restrict it just like in move_point
	//Clamp your shape points with the other shape points to keep it functional:
	if(p.index > 0){ //Let the previous clamp you?
		if( vertices[p.index].x-shape_points[p.index].x < vertices[p.index-1].x+shape_points[p.index-1].x ) shape_points[p.index].x = vertices[p.index].x-(vertices[p.index-1].x+shape_points[p.index-1].x);
	}
	if(p.index < vertices.size()-1){ //Clamp the next?
		if( vertices[p.index+1].x < vertices[p.index].x+shape_points[p.index].x ) shape_points[p.index].x = vertices[p.index+1].x-vertices[p.index].x;
		if( vertices[p.index+1].x-shape_points[p.index+1].x < vertices[p.index].x+shape_points[p.index].x ) shape_points[p.index+1].x = vertices[p.index+1].x-(vertices[p.index].x+shape_points[p.index].x);
	}
	
	return p;
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

vec2 BezierCurve::get_position(CurvePoint p){
	if(p.index<0) return vec2(0,0);
	if(p.index >= vertices.size()) return vec2(0,0);
	
	if(p.point_type==VERTEX) return vertices[p.index];
	else if(p.point_type==SHAPE_POINT) return shape_points[p.index];
	else return vec2(0,0); //Should never happen
}

bool BezierCurve::move_point(CurvePoint p,vec2 destination){
	
	if(p.index<0) return false;
	if(p.index >= vertices.size()) return false;
	
	if(p.point_type==VERTEX){
		clampOnGrid(destination);
		
		if(p.index==0) destination.x=-1.f; //Lock the first control point to the left side
		else if(p.index == vertices.size()-1) destination.x=1.f; //And last to the right side
		else{
			if(destination.x < vertices[p.index-1].x+shape_points[p.index-1].x) destination.x = vertices[p.index-1].x+shape_points[p.index-1].x; //Lock it to the previous shape point
			if(destination.x > vertices[p.index+1].x) destination.x = vertices[p.index+1].x;
		}
		
		vertices[p.index] = destination;
	}
	else if(p.point_type==SHAPE_POINT){
		
		
		// vertices[p.index] + relative_destination = destination, so:
		vec2 relative_destination = destination - vertices[p.index];
		if(p.is_inverted){
			// vertices[p.index] - relative_destination = destination since it's inverted
			relative_destination = relative_destination*-1.f; //If it's inverted, reflect it across the vertex
		}
		if(relative_destination.x<0.f) relative_destination.x = 0.f; //Clamp it so that the graph's tangeant vector can't go backwards
		
		//Make sure the graph point is always recoverable:
		if(relative_destination.y<-2.f) relative_destination.y = -2.f;
		else if(relative_destination.y>2.f) relative_destination.y = 2.f;
		
		shape_points[p.index] = relative_destination;
	}
	
	//Clamp your shape points with the other shape points to keep it functional:
	if(p.index > 0){ //Let the previous clamp you?
		if( vertices[p.index].x-shape_points[p.index].x < vertices[p.index-1].x+shape_points[p.index-1].x ) shape_points[p.index].x = vertices[p.index].x-(vertices[p.index-1].x+shape_points[p.index-1].x);
	}
	if(p.index < vertices.size()-1){ //Clamp the next?
		if( vertices[p.index+1].x < vertices[p.index].x+shape_points[p.index].x ) shape_points[p.index].x = vertices[p.index+1].x-vertices[p.index].x;
		if( vertices[p.index+1].x-shape_points[p.index+1].x < vertices[p.index].x+shape_points[p.index].x ) shape_points[p.index+1].x = vertices[p.index+1].x-(vertices[p.index].x+shape_points[p.index].x);
	}
	
	return true;
}

bool BezierCurve::move_point(CurvePoint p,float newPos,bool vertical){
	if(p.index<0) return false;
	if(p.index >= vertices.size()) return false;
	
	if(p.point_type == VERTEX){
		if(vertical) return move_point( p, vec2(vertices[p.index].x,newPos) );
		else return move_point( p, vec2(newPos,vertices[p.index].y) );
	}
	else if(p.point_type == SHAPE_POINT){
		if(vertical) return move_point( p, vec2(shape_points[p.index].x,newPos) );
		else return move_point( p, vec2(newPos,shape_points[p.index].y) );
	}
	else return false;
}

bool BezierCurve::remove_point(CurvePoint p){
	if(p.point_type != VERTEX) return false;
	if(p.index<1 || p.index>vertices.size()-2) return false;
	
	vertices.erase(vertices.begin()+p.index);
	shape_points.erase(shape_points.begin()+p.index);
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
		vec2 p2 = vertices[i]+vec2(shape_points[i].x,BEZIER_Y_SCALE*shape_points[i].y);
		vec2 p3 = vertices[i+1]-vec2(shape_points[i+1].x,BEZIER_Y_SCALE*shape_points[i+1].y);
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
