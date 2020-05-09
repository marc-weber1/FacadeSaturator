#include "TestUI.h"

#include "glm/glm.hpp"
#include <math.h>
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "CurveShaders.h"
#include "LoadShaders.h"

#include "data_resources.h"

//openGL drawing

bool TestUI::initGLContext(){
	// DEBUG
	std::ofstream debug_file;
	debug_file.open("C:/Users/facade/Documents/VSTs/facade-saturator-log.txt");
	for(int i=0;i<2*OSCILLOSCOPE_BUFFER_SIZE;i++){
		debug_file << oscilloscope_x_coords[i];
		debug_file << " " << std::endl;
	}
	debug_file.close();
	
	
	//Compile Shader
	single_color_shader = LoadShaders(VERTEX_2D_SHADER_BASIC, SINGLE_COLOR_FRAG_SHADER);
	circle_shader = LoadShaders(VERTEX_2D_SHADER_BASIC, SINGLE_COLOR_FRAG_SHADER, POINT_CIRCLE_GEOMETRY_SHADER);
	image_shader = LoadShaders(VERTEX_2D_SHADER_BASIC, IMAGE_FRAG_SHADER);
	if (!single_color_shader || !circle_shader || !image_shader) {
		printf("Failed to compile shaders.\n");
		return false;
	}
	
	// SET SINGLE COLOR UNIFORMS
	glUseProgram(single_color_shader);
	line_color_uniform = glGetUniformLocation(single_color_shader,"fragment_color");
	
	// SET CIRCLE UNIFORMS
	glUseProgram(circle_shader);
	// Color of circles does not change, so set it once
	GLint circle_color_uniform = glGetUniformLocation(circle_shader,"fragment_color");
	glUniform3fv(circle_color_uniform,1,&color_circles.x);
	// Aspect ratio however changes every time the window is configured oops maybe fix that later
	GLint aspect_ratio_uniform = glGetUniformLocation(circle_shader,"aspect_ratio");
	glUniform1f(aspect_ratio_uniform,1.f); //Sometimes not 1??
	
	// SET IMAGE UNIFORMS
	glUseProgram(image_shader);
	//Load the background texture
	glGenTextures(1, &bg_texture_uniform);
	glBindTexture(GL_TEXTURE_2D, bg_texture_uniform);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	stbi_set_flip_vertically_on_load(true);
	int bg_width, bg_height, bg_num_channels;
	unsigned char* background_image_data = stbi_load_from_memory(DATA_RESOURCES_BG_PNG, sizeof(DATA_RESOURCES_BG_PNG), &bg_width, &bg_height, &bg_num_channels, 0);
	if(!background_image_data){
		printf("Failed to load image.\n");
		return false;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bg_width, bg_height, 0, GL_RGB, GL_UNSIGNED_BYTE, background_image_data);
	//glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(background_image_data);
	
	
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
	
	printf("Context Created.\n");
	
	return true;
}

void TestUI::destroyGLContext(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(single_color_shader);
	glDeleteProgram(circle_shader);
}

void TestUI::drawFrame(PuglView* view_t){
	// Refresh the hi-res curve
	if( curve_updated ){
		hires_curve.clear();
		curve.get_curve_buffer(hires_curve);
		curve_updated = false;
	}
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Buffer background triangles
	//glUseProgram(image_shader);
	//glBufferData(GL_ARRAY_BUFFER,sizeof(ENTIRE_SCREEN),ENTIRE_SCREEN,GL_STATIC_DRAW);
	
	// RENDER BG IMAGE
	//glBindTexture(GL_TEXTURE_2D, bg_texture_uniform);
	//glDrawArrays(GL_TRIANGLES,0,sizeof(ENTIRE_SCREEN)/sizeof(glm::vec2));
	
	// Buffer oscilloscope lines (triangles?)
	float oscilloscope_sound_buffer_L[OSCILLOSCOPE_BUFFER_SIZE];
	oscilloscopeBuffer[0].Peek(oscilloscope_sound_buffer_L,0,OSCILLOSCOPE_BUFFER_SIZE);
	float oscilloscope_sound_buffer_R[OSCILLOSCOPE_BUFFER_SIZE];
	oscilloscopeBuffer[1].Peek(oscilloscope_sound_buffer_R,0,OSCILLOSCOPE_BUFFER_SIZE);
	float oscilloscope_sound_buffer[OSCILLOSCOPE_BUFFER_SIZE*2]; //THIS COULD FUCK UP ON SYSTEMS WHERE GLFLOAT IS NOT FLOAT
	float_interleave(oscilloscope_sound_buffer, oscilloscope_sound_buffer_L, oscilloscope_sound_buffer_R, OSCILLOSCOPE_BUFFER_SIZE);
	float oscilloscope_vertex_buffer[OSCILLOSCOPE_BUFFER_SIZE*4];
	float_interleave(oscilloscope_vertex_buffer, oscilloscope_x_coords, oscilloscope_sound_buffer, OSCILLOSCOPE_BUFFER_SIZE*2);
	glBufferData(GL_ARRAY_BUFFER,sizeof(oscilloscope_vertex_buffer),oscilloscope_vertex_buffer,GL_STATIC_DRAW);
	
	// RENDER OSCILLOSCOPE TRIANGLES
	glUseProgram(single_color_shader);
	glUniform3fv(line_color_uniform,1,&color_lines.x);
	glDrawArrays(GL_TRIANGLE_STRIP,0,OSCILLOSCOPE_BUFFER_SIZE*2);
	
	// RENDER OSCILLOSCOPE LINES
	glUniform3fv(line_color_uniform,1,&color_lines.x);
	glDrawArrays(GL_LINE_STRIP,0,OSCILLOSCOPE_BUFFER_SIZE*2);
	
	// Buffer shape points
	std::vector<glm::vec2> shape_points; //Could reduce the amount of calls for this
	curve.get_shape_point_buffer(shape_points);
	glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec2)*shape_points.size(),shape_points.data(),GL_STATIC_DRAW);
	
	// RENDER THE TANGEANT LINES
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
	
	// Buffer the hi-res curve
	glBufferData(GL_ARRAY_BUFFER,(GLsizei) sizeof(glm::vec2)*hires_curve.size(),hires_curve.data(),GL_STATIC_DRAW);
	
	// RENDER THE CURVE (hires)
	glUseProgram(single_color_shader);
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
		
		if(!index_at_point.exists()){ //Add point
			selected_point = curve.add_point(glm::vec2(x,y));
			
			setParameterFromUI(kNumPointsEnabled,(int) curve.get_num_vertices());
			//Also update the point values
			if(selected_point.exists()){
				updateParametersStartingFrom(selected_point.index);
			}
		}
		else if(index_at_point.point_type == VERTEX){ //Remove point
			selected_point=NULL_CURVE_POINT;
			const int index_to_update = index_at_point.index;
			const bool point_removed = curve.remove_point(index_at_point);
			
			if(point_removed){
				setParameterFromUI(kNumPointsEnabled,(int) curve.get_num_vertices());
				updateParametersStartingFrom(index_to_update);
			}
		}
		else if(index_at_point.point_type == SHAPE_POINT){
			
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
			
			
			int paramNumber = 0;
			if(selected_point.point_type == VERTEX){
				paramNumber = kInitCurvePoint;
			}
			else if(selected_point.point_type == SHAPE_POINT){
				paramNumber = kInitShapePoint;
			}
			paramNumber += 2*selected_point.index;
			setParameterFromUI(paramNumber,x);
			setParameterFromUI(paramNumber+1,y);
		}
	}
}



