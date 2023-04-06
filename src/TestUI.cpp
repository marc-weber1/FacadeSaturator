#include "TestUI.h"

#include "glm/glm.hpp"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "CurveShaders.h"
#include "LoadShaders.h"

#include "data_resources.h"

//openGL drawing

bool TestUI::initGLContext(){
	
	
	//Compile Shader
	single_color_shader = LoadShaders(VERTEX_2D_SHADER_BASIC, SINGLE_COLOR_FRAG_SHADER);
	image_shader = LoadShaders(VERTEX_2D_SHADER_BASIC, IMAGE_FRAG_SHADER);
	if (!single_color_shader || !circle_shader || !image_shader) {
		printf("Failed to compile shaders.\n");
		return false;
	}
	
	// SET SINGLE COLOR UNIFORMS
	glUseProgram(single_color_shader);
	line_color_uniform = glGetUniformLocation(single_color_shader,"fragment_color");
	
	// SET IMAGE UNIFORMS
	glUseProgram(image_shader);
	GLint image_brightness_uniform = glGetUniformLocation(image_shader,"image_brightness");
	glUniform1f(image_brightness_uniform,image_brightness);
	//Load the background texture
	glGenTextures(1, &bg_texture_uniform);
	glBindTexture(GL_TEXTURE_2D, bg_texture_uniform);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	int bg_width, bg_height, bg_num_channels;
	unsigned char* background_image_data = stbi_load_from_memory(DATA_RESOURCES_BG_PNG, sizeof(DATA_RESOURCES_BG_PNG), &bg_width, &bg_height, &bg_num_channels, 0);
	if(!background_image_data){
		printf("Failed to load image.\n");
		return false;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bg_width, bg_height, 0, GL_RGB, GL_UNSIGNED_BYTE, background_image_data);
	stbi_image_free(background_image_data);
	
	
	//Initialize data objects
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	//Where are the data chunks in the buffer?
	glVertexAttribPointer( //position
		0,2,GL_FLOAT,GL_FALSE,sizeof(GL_FLOAT)*2,(void*)0
	);
	glEnableVertexAttribArray(0);
	
	printf("Context Created.\n");
	
	return true;
}

void TestUI::destroyGLContext(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(single_color_shader);
}

void TestUI::drawFrame(PuglView* view_t){
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Buffer background triangles
	glUseProgram(image_shader);
	glBufferData(GL_ARRAY_BUFFER,sizeof(ENTIRE_SCREEN),ENTIRE_SCREEN,GL_STATIC_DRAW);
	
	// RENDER BG IMAGE
	glBindTexture(GL_TEXTURE_2D, bg_texture_uniform);
	glDrawArrays(GL_TRIANGLES,0,sizeof(ENTIRE_SCREEN)/sizeof(glm::vec2));
	
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
}