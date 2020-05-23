#include "IParamBezier.h"

#include <cassert>
#include <cmath>

#include <immintrin.h>
#include "memtools.h"

//DEBUG
#include <iostream>
#include <fstream>


using namespace glm;



// Utilities

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


// DEBUG

void IParamBezier::check_assertions(){
	
	//Assertion 1
	assert(vertices.size() == mDelegate->GetParam(kNumCurvePoints)->Int());
	assert(shape_points.size() == mDelegate->GetParam(kNumCurvePoints)->Int());
	
	//Assertion 2
	double max_shape_point_x = -1.;
	for(int i=1; i<vertices.size()-1; i++){
		assert((GLfloat)max_shape_point_x <= vertices[i].x);
		
		double next_param_x = mDelegate->GetParam(kInitCurvePoint+2*i)->Value();
		if(next_param_x > max_shape_point_x) max_shape_point_x = next_param_x;
	}
	
	//Assertion 3
	for(int i=0; i<vertices.size(); i++){
		GLfloat shape_point_x = (GLfloat) mDelegate->GetParam(kInitShapePoint+2*i)->Value();
		if(i<vertices.size()-1) shape_point_x = clamp(shape_point_x,0,vertices[i+1].x-vertices[i].x);
		if(i>0) shape_point_x = clamp(shape_point_x,0,vertices[i].x-(vertices[i-1].x+shape_points[i-1].x));
		
		assert(shape_points[i].x == shape_point_x);
	}
}



// Alter curve from UI

void IParamBezier::set_parameter_from_UI(int paramIdx, double val){
	double min = mDelegate->GetParam(paramIdx)->GetMin();
	double max = mDelegate->GetParam(paramIdx)->GetMax();
	
	mDelegate->BeginInformHostOfParamChangeFromUI(paramIdx);
	mDelegate->SendParameterValueFromUI(paramIdx,(val-min)/(max-min));
	mDelegate->EndInformHostOfParamChangeFromUI(paramIdx);
}

void IParamBezier::update_vertices_from_UI(int starting_point){
	int num_points = mDelegate->GetParam(kNumCurvePoints)->Int();
	
	if(starting_point>0){
		set_parameter_from_UI(kInitShapePoint+2*(starting_point-1),shape_points[starting_point-1].x);
		set_parameter_from_UI(kInitShapePoint+2*(starting_point-1)+1,shape_points[starting_point-1].y);
	}
	
	for(int i=starting_point; i<num_points; i++){
		if(i>0 && i<num_points-1) set_parameter_from_UI(kInitCurvePoint+2*i,vertices[i].x);
		set_parameter_from_UI(kInitCurvePoint+2*i+1,vertices[i].y);
		set_parameter_from_UI(kInitShapePoint+2*i,shape_points[i].x);
		set_parameter_from_UI(kInitShapePoint+2*i+1,shape_points[i].y);
	}
}

CurvePoint IParamBezier::add_point_from_UI(glm::vec2 location){
	if( mDelegate->GetParam(kNumCurvePoints)->Int() >= (int) mDelegate->GetParam(kNumCurvePoints)->GetMax() ) return NULL_CURVE_POINT;
	
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
	
	//Update the params
	set_parameter_from_UI(kNumCurvePoints,(double) vertices.size());
	update_vertices_from_UI(point_index);
	if(DEBUG_ASSERTIONS) check_assertions();
	
	curve_updated_ui = true;
	curve_updated_dsp = true;
	
	return p;
}