// Parameter Manipulation

// UI -> Plugin
void TestUI::setParameterFromUI(int paramIdx, double val){
	mDelegate->BeginInformHostOfParamChangeFromUI(paramIdx);
	//mDelegate->GetParam(paramIdx)->SetNormalized(val);
	mDelegate->SendParameterValueFromUI(paramIdx,val/2+0.5);
	mDelegate->EndInformHostOfParamChangeFromUI(paramIdx);
}

//UI -> Plugin
void TestUI::setParameterFromUI(int paramIdx, int val){
	double min = mDelegate->GetParam(paramIdx)->GetMin();
	double max = mDelegate->GetParam(paramIdx)->GetMax();
	
	mDelegate->BeginInformHostOfParamChangeFromUI(paramIdx);
	mDelegate->SendParameterValueFromUI(paramIdx,(1.0*val-min)/(max-min));
	mDelegate->EndInformHostOfParamChangeFromUI(paramIdx);
}

// Plugin -> UI
void TestUI::changeUIOnParamChange(int paramIdx){
	if(kInitCurvePoint <= paramIdx && paramIdx < kInitCurvePoint+2*kNumCurvePoints){
		curve.move_point( {paramIdx/2,VERTEX,false}, (float) (mDelegate->GetParam(paramIdx)->Value()), (paramIdx-kInitCurvePoint)%2 == 1 );
		curve_updated=true;
	}
	else if(kInitShapePoint <= paramIdx && paramIdx < kInitShapePoint+2*kNumCurvePoints){
		curve.move_point( {paramIdx/2,SHAPE_POINT,false}, (float) (mDelegate->GetParam(paramIdx)->Value()), (paramIdx-kInitShapePoint)%2 == 1 );
		curve_updated=true;
	}
	else if(paramIdx == kNumPointsEnabled){
		int target_num_points = mDelegate->GetParam(paramIdx)->Int();
		int current_num_points = (int) curve.get_num_vertices();
		
		if( current_num_points < target_num_points ){
			for(int i=current_num_points+1;i<=target_num_points;i++){
				curve.add_point(glm::vec2( mDelegate->GetParam(kInitCurvePoint+2*i)->Value(), mDelegate->GetParam(kInitCurvePoint+1+2*i)->Value() ));
			}
			updateParametersStartingFrom(1);
			curve_updated=true;
		}
		else if( target_num_points < current_num_points ){
			for(int i=current_num_points-2; i>target_num_points-2; i--){ //Can't remove the last point, keep removing second last
				curve.remove_point( {current_num_points-2,VERTEX,false} );
			}
			updateParametersStartingFrom(curve.get_num_vertices()-1);
			curve_updated=true;
		}
	}
}

void TestUI::updateParametersStartingFrom(int paramIdx){
	for(int i=paramIdx; i<curve.get_num_vertices(); i++){
		const glm::vec2 pos = curve.get_position({i,VERTEX,false});
		setParameterFromUI(kInitCurvePoint+2*i,pos.x);
		setParameterFromUI(kInitCurvePoint+2*i+1,pos.y);
		const glm::vec2 shape_pos = curve.get_position({i,SHAPE_POINT,false});
		setParameterFromUI(kInitShapePoint+2*i,shape_pos.x);
		setParameterFromUI(kInitShapePoint+2*i+1,shape_pos.y);
	}
}