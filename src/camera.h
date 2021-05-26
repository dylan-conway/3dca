#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/cam.h>

#include "defines.h"

struct Camera {
    vec3 position;
    vec3 up;
    vec3 target;
};

struct Camera InitCamera();
void GetMVP(struct Camera* cam, mat4* model, mat4 out_mvp);

#endif