CurvePoint IParamBezier::check_point(vec2 click_point){
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

bool IParamBezier::remove_point_from_UI(CurvePoint p){
	if(p.point_type != VERTEX) return false;
	if(p.index<1 || p.index>vertices.size()-2) return false;
	
	vertices.erase(vertices.begin()+p.index);
	shape_points.erase(shape_points.begin()+p.index);
	
	//Update the params
	set_parameter_from_UI(kNumCurvePoints,(double) vertices.size());
	update_vertices_from_UI(p.index);
	if(DEBUG_ASSERTIONS) check_assertions();
	
	curve_updated_ui = true;
	curve_updated_dsp = true;
	
	return true;
}

vec2 IParamBezier::move_point_from_UI(CurvePoint p, vec2 destination){
	if(p.index<0) return vec2(0.f,0.f);
	if(p.index >= vertices.size()) return vec2(0.f,0.f);
	
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
	if(p.index<vertices.size()-1) shape_points[p.index].x = clamp(shape_points[p.index].x,0,vertices[p.index+1].x-vertices[p.index].x);
	if(p.index>0) shape_points[p.index].x = clamp(shape_points[p.index].x,0,vertices[p.index].x-(vertices[p.index-1].x+shape_points[p.index-1].x));
	
	//Clamp the next point
	if(p.index<vertices.size()-1) shape_points[p.index+1].x = clamp(shape_points[p.index+1].x,0,vertices[p.index+1].x-(vertices[p.index].x+shape_points[p.index].x));
	
	//Update the params
	update_vertices_from_UI(p.index);
	if(DEBUG_ASSERTIONS) check_assertions();
	
	curve_updated_ui = true;
	curve_updated_dsp = true;
	
	if(p.point_type==VERTEX){
		return vertices[p.index];
	}
	else if(p.point_type==SHAPE_POINT){
		return shape_points[p.index];
	}
	else return vec2(0.f,0.f); //Should not happen
}



// Display curve on UI (All getters)

GLsizei IParamBezier::get_stride(){
	return sizeof(vec2);
}

void* IParamBezier::get_vertex_ptr(){
	return vertices.data();
}

GLsizei IParamBezier::get_num_vertices(){
	return (GLsizei) vertices.size();
}

/*GLsizeiptr IParamBezier::get_vertex_buffer_size(){
	return get_stride()*get_num_vertices();
}*/

void IParamBezier::get_shape_point_buffer(std::vector<vec2>& points){
	const size_t num_vertices = shape_points.size();
	for(int i=0;i<num_vertices;i++){
		points.push_back(vertices[i]-shape_points[i]);
		points.push_back(vertices[i]+shape_points[i]);
	}
}

void* IParamBezier::get_hires_ptr(){
	if(curve_updated_ui){
		update_hires_buffer();
		curve_updated_ui = false;
	}
	
	return hires_curve.data();
}

GLsizei IParamBezier::get_num_hires(){
	return (GLsizei) hires_curve.size();
}

void IParamBezier::update_hires_buffer(){ //Not quite optimized
	hires_curve.clear();

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
			
			hires_curve.push_back(l1+(l2-l1)*t);
		}
	}
	
	hires_curve.push_back(vertices[num_edges]);
}



// Change curve from host

void IParamBezier::update_vertex_x_from_host(int starting_vertex){
	if(starting_vertex<=0) return; //Cant move the lowest vertex
	
	GLfloat current_max = vertices[starting_vertex-1].x;
	
	for(int i=starting_vertex; i<vertices.size()-1; i++){
		GLfloat new_value = (GLfloat) mDelegate->GetParam(kInitCurvePoint+2*i)->Value();
		
		if(new_value>current_max) current_max = new_value;
		vertices[i].x = current_max;
	}
}

void IParamBezier::update_shape_point_x_from_host(int starting_vertex){
	for(int i=starting_vertex; i<vertices.size(); i++){
		GLfloat new_value = (GLfloat) mDelegate->GetParam(kInitShapePoint+2*i)->Value();
		
		if(i<vertices.size()-1) new_value = clamp(new_value,0,vertices[i+1].x-vertices[i].x);
		if(i>0) new_value = clamp(new_value,0,vertices[i].x-(vertices[i-1].x+shape_points[i-1].x));
		
		shape_points[i].x = new_value;
	}
}

void IParamBezier::add_vertices_from_host(int num_to_add){
	for(int i=0;i<num_to_add;i++){
		vertices.push_back(vec2(1.f,0.f));
		shape_points.push_back(DEFAULT_SHAPE_POINT);
	}
}

