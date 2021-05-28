#ifndef _MESHES_H_
#define _MESHES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>

#include "camera.h"
#include "shaders.h"

struct Cube {
    vec3 position;
    vec3 scale;
    mat4 model;
};

void InitCubeVAO();

struct Cube InitCube(vec3 position);
void SetPositionCube(struct Cube* cube, vec3 new_position);
void SetScaleCube(struct Cube* cube, vec3 new_scale);
void DrawCube(struct Cube* cube, struct Camera* cam);
void DrawStaticCube(vec3 position, struct Camera* cam);


#endif