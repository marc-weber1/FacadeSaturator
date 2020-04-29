#include "TestUI.h"

#include "glm/glm.hpp"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "CurveShaders.h"


//from loadShaders.cpp : (BAD FORM I KNOW I'LL FIX IT LATER SSH)
GLuint LoadShaders(const char * VertexSourcePointer, const char * FragmentSourcePointer);
GLuint LoadShaders(const char * VertexSourcePointer, const char * FragmentSourcePointer, const char* GeometrySourcePointer);


//openGL drawing

bool TestUI::initGLContext(){
	
	//Compile Shader
	single_color_shader = LoadShaders(VERTEX_2D_SHADER_BASIC, SINGLE_COLOR_FRAG_SHADER);
	circle_shader = LoadShaders(VERTEX_2D_SHADER_BASIC, SINGLE_COLOR_FRAG_SHADER, POINT_CIRCLE_GEOMETRY_SHADER);
	if (!single_color_shader || !circle_shader) {
		printf("Failed to compile shaders.\n");
		return false;
	}
	
	glUseProgram(single_color_shader);
	line_color_uniform = glGetUniformLocation(single_color_shader,"fragment_color");
	
	glUseProgram(circle_shader);
	// Color of circles does not change, so set it once
	GLint circle_color_uniform = glGetUniformLocation(circle_shader,"fragment_color");
	glUniform3fv(circle_color_uniform,1,&color_circles.x);
	// Aspect ratio however changes every time the window is configured oops maybe fix that later
	GLint aspect_ratio_uniform = glGetUniformLocation(circle_shader,"aspect_ratio");
	glUniform1f(aspect_ratio_uniform,1.f); //Sometimes not 1??
	
	//Initialize data objects
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	curve_updated = true; //To initialize the buffers
	
	//Where are the data chunks in the buffer?
	glVertexAttribPointer( //position
		0,2,GL_FLOAT,GL_FALSE,curve.get_stride(),(void*)0
	);
	glEnableVertexAttribArray(0);
	
	//Load the background texture
	//background_image_data = stbi_load_from_memory();
	
	printf("Context Created.\n");
	
	return true;
}

void TestUI::destroyGLContext(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(single_color_shader);
	glDeleteProgram(circle_shader);
	stbi_image_free(background_image_data);
}

void TestUI::drawFrame(PuglView* view_t){
	// Refresh the hi-res curve
	if( curve_updated ){
		hires_curve.clear();
		curve.get_curve_buffer(hires_curve);
		curve_updated = false;
	}
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Buffer shape points
	std::vector<glm::vec2> shape_points; //Could reduce the amount of calls for this
	curve.get_shape_point_buffer(shape_points);
	glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec2)*shape_points.size(),shape_points.data(),GL_STATIC_DRAW);
	
	// RENDER THE TANGEANT LINES
	glUseProgram(single_color_shader);
	glUniform3fv(line_color_uniform,1,&color_tangeants.x);
	glDrawArrays(GL_LINES,0,shape_points.size());
	
	// RENDER THE SHAPE CIRCLES
	glUseProgram(circle_shader);
	glUniform3fv(line_color_uniform,1,&color_shape_circles.x);
	glDrawArrays(GL_POINTS,0,shape_points.size());
	
	// Buffer vertices
	glBufferData(GL_ARRAY_BUFFER,curve.get_vertex_buffer_size(),curve.get_vertex_ptr(),GL_STATIC_DRAW);
	
	// RENDER THE POINT CIRCLES
	glDrawArrays(GL_POINTS,0,curve.get_num_vertices());
	
	// RENDER THE CURVE LINES
	glUseProgram(single_color_shader);
	//glUniform3fv(line_color_uniform,1,&color_lines.x);
	//glDrawArrays(GL_LINE_STRIP,0,curve.get_num_vertices());
	
	// Buffer the hi-res curve
	glBufferData(GL_ARRAY_BUFFER,(GLsizei) sizeof(glm::vec2)*hires_curve.size(),hires_curve.data(),GL_STATIC_DRAW);
	
	// RENDER THE CURVE (hires)
	glUniform3fv(line_color_uniform,1,&color_curve.x);
	glDrawArrays(GL_LINE_STRIP,0,hires_curve.size());
}



// UI Mouse Interaction

void TestUI::mouseDown(uint32_t button,double x,double y){
	if(button == LEFT_MOUSE_BTN){
		CurvePoint index_at_point = curve.check_point(glm::vec2(x,y));
		
		if(index_at_point.exists()) selected_point = index_at_point;
	}
	else if(button == RIGHT_MOUSE_BTN){
		CurvePoint index_at_point = curve.check_point(glm::vec2(x,y));
		
		if(!index_at_point.exists()) selected_point = curve.add_point(glm::vec2(x,y));
		else{
			selected_point=NULL_CURVE_POINT;
			printf("Removing...\n");
			curve.remove_point(index_at_point);
			printf("Removed.\n");
		}
		curve_updated = true;
	}
	
	mouse_down[button] = true;
}

void TestUI::mouseUp(uint32_t button,double x,double y){
	selected_point = NULL_CURVE_POINT;
	mouse_down[button] = false;
}

void TestUI::mouseMove(double x, double y){
	if( mouse_down[LEFT_MOUSE_BTN] || mouse_down[RIGHT_MOUSE_BTN] ){
		if(selected_point.exists()){
			curve.move_point(selected_point,glm::vec2(x,y));
			curve_updated = true;
		}
	}
}