void IParamBezier::remove_vertices_from_host(int num_to_remove){
	for(int i=0;i<num_to_remove;i++){
		vertices.pop_back();
		shape_points.pop_back();
	}
	vertices[vertices.size()-1].x = 1.f;
}

void IParamBezier::update_param_from_host(int paramIdx){
	int current_num_vertices = vertices.size();
	
	if(paramIdx == kNumCurvePoints){ //Host added/removed points
		/*int num_points_to_add = mDelegate->GetParam(kNumCurvePoints)->Int() - vertices.size();
		
		if(num_points_to_add>0){
			add_vertices_from_host(num_points_to_add);
			
			//For safety oops (when it loads the preset)
			update_vertex_x_from_host(1);
			update_shape_point_x_from_host(0);
		}
		else if(num_points_to_add<0){
			remove_vertices_from_host(-1*num_points_to_add);
			
			update_vertex_x_from_host(vertices.size()-1);
			update_shape_point_x_from_host(vertices.size()-1);
		}*/
	}
	else if(kInitCurvePoint <= paramIdx && paramIdx < kInitCurvePoint+2*current_num_vertices){ //Host moved a vertex
		int vertex_number = (paramIdx-kInitCurvePoint)/2;
		int is_vertical = (paramIdx-kInitCurvePoint)%2;
		
		if(0<=vertex_number && vertex_number < vertices.size()-1){
			if(is_vertical) vertices[vertex_number].y = (GLfloat) mDelegate->GetParam(paramIdx)->Value();
			else if(0<vertex_number && vertex_number<vertices.size()-1){ //Can't move the x coord of the first or last vertex
				update_vertex_x_from_host(vertex_number);
				update_shape_point_x_from_host(vertex_number);
			}
		}
	}
	else if(kInitShapePoint <= paramIdx && paramIdx < kInitShapePoint+2*current_num_vertices){ //Host moved a tangeant line
		int vertex_number = (paramIdx-kInitCurvePoint)/2;
		int is_vertical = (paramIdx-kInitCurvePoint)%2;
		
		if(0<=vertex_number && vertex_number < vertices.size()-1){
			if(is_vertical) shape_points[vertex_number].y = (GLfloat) mDelegate->GetParam(paramIdx)->Value();
			else{
				update_shape_point_x_from_host(vertex_number);
			}
		}
	}
	
	// v Don't check assertions here or loading a preset will call this function several times
	//if(DEBUG_ASSERTIONS) check_assertions();
	
	curve_updated_ui = true;
	curve_updated_dsp = true;
}

void IParamBezier::update_params_from_host(){ //Host is either refreshing or initializing the UI
	int num_points_to_add = mDelegate->GetParam(kNumCurvePoints)->Int() - vertices.size();
	if(num_points_to_add>0){
		add_vertices_from_host(num_points_to_add);
	}
	else if(num_points_to_add<0){
		remove_vertices_from_host(-1*num_points_to_add);
	}
	
	update_vertex_x_from_host(1);
	update_shape_point_x_from_host(0);
	
	for(int i=0;i<vertices.size();i++){
		vertices[i].y = (GLfloat) mDelegate->GetParam(kInitCurvePoint+2*i+1)->Value();
		shape_points[i].y = (GLfloat) mDelegate->GetParam(kInitShapePoint+2*i+1)->Value();
	}
	
	if(DEBUG_ASSERTIONS) check_assertions();
}



// Get function values from host - CALLED FROM DSP THREAD - USE CAUTION - OPTIMIZE

