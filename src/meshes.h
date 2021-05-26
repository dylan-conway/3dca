#ifndef _MESHES_H_
#define _MESHES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

struct Triangle {
    GLuint vao;
};

struct Triangle InitTriangle();
void DrawTriangle(struct Triangle* tri);

struct Cube {
    GLuint vao;
};

struct Cube InitCube();
void DrawCube(struct Cube* cube);


#endif