#ifndef _SHADERS_H_
#define _SHADERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <cglm/mat4.h>

// struct ShaderProgram {
//     GLuint program, mvp_location;
// };

int  CreateShaders();
void DeleteShaders();

void UpdateMVPUniform(mat4 mvp);
void UpdateCameraUniform(vec3 camera_position);
void UpdateLightDirectionUniform(vec3 light_direction);

#endif