void IParamBezier::waveshape(float* inputs, float* outputs, int nFrames, bool lastChannel){
	//__m128 f;
	
	if(curve_updated_dsp){
		
		bezier_lut_target_length = vertices.size()-1; //This is a race condition oops, what if vertices changes size
		
		for(int i=0;i<bezier_lut_target_length;i++){
			if(i==0) bezier_lut_target[i].x[0] =  -1.f;
			else bezier_lut_target[i].x[0] = vertices[i].x;
			if(i==bezier_lut_target_length-1) bezier_lut_target[i].x[3] = 1.f;
			else bezier_lut_target[i].x[3] = vertices[i+1].x;
			
			bezier_lut_target[i].y[0] = vertices[i].y;
			bezier_lut_target[i].y[3] = vertices[i+1].y;
			
			//Calculate some intermediate values with the shape points
			__m128 bezier_points_x = _mm_set_ps(bezier_lut_target[i].x[3],bezier_lut_target[i].x[3]-shape_points[i+1].x,bezier_lut_target[i].x[0]+shape_points[i].x,bezier_lut_target[i].x[0]); // [P3,P2,P1,P0]
			bezier_lut_target[i].x[1] = bezier_value(bezier_points_x,0.333333f);
			bezier_lut_target[i].x[2] = bezier_value(bezier_points_x,0.666667f);
			
			//And agen wida
			__m128 bezier_points_y = _mm_set_ps(bezier_lut_target[i].y[3],bezier_lut_target[i].y[3]-BEZIER_Y_SCALE*shape_points[i+1].y,bezier_lut_target[i].y[0]+BEZIER_Y_SCALE*shape_points[i].y,bezier_lut_target[i].y[0]);
			bezier_lut_target[i].y[1] = bezier_value(bezier_points_y,0.333333f);
			bezier_lut_target[i].y[2] = bezier_value(bezier_points_y,0.666667f);
		}
	}
	
	for(int i=0;i<nFrames;i++){
		
		//Guarantee sample safety (fix this so it still interpolates)
		if(inputs[i]<=-1.f){
			outputs[i] = bezier_lut_target[0].y[0];
			continue;
		}
		else if(inputs[i]>=1.f){
			outputs[i] = bezier_lut_target[bezier_lut_current_length-1].y[3];
			continue;
		}
		
		
		// TARGET VALUE
		
		//Binary search to find the vertices the sample lies between
		int lower = 0;
		int higher = bezier_lut_target_length;
		while(lower<higher){
			int m = (lower+higher)/2;
			if(bezier_lut_target[m].x[0] < inputs[i]) lower = m+1;
			else higher = m;
		}
		// At this point lower should count the number of elements less than inputs[i], so
		lower -= 1;
		
		// DEBUG
		if(DEBUG_ASSERTIONS) assert(lower < bezier_lut_target_length);
		if(DEBUG_ASSERTIONS) assert(lower > -1);
		
		__m128 x = _mm_load_ps(bezier_lut_target[lower].x);
		__m128 y = _mm_load_ps(bezier_lut_target[lower].y);
		
		float target_val = cubic_interpolate(x,y,inputs[i]);
		
		
		// CURRENT VALUE
		
		//Binary search to find the vertices the sample lies between
		lower = 0;
		higher = bezier_lut_current_length;
		while(lower<higher){
			int m = (lower+higher)/2;
			if(bezier_lut_current[m].x[0] < inputs[i]) lower = m+1;
			else higher = m;
		}
		// At this point lower should count the number of elements less than inputs[i], so
		lower -= 1;
		
		// DEBUG
		if(DEBUG_ASSERTIONS) assert(lower < bezier_lut_current_length);
		if(DEBUG_ASSERTIONS) assert(lower > -1);
		
		x = _mm_load_ps(bezier_lut_current[lower].x);
		y = _mm_load_ps(bezier_lut_current[lower].y);
		
		float current_val = cubic_interpolate(x,y,inputs[i]);
		
		//vv REPLACE THIS ! bad interpolation. Derivative not smooth >:
		outputs[i] = linear_interpolate(current_val,target_val,clamp(1.f*i/sample_smooth_number,0.f,1.f));
	}
	
	
	//Update the current if it needs to be
	if(curve_updated_dsp && lastChannel){
		if(DEBUG_ASSERTIONS) assert(nFrames >= sample_smooth_number); //Make sure that the sample smooth number is less than the buffer size
		
		float_copy((float*) bezier_lut_current, (float*) bezier_lut_target, bezier_lut_target_length*sizeof(bezier_segment)/sizeof(float));
		bezier_lut_current_length = bezier_lut_target_length;
		
		curve_updated_dsp = false;
	}
}